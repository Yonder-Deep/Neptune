#include "../../src/core/ring_buffer.hpp"
#include <iostream>
#include <cassert>
#include <thread>

void test_single_threaded() {
    SPSC<int, 8> buf;

    // push 3 items
    assert(buf.push(10));
    assert(buf.push(20));
    assert(buf.push(30));

    // pop in order (FIFO)
    assert(buf.pop() == 10);
    assert(buf.pop() == 20);
    assert(buf.pop() == 30);

    // empty
    assert(!buf.pop().has_value());

    // fill it up (7 items in 8-slot buffer)
    for (int i = 0; i < 7; i++) {
        assert(buf.push(i));
    }
    assert(!buf.push(99));  // 8th push fails, full

    std::cout << "Single-threaded tests passed!" << std::endl;
}

void test_multi_threaded() {
    constexpr int NUM_ITEMS = 100000;
    SPSC<int, 1024> buf;

    // Producer thread: push 0, 1, 2, ... 99999 in order
    std::thread producer([&buf]() {
        for (int i = 0; i < NUM_ITEMS; i++) {
            while (!buf.push(i)) {
                // buffer full, keep retrying
            }
        }
    });

    // Consumer (main thread): pop all items, verify ordering
    int expected = 0;
    while (expected < NUM_ITEMS) {
        auto val = buf.pop();
        if (val.has_value()) {
            assert(*val == expected);
            expected++;
        }
        // empty, retry
    }

    producer.join();
    std::cout << "Multi-threaded test passed! (" << NUM_ITEMS << " items)" << std::endl;
}

int main() {
    test_single_threaded();
    test_multi_threaded();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
