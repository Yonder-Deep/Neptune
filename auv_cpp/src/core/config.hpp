#pragma once
#include <yaml-cpp/yaml.h>

#include <stdexcept>
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

// PID (proportional, integral and deriv)
constexpr int CONTROL_LOOP_RATE = 0;

// SystemConfig
constexpr bool SIMULATION_MODE = true;
constexpr bool PERCEPTION_MODE = false;

// Structs
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

struct PIDGains {
    double kp = 0;
    double ki = 0;
    double kd = 0;
};

struct ControlConfig {
    PIDGains surge;
    PIDGains sway;
    PIDGains heave;
    PIDGains roll;
    PIDGains pitch;
    PIDGains yaw;

    double loop_hz = CONTROL_LOOP_RATE;
};

struct SystemConfig {
    bool simulation = SIMULATION_MODE;
    bool perception = PERCEPTION_MODE;
};
struct Config {
    MotorConfig motors;
    NetworkConfig network;
    SensorConfig sensors;
    ControlConfig control;
    SystemConfig system;

    /// Load config from YAML file. Missing values use defaults.
    static Config load(const std::string& path) {
        Config cfg;

        YAML::Node yaml;
        try {
            yaml = YAML::LoadFile(path);
        } catch (const YAML::Exception& e) {
            throw std::runtime_error("Failed to load config: " + std::string(e.what()));
        }

        // System
        if (yaml["system"]) {
            auto s = yaml["system"];
            if (s["simulation"])
                cfg.system.simulation = s["simulation"].as<bool>();
            if (s["perception"])
                cfg.system.perception = s["perception"].as<bool>();
        }

        // Motors
        if (yaml["motors"]) {
            auto m = yaml["motors"];
            if (m["pin1"])
                cfg.motors.pin1 = m["pin1"].as<int>();
            if (m["pin2"])
                cfg.motors.pin2 = m["pin2"].as<int>();
            if (m["pin3"])
                cfg.motors.pin3 = m["pin3"].as<int>();
            if (m["pin4"])
                cfg.motors.pin4 = m["pin4"].as<int>();
        }

        // Network
        if (yaml["network"]) {
            auto n = yaml["network"];
            if (n["ip"])
                cfg.network.ip = n["ip"].as<std::string>();
            if (n["port"])
                cfg.network.port = n["port"].as<int>();
        }

        // Sensors - GPS
        if (yaml["sensors"] && yaml["sensors"]["gps"]) {
            auto g = yaml["sensors"]["gps"];
            if (g["baudrate"])
                cfg.sensors.gps.baudrate = g["baudrate"].as<int>();
            if (g["timeout"])
                cfg.sensors.gps.timeout = g["timeout"].as<double>();
        }

        // Sensors - Depth
        if (yaml["sensors"] && yaml["sensors"]["depth"]) {
            auto d = yaml["sensors"]["depth"];
            if (d["i2c_addr"])
                cfg.sensors.depth.i2c_addr = d["i2c_addr"].as<int>();
            if (d["fluid_density"])
                cfg.sensors.depth.fluid_density = d["fluid_density"].as<double>();
        }

        // Sensors - IMU
        if (yaml["sensors"] && yaml["sensors"]["imu"]) {
            auto i = yaml["sensors"]["imu"];
            if (i["hard_iron_offset"] && i["hard_iron_offset"].size() == 3) {
                cfg.sensors.imu.hard_iron_offset = {i["hard_iron_offset"][0].as<double>(),
                                                    i["hard_iron_offset"][1].as<double>(),
                                                    i["hard_iron_offset"][2].as<double>()};
            }
            if (i["gyro_offset"] && i["gyro_offset"].size() == 3) {
                cfg.sensors.imu.gyro_offset = {i["gyro_offset"][0].as<double>(),
                                               i["gyro_offset"][1].as<double>(),
                                               i["gyro_offset"][2].as<double>()};
            }
            if (i["madgwick_gain"])
                cfg.sensors.imu.madgwick_gain = i["madgwick_gain"].as<double>();
        }

        // Control - PID gains helper lambda
        auto load_pid = [](const YAML::Node& node) -> PIDGains {
            PIDGains pid;
            if (node["kp"])
                pid.kp = node["kp"].as<double>();
            if (node["ki"])
                pid.ki = node["ki"].as<double>();
            if (node["kd"])
                pid.kd = node["kd"].as<double>();
            return pid;
        };

        if (yaml["control"]) {
            auto c = yaml["control"];
            if (c["loop_hz"])
                cfg.control.loop_hz = c["loop_hz"].as<double>();
            if (c["surge"])
                cfg.control.surge = load_pid(c["surge"]);
            if (c["sway"])
                cfg.control.sway = load_pid(c["sway"]);
            if (c["heave"])
                cfg.control.heave = load_pid(c["heave"]);
            if (c["roll"])
                cfg.control.roll = load_pid(c["roll"]);
            if (c["pitch"])
                cfg.control.pitch = load_pid(c["pitch"]);
            if (c["yaw"])
                cfg.control.yaw = load_pid(c["yaw"]);
        }

        return cfg;
    }

    /// Load with all defaults (no file needed)
    static Config load_default() { return Config{}; }
};
