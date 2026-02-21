/**
@Class Main

tests the motor class
*/


//script for testing the motor class

#include <iostream>
#include "motor.hpp"
//#include "imu.hpp"

#ifndef BUILD_SIMULATION
    #include <pigpio.h>
#endif

int main() {

    #ifndef BUILD_SIMULATION
        if(gpioInitialise() < 0) {
            std::cerr << "failed to initialize pigpio" << std::endl;
            return 1;
        }
    #else
        std::cout << "Simulation mode. no hardware" << std::endl;
    #endif

    //test each motor
    for(int i = 0; i < 50; i++) {
       try {
        Motor motor(i);
        motor.testMotor();
        std::cout << "Motor: " << i << std::endl;
       } catch(...) {
        std::cout << "Skipped: " << i << std::endl;
       }
    }

    #ifndef BUILD_SIMULATION
        gpioTerminate();
    #endif

    return 0;
}