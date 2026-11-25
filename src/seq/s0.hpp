#pragma once
#include "../common/input.hpp"
#include <string>

namespace pmedian {

struct S0Result {
    StationSet stations;
    double initial_cost = 0.0;
    double cost = 0.0;
    std::size_t iterations = 0;
};

S0Result solve_s0(const Instance& instance, StationSet stations);
std::string station_set_hash(const StationSet& sorted_stations);

} // namespace pmedian
