#pragma once
#include <string>

struct MotorConfig {
    int motor1pin = 4;  // From the python code
    int motor2pin = 11;
    int motor3pin = 18;
    int motor4pin = 24;
};
struct NetworkConfig {
    std::string IP = "";
    int port = 8080;
};

struct Config {
    MotorConfig motors;
    NetworkConfig system;

    static Config load(const std::string& path) {
        Config cfg;
        return cfg;
    }
};
