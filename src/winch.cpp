#include "winch.hpp"
//sps = steps per second

Winch::Winch(Stepper& s)
    : stepper(s), max_sps(2000.0f) {}

void Winch::init() {
    stepper.init();
    stepper.enable();
}

void Winch::set_speed(float sps) {
    if (sps > max_sps) sps = max_sps;
    stepper.set_speed(sps);
}

void Winch::reel_in_steps(int32_t steps) {
    if (steps < 0) steps = -steps;
    stepper.move_steps(+steps);
}

void Winch::reel_out_steps(int32_t steps) {
    if (steps < 0) steps = -steps;
    stepper.move_steps(-steps);
}

void Winch::stop() {
    stepper.stop();
}
