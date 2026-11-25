#include "s0.hpp"
#ifdef PMEDIAN_PROFILE_SEQ
#include "phase_profile.hpp"
#endif
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace pmedian {
namespace {

// S0 oblicza odległości i pełny koszt od nowa dla każdej zamiany.
double cost_of(const Instance& instance, const StationSet& stations) {
    double cost = 0.0;
    for (const auto& demand : instance.demand) {
        double nearest = std::numeric_limits<double>::infinity();
        for (std::size_t j : stations) {
            const auto& candidate = instance.candidates[j];
            const double distance = std::hypot(
                demand.position.x - candidate.x,
                demand.position.y - candidate.y);
            if (!std::isfinite(distance))
                throw std::runtime_error("Odleglosc poza zakresem double.");
            nearest = std::min(nearest, distance);
        }
        cost += demand.weight * nearest;
    }
    if (!std::isfinite(cost))
        throw std::runtime_error("Koszt poza zakresem double.");
    return cost;
}

} // namespace

S0Result solve_s0(const Instance& instance, StationSet stations) {
#ifdef PMEDIAN_PROFILE_SEQ
    PhaseProfile initial_profile("initial_cost");
    PhaseProfile swap_profile("trial_cost");
#endif
    constexpr double epsilon = 1e-9;
    if (instance.demand.empty() || instance.candidates.empty() || stations.empty())
        throw std::runtime_error("Instancja i zbior stacji nie moga byc puste.");
    std::sort(stations.begin(), stations.end());
    if (stations.back() >= instance.candidates.size() ||
        std::adjacent_find(stations.begin(), stations.end()) != stations.end())
        throw std::runtime_error("Niepoprawny zbior stacji.");

    S0Result result;
    {
#ifdef PMEDIAN_PROFILE_SEQ
        PhaseTimer timer(initial_profile);
#endif
        result.initial_cost = result.cost = cost_of(instance, stations);
    }
    std::vector<bool> selected(instance.candidates.size(), false);
    for (auto station : stations) selected[station] = true;

    while (true) {
        auto trial = stations;
        double best_delta = 0.0, best_cost = result.cost;
        bool found = false;
        std::size_t best_position = 0;
        std::pair<std::size_t, std::size_t> best_move{};

        // Wszystkie ruchy oceniamy względem jednego bieżącego rozwiązania.
        for (std::size_t position = 0; position < stations.size(); ++position) {
            const auto out = stations[position];
            for (std::size_t in = 0; in < selected.size(); ++in) {
                if (selected[in]) continue;
                trial[position] = in;
                double candidate_cost;
                {
#ifdef PMEDIAN_PROFILE_SEQ
                    PhaseTimer timer(swap_profile);
#endif
                    candidate_cost = cost_of(instance, trial);
                }
                const double delta = candidate_cost - result.cost;
                const auto move = std::make_pair(out, in);
                if (delta < best_delta ||
                    (found && delta == best_delta && move < best_move)) {
                    found = true;
                    best_delta = delta;
                    best_cost = candidate_cost;
                    best_move = move;
                    best_position = position;
                }
            }
            trial[position] = out;
        }

        if (!found || !(best_delta < -epsilon * result.cost)) break;

        // Zatwierdzamy tylko jedną, najlepszą zamianę z całego przeglądu.
        selected[best_move.first] = false;
        selected[best_move.second] = true;
        stations[best_position] = best_move.second;
        std::sort(stations.begin(), stations.end());
        result.cost = best_cost;
        ++result.iterations;
    }
    result.stations = std::move(stations);
    return result;
}

std::string station_set_hash(const StationSet& sorted_stations) {
    // FNV-1a 64-bit; indeksy po 8 bajtów little-endian, niezależnie od size_t.
    std::uint64_t hash = UINT64_C(14695981039346656037);
    for (auto station : sorted_stations) {
        const auto value = static_cast<std::uint64_t>(station);
        for (unsigned shift = 0; shift < 64; shift += 8) {
            hash ^= (value >> shift) & 0xffu;
            hash *= UINT64_C(1099511628211);
        }
    }
    std::ostringstream text;
    text << std::hex << std::setw(16) << std::setfill('0') << hash;
    return text.str();
}

} // namespace pmedian
