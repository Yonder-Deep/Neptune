// main.cpp
#include "common/config.hpp"
#include "drivers/gps/gps.hpp"
#include "drivers/gps/hardware_gps.hpp"
#include "drivers/imu/compass.hpp"
#include "drivers/motors/motor.hpp"
#include "drivers/motors/thruster.hpp"
#include "neptune.hpp"
#include "states/menu.hpp"

#include "lgpio.h"

#include <array>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

/// This configuration is loaded at startup to easily change the program's behavior.
/// We use reflect-cpp (<rfl/yaml.hpp>) to load the `config.yaml` in a typesafe way, and
/// immediately terminate the program if there is a malformed configuration.
struct Config {
    std::string gps_port;    // Path to gps, likely /dev/ttyACM0
    std::array<int, 4> pins; // front_left, front_right, back_left, back_right
};

int main() {
    auto config = expect(load_config<Config>("config.yaml"), "Config loading failed");

    GPS *gps = new HardwareGPS(config.gps_port);
    // Hardcoded since I dont think this will ever change on the pi
    if (config.pins.size() != 4) {
        std::cout << "Wrong number of input pins for motors:" << config.pins.size() << "\n";
        return 1;
    }
    int handle = lgGpiochipOpen(0);
    Motor *front_left = new Thruster(config.pins[0], handle);
    Motor *front_right = new Thruster(config.pins[1], handle);
    Motor *back_left = new Thruster(config.pins[2], handle);
    Motor *back_right = new Thruster(config.pins[3], handle);
    Neptune *neptune = new Neptune();
    neptune->gps = gps;
    neptune->back_left = back_left;
    neptune->back_right = back_right;
    neptune->front_left = front_left;
    neptune->front_right = front_right;
    neptune->compass = new Compass();
    neptune->state = new Menu();
    std::signal(SIGINT, [](int signum) {
        Neptune::instance->stop();
        exit(0);
    });
    neptune->start();
}
