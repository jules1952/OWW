#include "input.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void write_file(const std::filesystem::path& path, const std::string& text) {
    std::ofstream file(path, std::ios::binary);
    file.exceptions(std::ios::failbit | std::ios::badbit);
    file << text;
    file.close();
}

template <typename Read>
void expect_error(Read read, const std::filesystem::path& path, int line = 0) {
    const auto location = path.u8string() + (line == 0 ? "" : ":" + std::to_string(line));
    try {
        read();
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()).find(location + ": ") == 0) {
            return;
        }
        throw;
    }
    throw std::runtime_error("Expected parser error at " + location);
}

void check_inputs(const std::filesystem::path& directory) {
    std::filesystem::create_directories(directory);
    const auto instance_path = directory / "instance.txt";
    const auto start_path = directory / "start.txt";

    // Współrzędne z kropką i wykładnikiem oraz mieszane zakończenia wierszy.
    write_file(instance_path, "2 3\r\n0 1.25 7\r\n2e-2 -1 1\n4 3\n3 4\n0 0\n");
    const auto instance = pmedian::read_instance(instance_path);
    if (instance.demand.size() != 2 || instance.candidates.size() != 3
        || instance.demand[0].position.y != 1.25 || instance.demand[0].weight != 7
        || instance.demand[1].position.x != 0.02 || instance.demand[1].position.y != -1
        || instance.candidates[0].x != 4 || instance.candidates[1].x != 3
        || instance.candidates[2].x != 0) {
        throw std::runtime_error("Unexpected instance contents");
    }

    write_file(start_path, "1\r\n2");
    if (pmedian::read_start(start_path, instance) != pmedian::StationSet{2}) {
        throw std::runtime_error("Incorrect start for p=1");
    }
    write_file(start_path, "3\n0 1 2\n \n");
    if (pmedian::read_start(start_path, instance) != pmedian::StationSet{0, 1, 2}) {
        throw std::runtime_error("Incorrect start for p=m");
    }

    const auto bad_instance = [&](const char* contents, int line) {
        write_file(instance_path, contents);
        expect_error([&] { pmedian::read_instance(instance_path); }, instance_path, line);
    };
    bad_instance("0 1\n", 1);
    bad_instance("-1 1\n", 1);
    bad_instance("1 1 extra\n", 1);
    bad_instance("1 1\n0 0\n1 1 1\n", 2);
    bad_instance("1 1\nnan 0 1\n0 0\n", 2);
    bad_instance("1 1\n0 0 0\n0 0\n", 2);
    bad_instance("1 1\n0 0 1.5\n0 0\n", 2);
    bad_instance("1 1\n0 0 1\n", 3);
    bad_instance("1 1\n0 0 1\ninf 0\n", 3);
    bad_instance("1 1\n0 0 1\n0 0\n0 0\n", 4);

    const auto bad_start = [&](const char* contents, int line) {
        write_file(start_path, contents);
        expect_error([&] { pmedian::read_start(start_path, instance); }, start_path, line);
    };
    bad_start("0\n", 1);
    bad_start("4\n0 1 2 3\n", 1);
    bad_start("1\n-1\n", 2);
    bad_start("1\n3\n", 2);
    bad_start("2\n0 0\n", 2);
    bad_start("2\n1 0\n", 2);
    bad_start("2\n0\n1\n", 2);
    bad_start("1\n0 1\n", 2);
    bad_start("1\n0\n1\n", 3);

    const auto missing = directory / "missing.txt";
    expect_error([&] { pmedian::read_instance(missing); }, missing);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Expected a directory for test inputs\n";
        return 1;
    }
    try {
        check_inputs(argv[1]);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
