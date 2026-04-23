#include "thruster.hpp"
#include <lgpio.h>
#include <thread>
#include <stdexcept>
#include <iostream>


Thruster::Thruster(int pin, int handle, int speed /*= 0*/, int cycle /*= 50*/)  : pin(pin) , handle(handle), Motor(speed, cycle) {
    if(lgGpioClaimOutput(handle, 0, pin, 0) !=LG_OKAY ){
        throw new std::runtime_error("Failed to claim a pin for a motor");
    }
}

Thruster::~Thruster(){
    if(lgGpioFree(handle, pin) <0) {
        std::cerr << "Error code freeing pin " << pin << std::endl;
    }
}
float Thruster::getDutyCycle()
{
    // Convert to nanoseconds
    float cycle_width = 1 / (this->cycle * (1e-6)) ;
    float goal_interval = ((MAX_PWM_DIST * this->speed) + STOP_PWM);
    return (goal_interval / cycle_width) * 100;
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
void Thruster::armMotor(){
    this->speed = 0.0;
    this->armed = true;
    if(this->sendMotorPWM() < 0){
        //Maybe error here?
    }
    lguSleep(ARM_SECONDS);
}
