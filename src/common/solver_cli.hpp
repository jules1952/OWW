#pragma once

#include "solution.hpp"

namespace pmedian {

using Solver = SolverResult (*)(const Instance&, StationSet, int);

int run_solver_cli(int argc, char** argv, const char* executable, const char* variant,
                   const char* stage, const char* timing, Solver solve, bool supports_threads);

} // namespace pmedian
