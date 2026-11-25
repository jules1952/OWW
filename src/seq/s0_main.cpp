#include "s0.hpp"
#include "build_info.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    try {
        if (pmedian::print_build_info_if_requested(argc, argv)) return 0;
        if (argc != 4)
            throw std::runtime_error("Uzycie: pm_s0 INSTANCJA START WYNIK.csv");
        const std::filesystem::path output = argv[3];
        const auto stations_path = output.string() + ".stations.txt";
        const auto meta_path = output.string() + ".meta.txt";
        if (std::filesystem::exists(output) ||
            std::filesystem::exists(stations_path) || std::filesystem::exists(meta_path))
            throw std::runtime_error("Wynik juz istnieje; wybierz nowa nazwe CSV.");

        const auto read_begin = std::chrono::steady_clock::now();
        const auto instance = pmedian::read_instance(argv[1]);
        const auto start = pmedian::read_start(argv[2], instance);
        const auto begin = std::chrono::steady_clock::now();
        const double prep = std::chrono::duration<double>(begin - read_begin).count();
        const auto result = pmedian::solve_s0(instance, start);
        const double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - begin).count();

        if (output.has_parent_path())
            std::filesystem::create_directories(output.parent_path());
        std::ofstream csv(output), stations(stations_path), meta(meta_path);
        if (!csv || !stations || !meta)
            throw std::runtime_error("Nie mozna otworzyc plikow wynikowych.");
        csv.imbue(std::locale::classic());
        csv << std::setprecision(17)
            << "variant,instance,dist,n,m,p,threads,rep,t_prep,t_h2d,t_search,t_d2h,t_total,iterations,cost,peak_ram_mb,gpu_mem_mb,final_set_hash\n"
            << "seq," << std::quoted(std::filesystem::path(argv[1]).generic_string(), '"', '"')
            << ",euclidean," << instance.demand.size() << ','
            << instance.candidates.size() << ',' << start.size()
            << ",1,1," << prep << ",0," << elapsed << ",0,,"
            << result.iterations << ',' << result.cost << ",,,"
            << pmedian::station_set_hash(result.stations) << '\n';
        stations << result.stations.size() << '\n';
        stations << result.stations.front();
        for (std::size_t i = 1; i < result.stations.size(); ++i)
            stations << ' ' << result.stations[i];
        stations << '\n';
        meta << "stage=S0-direct\ninstance=" << argv[1] << "\nstart=" << argv[2]
             << "\nepsilon=1e-9\nprecision=double\nwarmups_in_process=0\n"
             << "hash=FNV1a64_u64_little_endian\n"
             << "t_prep=read_instance_and_start\nt_search=full_solve_s0\n";
        csv.close(); stations.close(); meta.close();
        if (!csv || !stations || !meta)
            throw std::runtime_error("Blad zapisu; pliki moga byc niepelne.");
        std::cout << std::setprecision(12)
                  << "Koszt: " << result.initial_cost << " -> " << result.cost
                  << "\nZamiany: " << result.iterations
                  << "\nCzas S0: " << elapsed << " s\nStacje: ";
        for (auto station : result.stations) std::cout << station << ' ';
        std::cout << "\nCSV: " << output.string() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
