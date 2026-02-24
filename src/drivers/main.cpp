/**
@Class Main

tests the motor class
*/


//script for testing the motor class

#include <iostream>
#include <chrono>
#include "motor.hpp"
//#include "imu.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

int main() {

    #ifndef BUILD_SIMULATION
        //need to find RP1 controller (varies with kernel/OS)

        /*
        sudo apt update
        sudo apt install -y build-essential cmake liblgpio-dev
        cd src
        cmake -B build
        cmake --build build
        ./build/motor_test

        sudo apt install gpiod
        sudo gpiodetect

        set gpio chip to the one for [pinctrl-rp1]

        could also make a helper function later to find gpio chip
        */

        int handle = lgGpiochipOpen(0); //open gpio chip 0
        if(handle < 0) {
            std::cerr << "failed to initialize lgpio" << std::endl;
            return 1;
        } else {
            int pin = 3;

            Motor motor(pin, handle);

            int result = motor.claimPin();
            if(result < 0) {
                std::cerr << "failed to claim output" << std::endl;
                return 1;
            }

            result = motor.setPwm(1700);
            if(result < 0) {
                std::cerr << "failed to set pwm" << std::endl;
                return 1;
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
            result = motor.setPwm(1500);
            if(result < 0) {
                std::cerr << "failed to set pwm" << std::endl;
                return 1;
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
            result = motor.setPwm(1300);
            if(result < 0) {
                std::cerr << "failed to set pwm" << std::endl;
                return 1;
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));

            motor.stopMotor();
            motor.freePin();

            lgGpiochipClose(handle);

        }
    #else
        std::cout << "Simulation mode. no hardware" << std::endl;
    #endif

    //test each pin. Probably won't need this
    /*
        for(int i = 0; i < 50; i++) {
       try {
        lgGpioClaimOutput(handle, 0, i, 0);

        Motor motor(i, handle);
        motor.testMotor();
        std::cout << "Motor: " << i << std::endl;
       } catch(...) {
        std::cout << "Skipped: " << i << std::endl;
       }
    }
       */

    return 0;
}