#include "s1.hpp"
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

// Dla jednego punktu zapamiętujemy dwie najbliższe WYBRANE stacje.
struct Nearest {
    std::size_t station;
    double first;
    double second;
};

double distance(const Point& a, const Point& b) {
    const double value = std::hypot(a.x - b.x, a.y - b.y);
    if (!std::isfinite(value))
        throw std::runtime_error("Odleglosc poza zakresem double.");
    return value;
}

// Wywołujemy raz na start i po każdej zaakceptowanej zamianie, nie po
// każdej PRÓBNEJ zamianie. Przy p=1 druga odległość pozostaje nieskończona.
void refresh_nearest(const Instance& instance, const StationSet& stations,
                     std::vector<Nearest>& nearest) {
    const double infinity = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < instance.demand.size(); ++i) {
        Nearest item{stations.front(), infinity, infinity};
        for (auto station : stations) {
            const double d = distance(instance.demand[i].position,
                                      instance.candidates[station]);
            if (d < item.first) {
                item.second = item.first;
                item.first = d;
                item.station = station;
            } else if (d < item.second) {
                // Równa odległość do INNEJ stacji też może być drugą najbliższą.
                item.second = d;
            }
        }
        nearest[i] = item;
    }
}

double initial_cost(const Instance& instance, const std::vector<Nearest>& nearest) {
    double cost = 0.0;
    for (std::size_t i = 0; i < instance.demand.size(); ++i)
        cost += instance.demand[i].weight * nearest[i].first;
    if (!std::isfinite(cost))
        throw std::runtime_error("Koszt poza zakresem double.");
    return cost;
}

double swapped_cost(const Instance& instance, const std::vector<Nearest>& nearest,
                    const std::vector<double>& added_distances, std::size_t out) {
    double cost = 0.0;
    for (std::size_t i = 0; i < instance.demand.size(); ++i) {
        const auto& item = nearest[i];
        const double remaining = item.station == out ? item.second : item.first;
        // Koszt sumujemy w tej samej kolejności punktów co S0. Nie sumujemy
        // osobno delt punktów, co mogłoby zmienić zaokrąglenia i rozstrzyganie remisów.
        cost += instance.demand[i].weight * std::min(remaining, added_distances[i]);
    }
    if (!std::isfinite(cost))
        throw std::runtime_error("Koszt poza zakresem double.");
    return cost;
}

} // namespace

S1Result solve_s1(const Instance& instance, StationSet stations) {
#ifdef PMEDIAN_PROFILE_SEQ
    PhaseProfile nearest_profile("refresh_nearest");
    PhaseProfile distance_profile("candidate_distances");
    PhaseProfile swap_profile("trial_cost");
#endif
    constexpr double epsilon = 1e-9;
    if (instance.demand.empty() || instance.candidates.empty() || stations.empty())
        throw std::runtime_error("Instancja i zbior stacji nie moga byc puste.");
    std::sort(stations.begin(), stations.end());
    if (stations.back() >= instance.candidates.size() ||
        std::adjacent_find(stations.begin(), stations.end()) != stations.end())
        throw std::runtime_error("Niepoprawny zbior stacji.");

    std::vector<Nearest> nearest(instance.demand.size());
    std::vector<double> added_distances(instance.demand.size());
    {
#ifdef PMEDIAN_PROFILE_SEQ
        PhaseTimer timer(nearest_profile);
#endif
        refresh_nearest(instance, stations, nearest);
    }
    S1Result result;
    result.initial_cost = result.cost = initial_cost(instance, nearest);
    std::vector<bool> selected(instance.candidates.size(), false);
    for (auto station : stations) selected[station] = true;

    while (true) {
        double best_delta = 0.0, best_cost = result.cost;
        bool found = false;
        std::size_t best_position = 0;
        std::pair<std::size_t, std::size_t> best_move{};
        for (std::size_t in = 0; in < selected.size(); ++in) {
            if (selected[in]) continue;
            {
#ifdef PMEDIAN_PROFILE_SEQ
                PhaseTimer timer(distance_profile);
#endif
                // Odległości do kandydata in są wspólne dla wszystkich stacji out.
                const auto& candidate = instance.candidates[in];
                for (std::size_t i = 0; i < added_distances.size(); ++i)
                    added_distances[i] = distance(instance.demand[i].position, candidate);
            }
            for (std::size_t position = 0; position < stations.size(); ++position) {
                const auto out = stations[position];
                double candidate_cost;
                {
#ifdef PMEDIAN_PROFILE_SEQ
                    PhaseTimer timer(swap_profile);
#endif
                    candidate_cost = swapped_cost(instance, nearest, added_distances, out);
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
        }
        if (!found || !(best_delta < -epsilon * result.cost)) break;

        selected[best_move.first] = false;
        selected[best_move.second] = true;
        stations[best_position] = best_move.second;
        std::sort(stations.begin(), stations.end());
        result.cost = best_cost;
        ++result.iterations;
#ifdef PMEDIAN_PROFILE_SEQ
        PhaseTimer timer(nearest_profile);
#endif
        refresh_nearest(instance, stations, nearest);
    }
    result.stations = std::move(stations);
    return result;
}

} // namespace pmedian
