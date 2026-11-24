#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace pmedian {

struct Point {
    double x;
    double y;
};

struct DemandPoint {
    Point position;
    double weight;
};

struct Instance {
    std::vector<DemandPoint> demand;
    // Pozycja w wektorze jest indeksem kandydata z pliku startowego.
    std::vector<Point> candidates;
};

using StationSet = std::vector<std::size_t>;

// Wczytują format tekstowy v1. Błędy wejścia zgłaszają jako std::runtime_error.
Instance read_instance(const std::filesystem::path& path);
StationSet read_start(const std::filesystem::path& path, const Instance& instance);

}  // namespace pmedian
