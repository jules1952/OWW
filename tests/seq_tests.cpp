#include "../src/seq/s0.hpp"
#include "../src/seq/s1.hpp"
#ifdef PMEDIAN_TEST_OPENMP
#include "../src/omp/p1.hpp"
#endif
#ifdef PMEDIAN_TEST_CUDA
#include "g1.hpp"
#include <string_view>
#endif
#if defined(PMEDIAN_TEST_OPENMP) || defined(PMEDIAN_TEST_CUDA)
#include <stdexcept>
#endif

#include <iostream>
#include <limits>

namespace {

using pmedian::Instance;
using pmedian::StationSet;

template <typename Result>
bool matches(const Result& result, double cost, const StationSet& stations,
             std::size_t iterations) {
    return result.cost == cost && result.stations == stations && result.iterations == iterations;
}

bool check(const char* name, const Instance& instance, const StationSet& start,
           double cost, const StationSet& stations, std::size_t iterations) {
    // Każdy solver porównujemy z oczekiwanym wynikiem, żeby wykryć także wspólny błąd.
    const auto s0 = pmedian::solve_s0(instance, start);
    const auto s1 = pmedian::solve_s1(instance, start);
    bool ok = true;
    if (!matches(s0, cost, stations, iterations)) {
        std::cerr << name << ": S0 returned an unexpected result\n";
        ok = false;
    }
    if (!matches(s1, cost, stations, iterations)) {
        std::cerr << name << ": S1 returned an unexpected result\n";
        ok = false;
    }
#ifdef PMEDIAN_TEST_OPENMP
    for (int requested : {1, 2, 4}) {
        const auto p1 = pmedian::solve_p1(instance, start, requested);
        if (!matches(p1, cost, stations, iterations) || p1.threads < 1 || p1.threads > requested ||
            (start.size() == instance.candidates.size() && p1.threads != 1)) {
            std::cerr << name << ": P1 returned an unexpected result with " << requested << " threads\n";
            ok = false;
        }
    }
#endif
#ifdef PMEDIAN_TEST_CUDA
    const auto g1 = pmedian::solve_g1(instance, start, 1);
    const bool has_swaps = start.size() < instance.candidates.size();
    if (!matches(g1, cost, stations, iterations) || !g1.cuda ||
        ((g1.cuda->device_bytes > 0) != has_swaps) ||
        ((g1.cuda->h2d_bytes > 0) != has_swaps) || ((g1.cuda->d2h_bytes > 0) != has_swaps)) {
        std::cerr << name << ": G1 returned an unexpected result or transfer counts\n";
        ok = false;
    }
#endif
    return ok;
}

#ifdef PMEDIAN_TEST_OPENMP
bool check_worker_exception() {
    const double limit = std::numeric_limits<double>::max();
    const Instance instance{{{{limit, 0}, 1}}, {{limit, 0}, {limit, 1}, {-limit, 0}}};
    try {
        pmedian::solve_p1(instance, {0}, 2);
    } catch (const std::runtime_error&) {
        return true;
    }
    std::cerr << "P1 did not propagate distance overflow from an OpenMP worker\n";
    return false;
}
#endif

#ifdef PMEDIAN_TEST_CUDA
bool check_cuda_overflow(const Instance& instance, std::string_view expected_message) {
    try {
        pmedian::solve_g1(instance, {0}, 1);
    } catch (const std::runtime_error& error) {
        if (error.what() == expected_message) return true;
        std::cerr << "G1 returned an unexpected error: " << error.what() << '\n';
        return false;
    }
    std::cerr << "G1 did not report overflow: " << expected_message << '\n';
    return false;
}
#endif

} // namespace

int main() {
    bool ok = true;
    ok &= check("tie on incoming station",
                {{{{0, 0}, 1}}, {{0, 0}, {0, 0}, {1, 0}}}, {2}, 0.0, {0}, 1);
    // (0, 3) i (1, 2) dają ten sam koszt. Gdyby najpierw porównano in, wygrałaby (1, 2).
    ok &= check("out before in",
                {{{{0, 0}, 1}, {{3, 0}, 1}, {{5, 0}, 1}}, {{2, 0}, {4, 0}, {5, 0}, {1, 0}, {7, 0}}},
                {0, 1}, 3.0, {1, 3}, 1);
    ok &= check("equal nearest distances",
                {{{{0, 0}, 1}}, {{2, 0}, {2, 0}, {0, 0}}}, {0, 1}, 0.0, {1, 2}, 1);
    ok &= check("all selected",
                {{{{0, 0}, 2}, {{4, 0}, 1}}, {{1, 0}, {3, 0}}}, {0, 1}, 3.0, {0, 1}, 0);
    ok &= check("zero current cost",
                {{{{0, 0}, 1}}, {{0, 0}, {0, 0}}}, {0}, 0.0, {0}, 0);
    ok &= check("epsilon boundary rejected",
                {{{{0, 0}, 1}}, {{1e9, 0}, {999999999, 0}}}, {0}, 1e9, {0}, 0);
    ok &= check("improvement above epsilon",
                {{{{0, 0}, 1}}, {{1e9, 0}, {999999998, 0}}}, {0}, 999999998.0, {1}, 1);
    ok &= check("weighted Euclidean cost",
                {{{{0, 0}, 3}, {{3, 4}, 1}}, {{3, 4}, {0, 0}, {0, 4}}}, {0}, 5.0, {1}, 1);
#ifdef PMEDIAN_TEST_OPENMP
    ok &= check_worker_exception();
#endif
#ifdef PMEDIAN_TEST_CUDA
    Instance multiple_blocks{{{{0, 0}, 1}}, std::vector<pmedian::Point>(133, {5, 0})};
    multiple_blocks.candidates.back() = {0, 0};
    ok &= check("winning swap in partial final block", multiple_blocks, {0}, 0.0, {132}, 1);

    const double limit = std::numeric_limits<double>::max();
    ok &= check_cuda_overflow({{{{limit, 0}, 1}}, {{limit, 0}, {-limit, 0}}},
                              "Odleglosc poza zakresem double.");
    ok &= check_cuda_overflow({{{{0, 0}, 2}}, {{0, 0}, {limit, 0}}},
                              "Koszt poza zakresem double.");
#endif
    return ok ? 0 : 1;
}
