#include "g1.hpp"
#include "../seq/build_info.hpp"
#include "solver_cli.hpp"

int main(int argc, char** argv) {
    if (pmedian::print_build_info_if_requested(argc, argv)) return 0;
    return pmedian::run_solver_cli(argc, argv, "pm_g1", "cuda", "G1-thread-per-swap",
                                  "solve_g1_excluding_setup_and_transfers", pmedian::solve_g1, false);
}
