#include "g1.hpp"
#include "nearest_search.hpp"
#include <cuda_runtime.h>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <utility>

namespace pmedian {
namespace {

using Clock = std::chrono::steady_clock;

double seconds_since(Clock::time_point begin) {
    return std::chrono::duration<double>(Clock::now() - begin).count();
}

void check_cuda(cudaError_t status, const char* operation) {
    if (status != cudaSuccess)
        throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(status));
}

std::size_t checked_product(std::size_t a, std::size_t b) {
    if (b != 0 && a > std::numeric_limits<std::size_t>::max() / b)
        throw std::runtime_error("Rozmiar bufora G1 poza zakresem size_t.");
    return a * b;
}

// Bufor zwalnia pamięć GPU także wtedy, gdy kolejne wywołanie CUDA zgłosi błąd.
template <typename T>
class DeviceBuffer {
public:
    explicit DeviceBuffer(std::size_t count) : bytes_(checked_product(count, sizeof(T))) {
        check_cuda(cudaMalloc(&data_, bytes_), "cudaMalloc");
    }
    ~DeviceBuffer() { cudaFree(data_); }
    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;

    void upload(const T* source) {
        check_cuda(cudaMemcpy(data_, source, bytes_, cudaMemcpyHostToDevice), "cudaMemcpy H2D");
    }
    void download(T* destination) const {
        check_cuda(cudaMemcpy(destination, data_, bytes_, cudaMemcpyDeviceToHost), "cudaMemcpy D2H");
    }
    T* data() const { return data_; }
    std::size_t bytes() const { return bytes_; }

private:
    T* data_ = nullptr;
    std::size_t bytes_;
};

__global__ void evaluate_swaps(const double* weights, const double* distances,
                               const NearestSearch::Nearest* nearest, const std::size_t* stations,
                               const std::size_t* incoming, std::size_t demand_count,
                               std::size_t p, std::size_t swap_count, double* costs) {
    const auto index = static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (index >= swap_count) return;

    const auto out = stations[index % p];
    const auto in = incoming[index / p];
    const double* candidate_distances = distances + in * demand_count;
    double cost = 0.0;
    for (std::size_t i = 0; i < demand_count; ++i) {
        const auto& item = nearest[i];
        const double remaining = item.station == out ? item.second : item.first;
        const double distance = candidate_distances[i] < remaining ? candidate_distances[i] : remaining;
        // Osobne zaokrąglenia i kolejność punktów jak w wersji referencyjnej.
        cost = __dadd_rn(cost, __dmul_rn(weights[i], distance));
    }
    costs[index] = cost;
}

void search_on_gpu(const Instance& instance, NearestSearch& search, Clock::time_point prep_begin,
                   CudaMetrics& metrics) {
    const auto n = instance.demand.size();
    const auto m = instance.candidates.size();
    const auto p = search.station_count();
    const auto swap_count = checked_product(p, m - p);
    constexpr unsigned block_size = 128;
    const auto block_count = (swap_count - 1) / block_size + 1;
    int device;
    int max_grid_x;
    check_cuda(cudaGetDevice(&device), "cudaGetDevice");
    check_cuda(cudaDeviceGetAttribute(&max_grid_x, cudaDevAttrMaxGridDimX, device),
               "cudaDeviceGetAttribute maxGridDimX");
    if (block_count > static_cast<std::size_t>(max_grid_x))
        throw std::runtime_error("Liczba blokow G1 przekracza limit GPU.");

    DeviceBuffer<double> device_weights(n);
    DeviceBuffer<double> device_distances(checked_product(n, m));
    DeviceBuffer<NearestSearch::Nearest> device_nearest(n);
    DeviceBuffer<std::size_t> device_stations(p);
    DeviceBuffer<std::size_t> device_incoming(m - p);
    DeviceBuffer<double> device_costs(swap_count);
    const auto state_bytes = device_nearest.bytes() + device_stations.bytes() + device_incoming.bytes();
    const auto fixed_bytes = device_weights.bytes() + device_distances.bytes();
    metrics.device_bytes = fixed_bytes + state_bytes + device_costs.bytes();

    StationSet incoming;
    incoming.reserve(m - p);
    std::vector<double> costs(swap_count);
    {
        // Macierz zapisujemy kolumnami: wszystkie odległości do jednego kandydata są obok siebie.
        // Wspólne obliczenie odległości na CPU zachowuje zaokrąglenia z S1.
        std::vector<double> distances(n * m);
        for (std::size_t in = 0; in < m; ++in)
            search.cache_candidate(in, distances.data() + in * n);
        std::vector<double> weights(n);
        for (std::size_t i = 0; i < n; ++i) weights[i] = instance.demand[i].weight;

        metrics.prep_seconds = seconds_since(prep_begin);
        const auto h2d_begin = Clock::now();
        device_weights.upload(weights.data());
        device_distances.upload(distances.data());
        check_cuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize H2D");
        metrics.h2d_seconds += seconds_since(h2d_begin);
        metrics.h2d_bytes += fixed_bytes;
    }

    while (true) {
        incoming.clear();
        for (std::size_t in = 0; in < m; ++in)
            if (!search.is_selected(in)) incoming.push_back(in);

        const auto h2d_begin = Clock::now();
        device_nearest.upload(search.nearest().data());
        device_stations.upload(search.stations().data());
        device_incoming.upload(incoming.data());
        check_cuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize H2D");
        metrics.h2d_seconds += seconds_since(h2d_begin);
        metrics.h2d_bytes += state_bytes;

        evaluate_swaps<<<static_cast<unsigned>(block_count), block_size>>>(
            device_weights.data(), device_distances.data(), device_nearest.data(),
            device_stations.data(), device_incoming.data(), n, p, swap_count, device_costs.data());
        check_cuda(cudaGetLastError(), "kernel G1");
        // Czekamy przed D2H, żeby czasu pracy kernela nie zaliczyć do czasu transferu.
        check_cuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize kernel G1");

        const auto d2h_begin = Clock::now();
        device_costs.download(costs.data());
        metrics.d2h_seconds += seconds_since(d2h_begin);
        metrics.d2h_bytes += device_costs.bytes();

        SwapMove best;
        for (std::size_t index = 0; index < swap_count; ++index)
            keep_best(best, search.move_from_cost(index % p, incoming[index / p], costs[index]));
        if (!search.apply(best)) break;
        search.refresh_nearest();
    }
}

} // namespace

SolverResult solve_g1(const Instance& instance, StationSet stations, int) {
    const auto prep_begin = Clock::now();
    NearestSearch search(instance, std::move(stations));
    search.initialize();
    CudaMetrics metrics;
    if (search.station_count() < instance.candidates.size()) {
        search_on_gpu(instance, search, prep_begin, metrics);
    } else {
        // Przy p = m brakuje zamian, więc wynik jest gotowy bez uruchamiania GPU.
        metrics.prep_seconds = seconds_since(prep_begin);
    }
    auto result = search.finish();
    result.cuda = metrics;
    return result;
}

} // namespace pmedian
