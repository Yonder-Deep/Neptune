//test 4 motors for the pool test

#include <iostream>
#include "motor.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif


// 4 motors 
// don't know what pins they're on
//also don't know gpio chip number but I assume it's 0

int main() {
    #ifndef BUILD_SIMULATION
    int handle = lgGpiochipOpen(0); //open gpio chip 0 

    if(handle < 0) {
        std::cerr << "failed to initialize lgpio" << std::endl;
        return 1;
    } else {
        //placeholder values
        int pin1 = 24; 
        int pin2 = 11;
        int pin3 = 4;
        int pin4 = 18;

        Motor motor1(pin1, handle);
        int result = motor1.claimPin();
        if(result < 0) {
            std::cerr << "failed to claim output" << std::endl;
            lgGpiochipClose(handle);
            return 1;
        }

        Motor motor2(pin2, handle);
        result = motor2.claimPin();
        if(result < 0) {
            std::cerr << "failed to claim output" << std::endl;
            motor1.freePin();
            lgGpiochipClose(handle);
            return 1;
        }

        Motor motor3(pin3, handle);
        result = motor3.claimPin();
        if(result < 0) {
            std::cerr << "failed to claim output" << std::endl;
            motor1.freePin();
            motor2.freePin();
            lgGpiochipClose(handle);
            return 1;
        }

        Motor motor4(pin4, handle);
        result = motor4.claimPin();
        if(result < 0) {
            std::cerr << "failed to claim output" << std::endl;
            motor1.freePin();
            motor2.freePin();
            motor3.freePin();
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

            result = motor1.setPwm(input1);
            if(result < 0) {
                std::cerr << "failed to set pwm" << std::endl;
                stop = true;
            }

            result = motor2.setPwm(input2);
            if(result < 0) {
                std::cerr << "failed to set pwm" << std::endl;
                stop = true;
            }

            result = motor3.setPwm(input3);
            if(result < 0) {
                std::cerr << "failed to set pwm" << std::endl;
                stop = true;
            }

            result = motor4.setPwm(input4);
            if(result < 0) {
                std::cerr << "failed to set pwm" << std::endl;
                stop = true;
            }

            std::cout << "stop test? (y/n)" << std::endl;
            char stopChar;
            std::cin >> stopChar;
            if(stopChar == 'y' || stopChar == 'Y') {
                stop = true;
            }

            if(stop) {
                motor1.stopMotor();
                motor2.stopMotor();
                motor3.stopMotor();
                motor4.stopMotor();

                motor1.freePin();
                motor2.freePin();
                motor3.freePin();
                motor4.freePin();

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
