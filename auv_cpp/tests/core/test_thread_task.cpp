#include "tasks/thread_task.hpp"
#include <iostream>
#include <cassert>
#include <atomic>
#include <chrono>
#include <thread>

// A simple task that increments a counter each loop iteration
struct CounterTask : ThreadTask {
    std::atomic<int> count{0};

    CounterTask() : ThreadTask("counter-task") {}

    void loop() override {
        count++;
        // Small sleep so we don't spin millions of times
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
};

void test_lifecycle() {
    std::cout << "\n=== Testing basic lifecycle ===\n";

    CounterTask task;
    assert(task.name == "counter-task");
    assert(task.count == 0);

    // startup spawns the thread, but it blocks until activate()
    task.startup();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    assert(task.count == 0);  // should still be 0 — not yet activated

    // activate — loop starts running
    task.activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(task.count > 0);  // should have run a few iterations
    std::cout << "  count after 100ms active: " << task.count << "\n";

    // shutdown + join
    task.shutdown();
    task.join();

    std::cout << "Lifecycle test passed!\n";
}

void test_deactivate_reactivate() {
    std::cout << "\n=== Testing deactivate/reactivate ===\n";

    CounterTask task;
    task.startup();
    task.activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(task.count > 0);

    // Deactivate — counter should stop
    task.deactivate();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    int count_paused = task.count.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    int count_still_stopped = task.count.load();
    // Allow at most 1 extra iteration (the one in-flight when deactivate was called)
    assert(count_still_stopped - count_paused <= 1);
    std::cout << "  count paused at: " << count_still_stopped << "\n";

    // Reactivate — counter should resume
    task.activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(task.count > count_still_stopped);
    std::cout << "  count after reactivate: " << task.count << "\n";

    task.shutdown();
    task.join();

    std::cout << "Deactivate/reactivate test passed!\n";
}

void test_destructor_cleanup() {
    std::cout << "\n=== Testing destructor cleanup ===\n";

    // If we forget shutdown()+join(), the destructor should handle it
    {
        CounterTask task;
        task.startup();
        task.activate();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // scope ends — destructor runs, should not hang or crash
    }

    std::cout << "Destructor cleanup test passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Nautilus AUV - Thread Task Tests\n";
    std::cout << "========================================\n";

    test_lifecycle();
    test_deactivate_reactivate();
    test_destructor_cleanup();

    std::cout << "\n========================================\n";
    std::cout << "All thread_task tests passed!\n";
    std::cout << "========================================\n";

    return 0;
}
