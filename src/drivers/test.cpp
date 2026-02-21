#include <pigpio.h>

#include <iostream>
#include <chrono>
#include <thread>

int main() {
    if(gpioInitialise() < 0) {
        std::cerr << "initialize pigpio failed" << std::endl;
        return 1; //so it exits and doesn't fall through to the else
    } else {

        //forward (1900 is max pwm)
        double pwm = 1700;
        //pin 1 tends to be 12C like IMU
        int pin = 3 //not sure what pin the motor is attached to yet
        gpioServo(pin, static_cast<unsigned int>(pwm));
        std::this_thread::sleep_for(std::chrono::seconds(5));

        //neutral 
        pwm = 1500;
        gpioServo(pin, static_cast<unsigned int>(pwm));
        std::this_thread::sleep_for(std::chrono::seconds(5));

        //backward (1100 is min pwm)
        pwm = 1300;
        gpioServo(pin, static_cast<unsigned int>(pwm));
        std::this_thread::sleep_for(std::chrono::seconds(5));

        gpioTerminate();
        return 0;
    }
}
