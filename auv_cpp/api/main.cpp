//script for testing the motor class

#include <iostream>
#include <pigpio.h>
#include "motor.hpp"

int main() {

    //initialize pigpio 
    if(gpioInitialise() < 0) {
        std::cerr << "failed to initialize pigpio" << std::endl;
        return 1;
    }

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

    gpioTerminate();
    return 0;
}