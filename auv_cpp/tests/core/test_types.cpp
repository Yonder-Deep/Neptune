#include <iostream>
#include "core/types.hpp"
#include <cassert>

template<typename T, size_t N>
void print_array(const std::array<T, N>& arr, const char* name) {
    std::cout << name << ": [";
    for (size_t i = 0; i < N; ++i) {
        std::cout << arr[i];
        if (i < N - 1) std::cout << ", ";
    }
    std::cout << "]\n";
}

void test_state() {
    std::cout << "\n=== Testing State ===\n";

    // Default construction
    State s1;
    assert(s1.position[0] == 0.0);
    assert(s1.velocity[2] == 0.0);

    // Parameterized construction
    State s2({10.0, 20.0, -5.0}, {1.0, 0.5, 0.0}, {0.0, 5.0, 90.0}, {0.0, 0.0, 0.1});
    assert(s2.position[0] == 10.0);
    assert(s2.attitude[2] == 90.0);

    // Modification
    s1.position[2] = -3.0;
    assert(s1.position[2] == -3.0);

    std::cout << "State tests passed!\n";
}

void test_motor_speeds() {
    std::cout << "\n=== Testing MotorSpeeds ===\n";

    // Default
    MotorSpeeds m1;
    assert(m1.forward == 0.0);

    // Valid
    MotorSpeeds m2(0.5, -0.3, 0.0, 0.8);
    assert(m2.forward == 0.5);

    // Invalid should throw
    bool caught = false;
    try {
        MotorSpeeds m3(1.5, 0.0, 0.0, 0.0);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    // Clamped
    MotorSpeeds m4 = MotorSpeeds::clamped(1.5, -2.0, 0.5, 0.0);
    assert(m4.forward == 1.0);
    assert(m4.turn == -1.0);

    std::cout << "MotorSpeeds tests passed!\n";
}

void test_log() {
    std::cout << "\n=== Testing Log ===\n";

    Log log1(LogSource::CTRL, "info", "Control loop started");
    assert(log1.source == LogSource::CTRL);
    assert(log1.dest == LogDest::LOG);

    State s;
    s.position = {1.0, 2.0, 3.0};
    Log log2(LogSource::LCAL, "state", s, LogDest::BASE);
    assert(log2.dest == LogDest::BASE);

    assert(std::holds_alternative<std::string>(log1.content));
    assert(std::holds_alternative<State>(log2.content));

    std::cout << "Log tests passed!\n";
}

void test_promise() {
    std::cout << "\n=== Testing Promise ===\n";

    bool executed = false;
    Promise p("test", 0.0, [&executed]() { executed = true; });

    assert(p.name == "test");
    if (p.is_ready()) {
        p.callback();
    }
    assert(executed);

    Promise p2("delayed", 1.0, []() {});
    assert(!p2.is_ready());

    std::cout << "Promise tests passed!\n";
}

void test_inheritance() {
    std::cout << "\n=== Testing Inheritance ===\n";

    ExpandedState es;
    es.position = {1.0, 2.0, 3.0};
    es.local_force = {10.0, 0.0, 5.0};
    assert(es.position[0] == 1.0);
    assert(es.local_force[0] == 10.0);

    InitialState is;
    is.mass = 10.0;
    is.inertia[0][0] = 1.0;
    assert(is.mass == 10.0);

    std::cout << "Inheritance tests passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Nautilus AUV - Core Types Tests\n";
    std::cout << "========================================\n";

    test_state();
    test_motor_speeds();
    test_log();
    test_promise();
    test_inheritance();

    std::cout << "\n========================================\n";
    std::cout << "All tests passed!\n";
    std::cout << "========================================\n";

    return 0;
}
