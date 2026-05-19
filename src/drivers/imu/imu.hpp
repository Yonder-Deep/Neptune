#pragma once

#include "lsm6dsox_reg.h"
#include "lis3mdl_reg.h"
#include <chrono>
#include <cstdint>
#include<stdexcept>
#include "platform_i2c.hpp"
#include <mutex>


#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

//lsm6dsox is aceleromter and gyroscope
//lis3mdl is magnetometer

struct ImuConfig {
    int bus = 1; //assume i2c bus is 1
    uint8_t ag_addr = 0x6A; //assume lsm6dsox address is 0x6A
    uint8_t mag_addr = 0x1C; //assume lis3mdl address is 0x1C

    //imufusion library https://github.com/xioTechnologies/Fusion/blob/main/Fusion/FusionAhrs.c
    //following values are hard coded from imu.py
    float accel_reject = 10.0f; ///90.0f for default in library
    float mag_reject = 10.0f; //90.0f for default in library
    float gain = .5f; 
};

struct Vec3f {float x, y, z; };
struct EulerDeg {float roll, pitch, yaw; };

class IMU {
public:
    explicit IMU(const ImuConfig& cfg = {}); 
    ~IMU();

    void updateImuReading(); 
    EulerDeg readEuler() const;
    
    Vec3f readAccel() const;
    Vec3f readGyro() const;
    Vec3f readMag() const;

private:
    void openDevices_();
    void verifyWhoAmI_();
    void configureSensors_();
    void closeDevices_();

    //handles
    platformHandleT ag_handle_{};
    platformHandleT m_handle_{};

    //contexts from lis3mdl and lsm6dsox libraries
    stmdev_ctx_t ag_ctx_{};
    stmdev_ctx_t m_ctx_{};

    mutable std::mutex mtx_;
    EulerDeg euler_{}; 
    Vec3f accel_{}, gyro_{}, mag_{};

    //calibration stuff from imu.py
    float B_[3] = {-18.49f, 46.32f, 29.76f};
    float Ainv_[3][3] = {
        { 32.71410,  0.43184,  0.13793},
             {  0.43184, 38.04931,  0.83862},
             {  0.13793,  0.83862, 29.27600}
    };

    ImuConfig cfg_;
    std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();
};