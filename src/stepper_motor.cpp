#include "stepper_motor.hpp"

#include <cmath>
#include <chrono>
#include <thread>

#ifndef BUILD_SIMULATION
    #include <pigpio.h>
#endif

Stepper::Stepper(int stepPin, int dirPin, int enablePin, bool enableActiveLow) : step_pin(stepPin),
      dir_pin(dirPin),
      en_pin(enablePin),
      en_active_low(enableActiveLow),
      steps_per_sec(500.0f),
      stop_requested(false){

      }

void Stepper::write_pin(int pin, bool value) {
#ifndef BUILD_SIMULATION
     gpioWrite(pin, value ? 1 : 0);
#else
      (void) pin;
    (void) value;
#endif
}

void Stepper::init() {
#ifndef BUILD_SIMULATION
    gpioSetMode(step_pin, PI_OUTPUT);
     gpioSetMode(dir_pin, PI_OUTPUT);
    if (en_pin >= 0)
      gpioSetMode(en_pin, PI_OUTPUT);
#endif

    disable();
    write_pin(step_pin, false);
}

void Stepper::enable() {
    if (en_pin >= 0)
        write_pin(en_pin, en_active_low ? false : true);
}

void Stepper::disable() {
    if (en_pin >= 0)
        write_pin(en_pin, en_active_low ? true : false);
}

void Stepper::set_speed(float sps) {
    if (sps < 1.0f)
        sps = 1.0f;
    steps_per_sec = sps;
}

void Stepper::set_direction(bool dir) {
    write_pin(dir_pin, dir);
}

void Stepper::step_once() {
    write_pin(step_pin, true);

#ifndef BUILD_SIMULATION
    gpioDelay(3);  
#else
    std::this_thread::sleep_for(std::chrono::microseconds(3));
#endif

    write_pin(step_pin, false);
}

void Stepper::move_steps(int32_t steps) {
    if (steps == 0)
        return;

    stop_requested = false;
    enable();

    bool dir = (steps > 0);
    set_direction(dir);

    int32_t total_steps = std::abs(steps);

    float period_us = 1e6f / steps_per_sec;
    if (period_us < 50.0f)
        period_us = 50.0f;  

    for (int32_t i = 0; i < total_steps; ++i) {
        if (stop_requested)
            break;

        step_once();

#ifndef BUILD_SIMULATION
        gpioDelay(static_cast  <unsigned int> (period_us));
#else
        std::this_thread::sleep_for(
            std::chrono::microseconds(static_cast <int>  (period_us)));
#endif
    }
}

void Stepper::stop() {
    stop_requested = true;
}
