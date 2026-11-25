#include "../src/seq/s0.hpp"
#include "../src/seq/s1.hpp"

#include <iostream>

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
    return ok;
}

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
    return ok ? 0 : 1;
}
