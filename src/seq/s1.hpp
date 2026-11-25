#pragma once
#include "solution.hpp"

namespace pmedian {

using S1Result = SolverResult;

// Ocenia koszt zamiany z użyciem odległości do dwóch najbliższych stacji.
S1Result solve_s1(const Instance& instance, StationSet stations);

} // namespace pmedian
