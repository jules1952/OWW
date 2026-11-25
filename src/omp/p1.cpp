#include "p1.hpp"
#include "nearest_search.hpp"
#include <omp.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>

namespace pmedian {
namespace {

struct Worker {
    std::vector<double> distances;
    SwapMove best;
    std::exception_ptr error;
};

} // namespace

SolverResult solve_p1(const Instance& instance, StationSet stations, int threads) {
    if (threads < 1) throw std::invalid_argument("Liczba watkow musi byc dodatnia.");
    NearestSearch search(instance, std::move(stations));
    search.initialize();
    const auto remaining = instance.candidates.size() - search.station_count();
    if (remaining == 0) return search.finish();

    const int worker_limit = static_cast<int>(std::min(remaining, static_cast<std::size_t>(threads)));
    std::vector<Worker> workers(static_cast<std::size_t>(worker_limit));
    for (auto& worker : workers) worker.distances.resize(instance.demand.size());
    // Typ ze znakiem zapewnia zgodność z pętlą OpenMP w MSVC.
    const auto candidate_count = static_cast<std::ptrdiff_t>(instance.candidates.size());
    int actual_threads = 1;
    int peak_threads = 1;

    while (true) {
        // Wątek dostaje kandydata in razem ze wszystkimi jego zamianami, aby użyć
        // tych samych odległości dla każdego out. Obliczenia pojedynczej delty są sekwencyjne.
        #pragma omp parallel num_threads(worker_limit) default(none) shared(search, workers, actual_threads, candidate_count)
        {
            auto& worker = workers[static_cast<std::size_t>(omp_get_thread_num())];
            SwapMove best;
            std::exception_ptr error;
            #pragma omp master
            {
                actual_threads = omp_get_num_threads();
            }
            #pragma omp for schedule(static)
            for (std::ptrdiff_t candidate = 0; candidate < candidate_count; ++candidate) {
                const auto in = static_cast<std::size_t>(candidate);
                if (error || search.is_selected(in)) continue;
                try {
                    search.cache_candidate(in, worker.distances);
                    for (std::size_t position = 0; position < search.station_count(); ++position)
                        keep_best(best, search.evaluate(position, in, worker.distances));
                } catch (...) {
                    // Wyjątek zgłaszamy dopiero po powrocie do wątku sterującego.
                    error = std::current_exception();
                }
            }
            worker.best = best;
            worker.error = error;
        }

        peak_threads = std::max(peak_threads, actual_threads);
        SwapMove best;
        for (int i = 0; i < actual_threads; ++i) {
            const auto& worker = workers[static_cast<std::size_t>(i)];
            if (worker.error) std::rethrow_exception(worker.error);
            keep_best(best, worker.best);
        }
        if (!search.apply(best)) break;
        search.refresh_nearest();
    }
    auto result = search.finish();
    result.threads = peak_threads;
    return result;
}

} // namespace pmedian
