/**
@Class Main

Tests the motor class using hardware PWM via sysfs.

Requires:
  - dtoverlay=pwm-2chan in /boot/firmware/config.txt
  - Motor wired to GPIO 18 (PWM channel 0)
  - Run with sudo (sysfs PWM needs root)
*/

#include <iostream>
#include <chrono>
#include <thread>
#include "motor.hpp"

int main() {
    // Channel 0 = GPIO 18, Channel 1 = GPIO 19
    Motor motor(0);

    int result = motor.init();
    if (result < 0) {
        std::cerr << "failed to initialize motor" << std::endl;
        return 1;
    }

    std::cout << "Motor initialized. Arming ESC (7 seconds at neutral)..." << std::endl;

    // Arm ESC: hold neutral (1500us) for 7 seconds
    result = motor.setPwm(1500);
    if (result < 0) {
        std::cerr << "failed to set neutral" << std::endl;
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(7));

    // Forward at lowest speed (1525us) for 5 seconds
    std::cout << "Forward (1525)..." << std::endl;
    result = motor.setPwm(1525);
    if (result < 0) {
        std::cerr << "failed to set forward" << std::endl;
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Neutral for 3 seconds
    std::cout << "Neutral (1500)..." << std::endl;
    motor.setPwm(1500);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Reverse at lowest speed (1475us) for 5 seconds
    std::cout << "Reverse (1475)..." << std::endl;
    result = motor.setPwm(1475);
    if (result < 0) {
        std::cerr << "failed to set reverse" << std::endl;
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Stop
    std::cout << "Stopping..." << std::endl;
    motor.stopMotor();
    motor.cleanup();

    std::cout << "Test complete." << std::endl;
    return 0;
}
