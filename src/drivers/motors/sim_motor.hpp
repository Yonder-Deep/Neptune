#pragma once
#include "motor.hpp"
#include <lgpio.h>
#include <httplib.h>

class SimulatedMotor : public Motor
{
   
    public:
    //At some point we should prob use this datatype everywhere, for now its here
     enum MotorLocation {
        FrontRight,
        FrontLeft,
        BackRight,
        BackLeft
    };
    MotorLocation ml;
    bool armed;
    SimulatedMotor(SimulatedMotor::MotorLocation ml, int speed = 0.0, float cycle = 50);
    int setSpeed(float speed) override;

    int setFrequency(float cycle) override;

    void armMotor() override;
    private:

    std::string GetMotorPath();
    const std::string SIM_ADDR = "http://localhost:6767"; 
    static httplib::Client* sim_connection;
};