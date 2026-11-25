#pragma once

#include "solution.hpp"

namespace pmedian {

// threads to górny limit. Wynik podaje największą faktyczną liczbę wątków w iteracji.
SolverResult solve_p1(const Instance& instance, StationSet stations, int threads);

} // namespace pmedian
