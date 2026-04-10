#pragma once
#include <chrono>
#include <cstdint>
#include <algorithm>
#include <ctime>
#include <cstring>
#include <string> 

using SteadyClock = std::chrono::steady_clock;
using TimePoint   = SteadyClock::time_point;
using Nanoseconds = std::chrono::nanoseconds;

inline TimePoint now() {
    return SteadyClock::now();
}

inline double elapsed_seconds(TimePoint start, TimePoint end) {
    return std::chrono::duration<double>(end - start).count();
}

struct WCETTracker {

    TimePoint start_time;
    int64_t worst = 0;
    int64_t total = 0;
    uint64_t count = 0;

    void start() {
        start_time = now();
    }

    void stop() {
        auto elapsed = std::chrono::duration_cast<Nanoseconds>(now() - start_time).count();

        worst = std::max(worst, elapsed);
        total += elapsed;
        count++;
    }

    int64_t worst_ns() const {
        return worst;
    }

    double mean_ns() const {
        if (count == 0) return 0.0;
        return static_cast<double>(total) / static_cast<double>(count); //creates temp variable and performs division
    }

    void reset() {
        worst = 0;
        total = 0;
        count = 0;
    }
};


struct RateLoopTimer {
    Nanoseconds period;          // target cycle duration (e.g., 1,000,000 ns for 1 kHz)
    TimePoint cycle_start;       // when tick() was called
    bool _overran = false;       // did last cycle exceed the period?

    explicit RateLoopTimer(double frequency_hz)
        : period(Nanoseconds(static_cast<int64_t>(1'000'000'000.0 / frequency_hz))) {}

    void tick() {
        cycle_start = now();
    }

    double wait() {
        TimePoint target = cycle_start + period;

        if (now() >= target) {
            // Work already took longer than the period — we missed the deadline
            _overran = true;
        } else {
            _overran = false;
            // Spin-wait until the period has elapsed
            while (now() < target) {}
        }

        return elapsed_seconds(cycle_start, now());
    }

    bool overran() const {
        return _overran;
    }
};

// Generate a timestamped filename for logs
// Returns a string like "logs/neptune_2026-04-09_14-30-45.log"
inline std::string timestamped_filename(const std::string& prefix = "neptune", 
                                       const std::string& dir = "logs",
                                       const std::string& ext = "log") {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* timeinfo = std::localtime(&time_t);

    char buffer[256];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeinfo);
    
    return dir + "/" + prefix + "_" + buffer + "." + ext;
}