/**
@Class Main

Tests the motor class using RT-threaded PWM generation.
Run with sudo for real-time scheduling priority.

Build:
  cd src
  cmake -B build
  cmake --build build
  sudo ./build/motor_test
*/

#include <iostream>
#include <chrono>
#include <thread>
#include "motor.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

int main() {
    #ifndef BUILD_SIMULATION
        int handle = lgGpiochipOpen(0);
        if (handle < 0) {
            std::cerr << "failed to initialize lgpio" << std::endl;
            return 1;
        }

        int pin = 18; // GPIO 18 = physical pin 12

        Motor motor(pin, handle);

        int result = motor.init();
        if (result < 0) {
            std::cerr << "failed to initialize motor" << std::endl;
            lgGpiochipClose(handle);
            return 1;
        }

        // Arm ESC: hold neutral (1500us) for 7 seconds
        std::cout << "Arming ESC (7 seconds at neutral)..." << std::endl;
        motor.setPwm(1500);
        std::this_thread::sleep_for(std::chrono::seconds(7));

        // Forward at lowest speed (1525us) for 5 seconds
        std::cout << "Forward (1525)..." << std::endl;
        motor.setPwm(1525);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Neutral for 3 seconds
        std::cout << "Neutral (1500)..." << std::endl;
        motor.setPwm(1500);
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // Reverse at lowest speed (1475us) for 5 seconds
        std::cout << "Reverse (1475)..." << std::endl;
        motor.setPwm(1475);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Stop
        std::cout << "Stopping..." << std::endl;
        motor.cleanup();
        lgGpiochipClose(handle);

    #else
        std::cout << "Simulation mode. no hardware" << std::endl;
    #endif

    std::cout << "Test complete." << std::endl;
    return 0;
}
