#include "s0.hpp"
#include "build_info.hpp"
#include "solver_cli.hpp"
#include <utility>

int main(int argc, char** argv) {
    if (pmedian::print_build_info_if_requested(argc, argv)) return 0;
    return pmedian::run_solver_cli(argc, argv, "pm_s0", "seq", "S0-direct", "full_solve_s0",
        [](const pmedian::Instance& instance, pmedian::StationSet stations, int) {
            return pmedian::solve_s0(instance, std::move(stations));
        }, false);
}
