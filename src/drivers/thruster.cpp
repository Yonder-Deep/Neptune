    #include "thruster.hpp"
#include <lgpio.h>
#include <thread>
#include <stdexcept>

float Thruster::getDutyCycle()
{
    // Convert to nanoseconds
    float cycle_width = (1 / this->cycle) * (1e-9);
    float goal_interval = ((MAX_PWM_DIST * this->speed) + STOP_PWM);
    return goal_interval / cycle_width;
}

int Thruster::sendMotorPWM()
{
    if(!this->armed){
        throw new std::logic_error("Tried to affect motor before arming. Always ensure motors are armed before activating them");
    }
    return lgTxPwm(this->handle, this->pin, this->cycle, getDutyCycle(), 0, 0);

}

int Thruster::setSpeed(float newSpeed) {
    this->speed = newSpeed;
    if(newSpeed < -1.0 || newSpeed > 1.0){
        //TODO clamp here instead   ? 
        throw new std::logic_error("Motor Speed out of range");
    }
    return sendMotorPWM();
}

int Thruster::setFrequency(float cycle)
{
    this->cycle = cycle;
    return sendMotorPWM();
}
