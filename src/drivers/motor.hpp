/**
@Header Motor
@brief sets speed of motor and can test it

motor.hpp is the header file for the motor class
it can set the speed of the motor and test it
*/

#pragma once


#include <iostream>
#include <chrono>
#include <thread>

//hopefully should work on the pi. It works in simulation. Otherwise it won't build
#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

class Motor {
    private:
        int pin;
        int handle;
        int pwm;
    
    public: 
        //hardcoded values from AUV
        const int CENTER_PWM_RANGE = 400;
        const int CENTER_PWM_VALUE = 1500;

        //1100 <- 1500 -> 1900

        //gpio chip is opened/closed eleswhere
        Motor(int gpioPin, int handle) : pin(gpioPin), handle(handle), pwm(1500) { }

        int claimPin() {
            #ifndef BUILD_SIMULATION
                int result = lgGpioClaimOutput(handle, 0, pin, 0); //claim pin for motor
                return result;
            #else
                return 0;
            #endif
        }

        //set speed of motor
        int setPwm(int newPwm) {
            pwm = newPwm;

            if(pwm < 1100 || pwm > 1900) {
                std::cerr << "pwm value out of range" << std::endl;
                return -1;
            }

            //change motor speed
            #ifndef BUILD_SIMULATION
                //lgTxServo isn't good: int result = lgTxServo(handle, pin, pwm, 50, 0, 0);
                //duty cycle can be set from 0 to 100. 100 is for full power 0 is off
                //int result = lgTxPwm(handle, pin, pwm, 9, 0, 0); 

                int pulseOn = pwm;
                int pulseOff = 20000 - pulseOn; //assume 50Hz

                int result = lgTxPulse(handle, pin, pulseOn, pulseOff, 0, 0);
                return result;
            #else
                std::cout << "simulation Motor " << pin << " PWM: " << pwm << std::endl;
                return 0;
            #endif
        }

        void stopMotor() {
            #ifndef BUILD_SIMULATION
                //apparently not great according to documentation: lgTxServo(handle, pin, 0, 50, 0, 0); 
                // //turn off pulsewidths
                //lgTxPwm(handle, pin, 1500, 8, 0, 0); 
                int pulseOn = 1500;
                int pulseOff = 20000 - pulseOn;
                lgTxPulse(handle, pin, pulseOn, pulseOff, 0, 0);
            #else
                std::cout << "simulation Motor " << pin << " stopped" << std::endl;
            #endif
        }

        int freePin() {
            #ifndef BUILD_SIMULATION
                int result = lgGpioFree(handle, pin);
                return result;
            #else
                return 0;
            #endif
        }

        //test the motor by setting speed values between time intervals
        void testMotor() {
            std::cout << ("TESTING MOTOR 2") << std::endl;
            this->setPwm(1700);
            std::this_thread::sleep_for(std::chrono::seconds(5));  
            this->stopMotor();
        }

};


