#include "core/time_utils.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <cmath>

void test_now() {
    std::cout << "\n=== Testing now() ===\n";

    TimePoint a = now();
    TimePoint b = now();
    // b should be >= a (time moves forward)
    assert(b >= a);

    std::cout << "now() tests passed!\n";
}

void test_elapsed_seconds() {
    std::cout << "\n=== Testing elapsed_seconds() ===\n";

    TimePoint start = now();
    // Sleep ~50ms
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    TimePoint end = now();

    double dt = elapsed_seconds(start, end);
    // Should be roughly 50ms (allow 20-200ms range for CI/Docker variability)
    assert(dt > 0.02);
    assert(dt < 0.2);
    std::cout << "  elapsed: " << dt * 1000.0 << " ms\n";

    std::cout << "elapsed_seconds() tests passed!\n";
}

void test_wcet_tracker() {
    std::cout << "\n=== Testing WCETTracker ===\n";

    WCETTracker tracker;

    // Initial state
    assert(tracker.worst_ns() == 0);
    assert(tracker.mean_ns() == 0.0);

    // Run a few measurements
    for (int i = 0; i < 10; i++) {
        tracker.start();
        // Burn some time — volatile prevents optimizer from removing the loop
        volatile int x = 0;
        for (int j = 0; j < 10000; j++) { x += j; }
        tracker.stop();
    }

    assert(tracker.count == 10);
    assert(tracker.worst_ns() > 0);
    assert(tracker.mean_ns() > 0.0);
    // Worst must be >= mean
    assert(tracker.worst_ns() >= static_cast<int64_t>(tracker.mean_ns()));
    std::cout << "  worst: " << tracker.worst_ns() << " ns\n";
    std::cout << "  mean:  " << tracker.mean_ns() << " ns\n";
    std::cout << "  count: " << tracker.count << "\n";

    // Reset
    tracker.reset();
    assert(tracker.worst_ns() == 0);
    assert(tracker.mean_ns() == 0.0);
    assert(tracker.count == 0);

    std::cout << "WCETTracker tests passed!\n";
}

void test_rate_loop_timer() {
    std::cout << "\n=== Testing RateLoopTimer ===\n";

    // 100 Hz = 10ms period (use lower freq so the test isn't flaky in Docker)
    RateLoopTimer timer(100.0);
    constexpr int NUM_ITERATIONS = 5;
    double total_dt = 0.0;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        timer.tick();
        // Simulate some work (~1ms)
        volatile int x = 0;
        for (int j = 0; j < 100000; j++) { x += j; }
        double dt = timer.wait();
        total_dt += dt;
        // Each iteration should be ~10ms (allow 8-20ms range)
        assert(dt > 0.008);
        assert(dt < 0.020);
    }

    double avg = total_dt / NUM_ITERATIONS;
    std::cout << "  avg dt: " << avg * 1000.0 << " ms (target: 10.0 ms)\n";
    // Average should be close to 10ms
    assert(std::abs(avg - 0.01) < 0.002);

    // Test overrun detection: set a very high frequency that work will exceed
    RateLoopTimer fast_timer(1000000.0);  // 1 MHz = 1µs period
    fast_timer.tick();
    // Do enough work to exceed 1µs
    volatile int y = 0;
    for (int j = 0; j < 100000; j++) { y += j; }
    fast_timer.wait();
    assert(fast_timer.overran());

    std::cout << "RateLoopTimer tests passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Nautilus AUV - Time Utils Tests\n";
    std::cout << "========================================\n";

    test_now();
    test_elapsed_seconds();
    test_wcet_tracker();
    test_rate_loop_timer();

    std::cout << "\n========================================\n";
    std::cout << "All time_utils tests passed!\n";
    std::cout << "========================================\n";

    return 0;
}
