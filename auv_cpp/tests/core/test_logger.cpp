#include <thread>
#include <vector>
#include <chrono>
#include <iostream>

#include "logger.hpp"
#include "types.hpp"   // LogSource

using namespace neptune;

// need to link fmt to run spdlog, compile using:
// g++ test_logger.cpp -lspdlog -lfmt -pthread -o tl

void thread_worker(int id)
{
    for (int i = 0; i < 10; ++i) {
        log_debug(LogSource::NAV,
                  "Worker " + std::to_string(id) +
                  " iteration " + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main()
{

    // 1. Logging before initialization (must be safe no-op)
   
    log_info(LogSource::MAIN, "This should NOT crash (pre-init)");

    // 2. Initialize logger (idempotent)

    log_init("test.log", LogLevel::Info, LogLevel::Debug);

    // Call again to verify idempotence
    log_init("test.log", LogLevel::Info, LogLevel::Debug);

    log_info(LogSource::MAIN, "Logger initialized");


    // all log levels
    log_debug   (LogSource::CTRL, "Debug message");
    log_info    (LogSource::CTRL, "Info message");
    log_warn    (LogSource::CTRL, "Warning message");
    log_error   (LogSource::CTRL, "Error message");
    log_critical(LogSource::CTRL, "Critical message");


    // Multi-threaded logging

    constexpr int num_threads = 4;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(thread_worker, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    log_info(LogSource::MAIN, "All worker threads finished");


    // Shutdown
    log_shutdown();

    // Logging after shutdown should be safe no-op
    log_error(LogSource::MAIN, "This should not crash (post-shutdown)");

    std::cout << "Logger test completed successfully.\n";
    std::cout << "Check console output and test.log for correctness.\n";

    return 0;
}
