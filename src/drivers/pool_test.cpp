//test 4 motors for the pool test

#include <iostream>
#include <chrono>
#include <thread>
#include "motor.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif


// 4 motors

int main() {
    #ifndef BUILD_SIMULATION
    int handle = lgGpiochipOpen(0); //open gpio chip 0

    if(handle < 0) {
        std::cerr << "failed to initialize lgpio" << std::endl;
        return 1;
    } else {
        int pin1 = 24;
        int pin2 = 11;
        int pin3 = 4;
        int pin4 = 18;

        Motor motor1(pin1, handle);
        Motor motor2(pin2, handle);
        Motor motor3(pin3, handle);
        Motor motor4(pin4, handle);

        // Initialize all motors (starts RT PWM threads)
        if (motor1.init() < 0 || motor2.init() < 0 ||
            motor3.init() < 0 || motor4.init() < 0) {
            std::cerr << "failed to initialize motors" << std::endl;
            motor1.cleanup();
            motor2.cleanup();
            motor3.cleanup();
            motor4.cleanup();
            lgGpiochipClose(handle);
            return 1;
        }

        // Arm all 4 ESCs: hold neutral (1500us) for 7 seconds before accepting commands
        std::cout << "Arming 4 ESCs (7 seconds at 1500us neutral)..." << std::endl;
        motor1.setPwm(1500);
        motor2.setPwm(1500);
        motor3.setPwm(1500);
        motor4.setPwm(1500);
        std::this_thread::sleep_for(std::chrono::seconds(7));
        std::cout << "ESCs armed. Ready for speed input." << std::endl;

        //for thruster speed
        //https://bluerobotics.com/store/thrusters/t100-t200-thrusters/t200-thruster-r2-rp/
        //for pwm and stuff
        //https://bluerobotics.com/learn/thruster-usage-guide/

        while(true) {
            int input1;
            int input2;
            int input3;
            int input4;

            bool stop = false;

            std::cout << "--- Motor Speed Input ---" << std::endl;
            std::cout << "1500 = stop" << std::endl;
            std::cout << "1530-1900 = forward (higher = faster)" << std::endl;
            std::cout << "1470-1100 = reverse (lower = faster)" << std::endl;
            std::cout << "Note: ~1475-1525 is the ESC dead zone; use wider offsets to actually move" << std::endl;

            std::cout << "Input speed for the front motor: " << std::endl;
            std::cin >> input1;
            std::cout << "Input speed for the right motor: " << std::endl;
            std::cin >> input2;
            std::cout << "Input speed for the left motor: " << std::endl;
            std::cin >> input3;
            std::cout << "Input speed for the back motor: " << std::endl;
            std::cin >> input4;

            if (motor1.setPwm(input1) < 0) stop = true;
            if (motor2.setPwm(input2) < 0) stop = true;
            if (motor3.setPwm(input3) < 0) stop = true;
            if (motor4.setPwm(input4) < 0) stop = true;

            std::cout << "stop test? (y/n)" << std::endl;
            char stopChar;
            std::cin >> stopChar;
            if(stopChar == 'y' || stopChar == 'Y') {
                stop = true;
            }

            if(stop) {
                motor1.cleanup();
                motor2.cleanup();
                motor3.cleanup();
                motor4.cleanup();

                lgGpiochipClose(handle);
                break;
            }
        }

    }
    #else
        std::cout << "Simulation mode. no hardware" << std::endl;
    #endif

    return 0;
}
