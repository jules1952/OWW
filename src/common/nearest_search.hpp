#pragma once

#include "solution.hpp"

namespace pmedian {

// S1 i P1 oceniają zamiany na podstawie tego samego zestawu stacji i odległości.
// Podczas oceny stan pozostaje stały, więc wątki P1 mogą odczytywać go równolegle.
class NearestSearch {
public:
    NearestSearch(const Instance& instance, StationSet stations);
    void initialize();
    void refresh_nearest();
    void cache_candidate(std::size_t in, std::vector<double>& distances) const;
    SwapMove evaluate(std::size_t position, std::size_t in,
                      const std::vector<double>& distances) const;
    bool apply(const SwapMove& best);
    SolverResult finish();

    bool is_selected(std::size_t candidate) const { return selected_[candidate]; }
    std::size_t station_count() const { return result_.stations.size(); }

private:
    struct Nearest {
        std::size_t station;
        double first;
        double second;
    };

    const Instance& instance_;
    SolverResult result_;
    std::vector<bool> selected_;
    std::vector<Nearest> nearest_;
};

} // namespace pmedian
