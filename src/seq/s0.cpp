#include "s0.hpp"
#ifdef PMEDIAN_PROFILE_SEQ
#include "phase_profile.hpp"
#endif
#include <algorithm>
#include <cmath>
#include <limits>
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
    validate_stations(instance, stations);

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
        SwapMove best;

        // Próby zmieniają jedynie kopię zestawu. Każda delta odnosi się do tego samego kosztu.
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
                keep_best(best, {candidate_cost - result.cost, candidate_cost, out, in, position});
            }
            trial[position] = out;
        }

        if (!improves(best, result.cost)) break;

        // Dopiero po całym przeglądzie zmieniamy bieżący zestaw.
        selected[best.out] = false;
        selected[best.in] = true;
        stations[best.position] = best.in;
        std::sort(stations.begin(), stations.end());
        result.cost = best.cost;
        ++result.iterations;
    }
    result.stations = std::move(stations);
    return result;
}

} // namespace pmedian
