#pragma once
#include "solution.hpp"

namespace pmedian {

using S1Result = SolverResult;

// Ten sam wybór ruchu i warunek stopu co S0; inna ocena kosztu zamiany.
S1Result solve_s1(const Instance& instance, StationSet stations);

} // namespace pmedian
