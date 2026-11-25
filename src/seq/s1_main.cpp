#include "s1.hpp"
#include "build_info.hpp"
#include "solver_cli.hpp"
#include <utility>

int main(int argc, char** argv) {
    if (pmedian::print_build_info_if_requested(argc, argv)) return 0;
    return pmedian::run_solver_cli(argc, argv, "pm_s1", "seq", "S1-nearest2-candidate-cache",
        "full_solve_s1", [](const pmedian::Instance& instance, pmedian::StationSet stations, int) {
            return pmedian::solve_s1(instance, std::move(stations));
        }, false);
}
