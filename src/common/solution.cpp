#include "solution.hpp"
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace pmedian {

void validate_stations(const Instance& instance, StationSet& stations) {
    if (instance.demand.empty() || instance.candidates.empty() || stations.empty())
        throw std::runtime_error("Instancja i zbior stacji nie moga byc puste.");
    std::sort(stations.begin(), stations.end());
    if (stations.back() >= instance.candidates.size() ||
        std::adjacent_find(stations.begin(), stations.end()) != stations.end())
        throw std::runtime_error("Niepoprawny zbior stacji.");
}

std::string station_set_hash(const StationSet& sorted_stations) {
    // FNV-1a 64-bit, każdy indeks zapisany jako 8 bajtów little-endian.
    std::uint64_t hash = UINT64_C(14695981039346656037);
    for (auto station : sorted_stations) {
        const auto value = static_cast<std::uint64_t>(station);
        for (unsigned shift = 0; shift < 64; shift += 8) {
            hash ^= (value >> shift) & 0xffu;
            hash *= UINT64_C(1099511628211);
        }
    }
    std::ostringstream text;
    text << std::hex << std::setw(16) << std::setfill('0') << hash;
    return text.str();
}

} // namespace pmedian
