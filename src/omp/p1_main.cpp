#include "p1.hpp"
#include "../seq/build_info.hpp"
#include "solver_cli.hpp"

int main(int argc, char** argv) {
    if (pmedian::print_build_info_if_requested(argc, argv)) return 0;
    return pmedian::run_solver_cli(argc, argv, "pm_p1", "omp", "P1-candidate-static",
                                  "full_solve_p1", pmedian::solve_p1, true);
}
