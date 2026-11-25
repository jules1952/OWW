#pragma once

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>

namespace pmedian {

// Profil faz działa wyłącznie w osobnej kompilacji pomiarowej.
struct PhaseProfile {
    using Clock = std::chrono::steady_clock;
    const char* name;
    Clock::duration elapsed{};
    std::size_t calls = 0;

    explicit PhaseProfile(const char* phase_name) : name(phase_name) {}

    ~PhaseProfile() {
        std::cerr << name << ',' << calls << ',' << std::setprecision(17)
                  << std::chrono::duration<double>(elapsed).count() << '\n';
    }
};

struct PhaseTimer {
    PhaseProfile& phase;
    PhaseProfile::Clock::time_point start = PhaseProfile::Clock::now();

    explicit PhaseTimer(PhaseProfile& measured_phase) : phase(measured_phase) {}

    ~PhaseTimer() {
        phase.elapsed += PhaseProfile::Clock::now() - start;
        ++phase.calls;
    }
};

} // namespace pmedian
