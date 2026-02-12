#include <iostream>

#include "../../src/core/config.hpp"

int main() {
    Config cfg = Config::load("../../data/config.yaml");
    std::cout << "motor pin 1: " << cfg.motors.pin1 << std::endl;
    std::cout << "gps baudrate: " << cfg.sensors.gps.baudrate << std::endl;
    std::cout << "imu gain: " << cfg.sensors.imu.madgwick_gain << std::endl;
    return 0;
}
