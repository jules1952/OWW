#include "nearest_search.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace pmedian {
namespace {

double distance(const Point& a, const Point& b) {
    const double value = std::hypot(a.x - b.x, a.y - b.y);
    if (!std::isfinite(value))
        throw std::runtime_error("Odleglosc poza zakresem double.");
    return value;
}

} // namespace

NearestSearch::NearestSearch(const Instance& instance, StationSet stations)
    : instance_(instance) {
    validate_stations(instance, stations);
    selected_.resize(instance.candidates.size());
    for (auto station : stations) selected_[station] = true;
    result_.stations = std::move(stations);
    nearest_.resize(instance.demand.size());
}

void NearestSearch::initialize() {
    refresh_nearest();
    double cost = 0.0;
    for (std::size_t i = 0; i < nearest_.size(); ++i)
        cost += instance_.demand[i].weight * nearest_[i].first;
    if (!std::isfinite(cost))
        throw std::runtime_error("Koszt poza zakresem double.");
    result_.initial_cost = result_.cost = cost;
}

void NearestSearch::refresh_nearest() {
    const auto& stations = result_.stations;
    const double infinity = std::numeric_limits<double>::infinity();
    for (std::size_t i = 0; i < nearest_.size(); ++i) {
        // Przy jednej stacji druga odległość pozostaje nieskończona.
        Nearest item{stations.front(), infinity, infinity};
        for (auto station : stations) {
            const double d = distance(instance_.demand[i].position, instance_.candidates[station]);
            if (d < item.first) {
                item.second = item.first;
                item.first = d;
                item.station = station;
            } else if (d < item.second) {
                // Dwie różne stacje mogą być tak samo blisko punktu.
                item.second = d;
            }
        }
        nearest_[i] = item;
    }
}

void NearestSearch::cache_candidate(std::size_t in, double* distances) const {
    const auto& candidate = instance_.candidates[in];
    for (std::size_t i = 0; i < nearest_.size(); ++i)
        distances[i] = distance(instance_.demand[i].position, candidate);
}

SwapMove NearestSearch::evaluate(std::size_t position, std::size_t in,
                                 const std::vector<double>& distances) const {
    const auto out = result_.stations[position];
    double cost = 0.0;
    for (std::size_t i = 0; i < nearest_.size(); ++i) {
        const auto& item = nearest_[i];
        const double remaining = item.station == out ? item.second : item.first;
        // Sumujemy punkty w tej samej kolejności co S0, aby ograniczyć
        // różnice zaokrągleń, które mogłyby zmienić wybór ruchu.
        cost += instance_.demand[i].weight * std::min(remaining, distances[i]);
    }
    return move_from_cost(position, in, cost);
}

SwapMove NearestSearch::move_from_cost(std::size_t position, std::size_t in, double cost) const {
    if (!std::isfinite(cost))
        throw std::runtime_error("Koszt poza zakresem double.");
    return {cost - result_.cost, cost, result_.stations[position], in, position};
}

bool NearestSearch::apply(const SwapMove& best) {
    if (!improves(best, result_.cost)) return false;
    selected_[best.out] = false;
    selected_[best.in] = true;
    auto& stations = result_.stations;
    stations[best.position] = best.in;
    std::sort(stations.begin(), stations.end());
    result_.cost = best.cost;
    ++result_.iterations;
    return true;
}

SolverResult NearestSearch::finish() {
    return std::move(result_);
}

} // namespace pmedian
