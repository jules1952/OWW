#pragma once

#include "solution.hpp"

namespace pmedian {

// S1, P1 i G1 używają tego samego stanu bieżącego. Podczas oceny zamian
// stan pozostaje stały, więc można go odczytywać równolegle.
class NearestSearch {
public:
    struct Nearest {
        std::size_t station;
        double first;
        double second;
    };

    NearestSearch(const Instance& instance, StationSet stations);
    void initialize();
    void refresh_nearest();
    // Bufor mieści po jednej odległości dla każdego punktu popytu.
    void cache_candidate(std::size_t in, double* distances) const;
    SwapMove evaluate(std::size_t position, std::size_t in,
                      const std::vector<double>& distances) const;
    SwapMove move_from_cost(std::size_t position, std::size_t in, double cost) const;
    bool apply(const SwapMove& best);
    SolverResult finish();

    bool is_selected(std::size_t candidate) const { return selected_[candidate]; }
    std::size_t station_count() const { return result_.stations.size(); }
    const StationSet& stations() const { return result_.stations; }
    const std::vector<Nearest>& nearest() const { return nearest_; }

private:

    const Instance& instance_;
    SolverResult result_;
    std::vector<bool> selected_;
    std::vector<Nearest> nearest_;
};

} // namespace pmedian
