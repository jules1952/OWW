#pragma once
#include "solution.hpp"

namespace pmedian {

using S0Result = SolverResult;

S0Result solve_s0(const Instance& instance, StationSet stations);

} // namespace pmedian
