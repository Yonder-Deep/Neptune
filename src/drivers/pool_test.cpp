//test 4 motors for the pool test

#include <iostream>
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

            std::cout << "1500 will stop the thruster" << std::endl;
            std::cout << "1525 to 1900 is for forward thrust" << std::endl;
            std::cout << "1475 to 1100 is for backward thrust" << std::endl;

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
