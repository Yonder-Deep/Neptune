/**
@Header Motor
@brief sets speed of motor and can test it

motor.hpp is the header file for the motor class
it can set the speed of the motor and test it
*/

#pragma once

#ifndef BUILD_SIMULATION
    #include <pigpio.h>
#endif

#include <iostream>
#include <chrono>
#include <thread>

class Motor {
    private:
        int pin;
        double speed;
    
    public: 
        //hardcoded values from AUV
        const int CENTER_PWM_RANGE = 400;
        const int CENTER_PWM_VALUE = 1500;
        const int MAX_SPEED = 150;

        //1100 <- 1500 -> 1900

        //initialize gpio elsewhere with gpioInitialise()
        Motor(int gpioPin) : pin(gpioPin), speed(0.0) { }

        //set speed of motor
        void setSpeed(double newSpeed) {
            speed = newSpeed;

             //threshold for positive or negative speed
            if (speed > MAX_SPEED) {
                speed -= MAX_SPEED;
                speed *= -1;
            }

            //conversion from received radio speed to PWM value
            double pwmSpeed = speed * (CENTER_PWM_RANGE) / MAX_SPEED + CENTER_PWM_VALUE;

            //change motor speed
            #ifndef BUILD_SIMULATION
                gpioServo(pin, static_cast<unsigned int>(pwmSpeed));
            #else
                std::cout << "simulation Motor " << pin << " speed: " << " -> PWM: " << pwmSpeed << std::endl;
            #endif
        }

        //test the motor by setting speed values between time intervals
        void testMotor() {
            std::cout << ("TESTING MOTOR 2") << std::endl;
            this->setSpeed(MAX_SPEED / 3);
            std::this_thread::sleep_for(std::chrono::seconds(5));  
            this->setSpeed(0.0);
        }

};


