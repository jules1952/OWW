#pragma once

#include <iostream>
#include <string_view>

namespace pmedian {

// Dane pochodzą z konfiguracji kompilacji danego pliku wykonywalnego.
inline bool print_build_info_if_requested(int argc, char** argv) {
    if (argc != 2 || std::string_view(argv[1]) != "--build-info") return false;
    std::cout << "compiler=" << PMEDIAN_BUILD_COMPILER
              << "\nconfiguration=" << PMEDIAN_BUILD_CONFIG
              << "\nprofile=" << PMEDIAN_BUILD_PROFILE << '\n';
#ifdef PMEDIAN_BUILD_OPENMP
    std::cout << "openmp=" << PMEDIAN_BUILD_OPENMP << '\n';
#endif
    return true;
}

} // namespace pmedian
