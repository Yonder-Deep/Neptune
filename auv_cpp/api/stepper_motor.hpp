#pragma once
#include <cstdint>
#include <atomic>

/**
 * @class Stepper
 * @brief Simple step/dir stepper motor controller
 *
 * Designed for STEP/DIR drivers (A4988, DRV8825, TMC, etc.)
 * Works on Raspberry Pi using pigpio, and on Windows using BUILD_SIMULATION.
 */
class Stepper {
private:
    int step_pin;
    int dir_pin;
    int en_pin;              // -1 if unused 
    bool en_active_low;

    float steps_per_sec;
    std::atomic<bool> stop_requested;

    void write_pin(int pin, bool value);

public:
    Stepper(int stepPin, int dirPin, int enablePin = -1, bool enableActiveLow = true);

    void init();
    void enable();
    void disable();

    void set_speed(float steps_per_sec);
    void set_direction(bool dir);

    void step_once();
    void move_steps(int32_t steps);

    void stop();  // immediately stops motion
};
