#pragma once
#include "../common/input.hpp"

namespace pmedian {

struct S1Result {
    StationSet stations;
    double initial_cost = 0.0;
    double cost = 0.0;
    std::size_t iterations = 0;
};

// Ten sam wybór ruchu i warunek stopu co S0; inna ocena kosztu zamiany.
S1Result solve_s1(const Instance& instance, StationSet stations);

} // namespace pmedian
