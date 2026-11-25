#pragma once

#include "solution.hpp"

namespace pmedian {

// threads ogranicza rozmiar zespołu. Wynik podaje największy faktyczny zespół.
SolverResult solve_p1(const Instance& instance, StationSet stations, int threads);

} // namespace pmedian
