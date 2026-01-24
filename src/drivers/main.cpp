//script for testing the motor class

#include <iostream>
#include "motor.hpp"

#ifndef BUILD_SIMULATION
#include <pigpio.h>
#endif


int main() {

    #ifndef BUILD_SIMULATION
    //initialize pigpio 
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
        motor.test_motor();
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