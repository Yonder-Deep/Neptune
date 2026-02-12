#pragma once
#include <string>

#include "types.hpp"

// TODO: These values need calibration before deployment

// Motors (GPIO pins)
constexpr int MOTORPIN1 = 0;
constexpr int MOTORPIN2 = 0;
constexpr int MOTORPIN3 = 0;
constexpr int MOTORPIN4 = 0;

// Network
constexpr int PORT = 0;

// Sensor GPS
constexpr int BAUDRATE = 0;
constexpr double TIMEOUT = 0;

// Sensor Depth
constexpr int DEPTH_SENSOR_ADDR = 0x00;
constexpr double WATER_DENSITY = 0;

// Sensor IMU
constexpr double MADGWICK_GAIN = 0;

struct MotorConfig {
    int pin1 = MOTORPIN1;
    int pin2 = MOTORPIN2;
    int pin3 = MOTORPIN3;
    int pin4 = MOTORPIN4;
};

struct NetworkConfig {
    std::string ip = "";  // TODO: Set before deployment
    int port = PORT;
};

struct GPSConfig {
    int baudrate = BAUDRATE;
    double timeout = TIMEOUT;
};

struct DepthConfig {
    int i2c_addr = DEPTH_SENSOR_ADDR;
    double fluid_density = WATER_DENSITY;
};

struct IMUConfig {
    Vec3 hard_iron_offset = {0, 0, 0};
    Mat3x3 soft_iron_matrix = {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
    Vec3 gyro_offset = {0, 0, 0};
    double madgwick_gain = MADGWICK_GAIN;
};

struct SensorConfig {
    GPSConfig gps;
    DepthConfig depth;
    IMUConfig imu;
};

struct Config {
    MotorConfig motors;
    NetworkConfig network;
    SensorConfig sensors;

    static Config load(const std::string& path) {
        Config cfg;
        return cfg;
    }
};
