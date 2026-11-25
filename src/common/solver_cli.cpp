#include "solver_cli.hpp"
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string_view>

namespace pmedian {
namespace {

int requested_threads(std::string_view value) {
    int count;
    const auto end = value.data() + value.size();
    const auto parsed = std::from_chars(value.data(), end, count);
    if (parsed.ec != std::errc{} || parsed.ptr != end || count < 1)
        throw std::runtime_error("Liczba watkow musi byc dodatnia liczba calkowita.");
    return count;
}

} // namespace

int run_solver_cli(int argc, char** argv, const char* executable, const char* variant,
                   const char* stage, const char* timing, Solver solve, bool supports_threads) {
    try {
        int threads = 1;
        if (supports_threads && argc == 6 && std::string_view(argv[4]) == "--threads") {
            threads = requested_threads(argv[5]);
        } else if (argc != 4) {
            std::string usage = std::string("Uzycie: ") + executable + " INSTANCJA START WYNIK.csv";
            if (supports_threads) usage += " [--threads N]";
            throw std::runtime_error(usage);
        }

        const std::filesystem::path output = argv[3];
        const auto stations_path = output.string() + ".stations.txt";
        const auto meta_path = output.string() + ".meta.txt";
        if (std::filesystem::exists(output) || std::filesystem::exists(stations_path) ||
            std::filesystem::exists(meta_path))
            throw std::runtime_error("Wynik juz istnieje; wybierz nowa nazwe CSV.");

        using Clock = std::chrono::steady_clock;
        const auto read_begin = Clock::now();
        const auto instance = read_instance(argv[1]);
        const auto start = read_start(argv[2], instance);
        const auto begin = Clock::now();
        const double prep = std::chrono::duration<double>(begin - read_begin).count();
        const auto result = solve(instance, start, threads);
        const double elapsed = std::chrono::duration<double>(Clock::now() - begin).count();

        if (output.has_parent_path())
            std::filesystem::create_directories(output.parent_path());
        std::ofstream csv(output), stations(stations_path), meta(meta_path);
        if (!csv || !stations || !meta)
            throw std::runtime_error("Nie mozna otworzyc plikow wynikowych.");
        csv.imbue(std::locale::classic());
        csv << std::setprecision(17)
            << "variant,instance,dist,n,m,p,threads,rep,t_prep,t_h2d,t_search,t_d2h,t_total,iterations,cost,peak_ram_mb,gpu_mem_mb,final_set_hash\n"
            << variant << ',' << std::quoted(std::filesystem::path(argv[1]).generic_string(), '"', '"')
            << ",euclidean," << instance.demand.size() << ',' << instance.candidates.size() << ','
            << start.size() << ',' << result.threads << ",1," << prep << ",0," << elapsed << ",0,,"
            << result.iterations << ',' << result.cost << ",,," << station_set_hash(result.stations) << '\n';
        stations << result.stations.size() << '\n' << result.stations.front();
        for (std::size_t i = 1; i < result.stations.size(); ++i)
            stations << ' ' << result.stations[i];
        stations << '\n';
        meta.imbue(std::locale::classic());
        meta << "stage=" << stage << "\ninstance=" << argv[1] << "\nstart=" << argv[2]
             << "\nepsilon=" << epsilon << "\nprecision=double\nwarmups_in_process=0\n"
             << "hash=FNV1a64_u64_little_endian\nt_prep=read_instance_and_start\n"
             << "t_search=" << timing << '\n';
        if (supports_threads) meta << "threads_requested=" << threads << '\n';
        csv.close();
        stations.close();
        meta.close();
        if (!csv || !stations || !meta)
            throw std::runtime_error("Blad zapisu; pliki moga byc niepelne.");
        std::cout << std::setprecision(12)
                  << "Koszt: " << result.initial_cost << " -> " << result.cost
                  << "\nZamiany: " << result.iterations
                  << "\nCzas: " << elapsed << " s\nStacje: ";
        for (auto station : result.stations) std::cout << station << ' ';
        std::cout << "\nCSV: " << output.string() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

} // namespace pmedian
