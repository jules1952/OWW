#pragma once

#include "input.hpp"
#include <string>

namespace pmedian {

inline constexpr double epsilon = 1e-9;

struct SolverResult {
    StationSet stations;
    double initial_cost = 0.0;
    double cost = 0.0;
    std::size_t iterations = 0;
    int threads = 1;
};

// Przegląd zaczyna się od delty 0. Zapamiętujemy tylko ruchy obniżające koszt.
struct SwapMove {
    double delta = 0.0;
    double cost = 0.0;
    std::size_t out = 0;
    std::size_t in = 0;
    std::size_t position = 0;
};

// Przy równej delcie porównujemy out, a potem in. Dzięki temu kolejność oceny
// zamian w poszczególnych wątkach nie wpływa na wybór ruchu.
inline void keep_best(SwapMove& best, const SwapMove& candidate) {
    if (candidate.delta < best.delta ||
        (candidate.delta == best.delta && candidate.delta < 0.0 &&
         (candidate.out < best.out ||
          (candidate.out == best.out && candidate.in < best.in))))
        best = candidate;
}

inline bool improves(const SwapMove& move, double cost) {
    // Próg bierze pod uwagę bieżący koszt i obowiązuje dopiero przy zatwierdzaniu.
    return move.delta < -epsilon * cost;
}

void validate_stations(const Instance& instance, StationSet& stations);
std::string station_set_hash(const StationSet& sorted_stations);

} // namespace pmedian
