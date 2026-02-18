#pragma once
#include "stepper_motor.hpp"
#include <cstdint>

class Winch {
private:
    Stepper& stepper;

    // optional safety limits
    float max_sps;

public:
    explicit Winch(Stepper& s);

    void init();
    void set_speed(float steps_per_sec);

    void reel_in_steps(int32_t steps);
    void reel_out_steps(int32_t steps);

    void stop();
};
