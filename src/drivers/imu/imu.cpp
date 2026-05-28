#include "platform_i2c.hpp"
#include "lsm6dsox_reg.h"
#include "lis3mdl_reg.h"

#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "Fusion.h"
#include <stdbool.h>

#include <cmath>
#include <algorithm>

#include "imu.hpp"

#ifndef BUILD_SIMULATION
#include <lgpio.h>
#endif

//lsm6dsox is for  accelerometer and gyroscope
//lis3mdl is for  magnometer
//fusion ahrs is to turn raw data into roll, pitch, and yaw

IMU::IMU(const ImuConfig& cfg) {
    cfg_ = cfg;
    openDevices_();
    configureSensors_();
}

IMU::~IMU() {
    closeDevices_();
}

void IMU::openDevices_() {
    //initialize lsm6dsox stuff
    ag_handle_.i2cBus = cfg_.bus;
    ag_handle_.i2cAddress = cfg_.ag_addr;
    ag_handle_.i2cHandle = lgI2cOpen(cfg_.bus, cfg_.ag_addr, 0);
    if(ag_handle_.i2cHandle < 0) {
        throw std::runtime_error("i2c for ag didn't open");
    }

    ag_ctx_ = {};
    ag_ctx_.write_reg = platform_write;
    ag_ctx_.read_reg = platform_read;
    ag_ctx_.mdelay = platform_delay;
    ag_ctx_.handle = &ag_handle_;

    //initialize lis3mdl stuff
    m_handle_.i2cBus = cfg_.bus;
    m_handle_.i2cAddress = cfg_.mag_addr;
    m_handle_.i2cHandle = lgI2cOpen(cfg_.bus, cfg_.mag_addr, 0);
    if(m_handle_.i2cHandle < 0) {
        lgI2cClose(ag_handle_.i2cHandle);
        throw std::runtime_error("i2c for m didn't open");
    }

    m_ctx_ = {};
    m_ctx_.write_reg = platform_write;
    m_ctx_.read_reg = platform_read;
    m_ctx_.mdelay = platform_delay;
    m_ctx_.handle = &m_handle_;

    //initialize ahrs
    FusionAhrsInitialise(&ahrs);

    //basic settings from fusion library
    const FusionAhrsSettings settings = {
        .convention = FusionConventionNwu,
        .gain = cfg_.gain,
        .gyroscopeRange = 2000.0f, 
        .accelerationRejection = cfg_.accel_reject,
        .magneticRejection = cfg_.mag_reject,
        .recoveryTriggerPeriod = 
            std::max(1u, static_cast<unsigned int>(std::lround(cfg_.recovery_seconds * cfg_.update_hz))), // recovery seconds * update rate(Hz)
    };

    FusionAhrsSetSettings(&ahrs, &settings);
}


void IMU::configureSensors_() {
        int32_t ret = 1;
        ret = lsm6dsox_reset_set(&ag_ctx_, PROPERTY_ENABLE);
        if(ret != 0) {
            throw std::runtime_error("IMU didn't reset");
        }

        platform_delay(10);
    
        ret = lsm6dsox_block_data_update_set(&ag_ctx_, PROPERTY_ENABLE);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("lsm6dsox won't update x/y/z at once");
        }

        ret = lsm6dsox_xl_full_scale_set(&ag_ctx_, LSM6DSOX_8g);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("sensitivity for accelerometer isn't set to +-8g");
        }

        ret = lsm6dsox_gy_full_scale_set(&ag_ctx_, LSM6DSOX_2000dps);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("gyro range isn't set to +- 2000 degrees/second");
        }

        ret = lsm6dsox_xl_data_rate_set(&ag_ctx_, LSM6DSOX_XL_ODR_104Hz);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("accelerometer output range isn't set to 104Hz");
        }

        ret = lsm6dsox_gy_data_rate_set(&ag_ctx_, LSM6DSOX_GY_ODR_104Hz);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("gyroscope output range isn't set to 104Hz");
        }
    
        ret = lis3mdl_block_data_update_set(&m_ctx_, PROPERTY_ENABLE);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("lis3mdl won't update x/y/z at once");
        }

        ret = lis3mdl_full_scale_set(&m_ctx_, LIS3MDL_4_GAUSS);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("magnometer range isn't set to +-4 gauss");
        }

        ret = lis3mdl_data_rate_set(&m_ctx_, LIS3MDL_UHP_80Hz);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("output data rate isn't set to 80Hz");
        }

        ret = lis3mdl_operating_mode_set(&m_ctx_, LIS3MDL_CONTINUOUS_MODE);
        if(ret != 0) {
            closeDevices_();
            throw std::runtime_error("magnometer isn't running continuously");
        }

        platform_delay(50);
}

void IMU::updateImuReading() {
    //read raw sensor data
    int16_t accel_raw[3] = {0, 0, 0};
    int16_t gyro_raw[3] = {0, 0, 0};
    int16_t mag_raw[3] = {0, 0, 0};
    int32_t ret;

    //get raw imu values
    ret = lsm6dsox_acceleration_raw_get(&ag_ctx_, accel_raw);
    if(ret != 0) {
        throw std::runtime_error("didn't get raw accelerometer values");
    }

    ret = lsm6dsox_angular_rate_raw_get(&ag_ctx_, gyro_raw);
    if(ret != 0) {
        throw std::runtime_error("didn't get raw gyroscope values");
    }

    ret = lis3mdl_magnetic_raw_get(&m_ctx_, mag_raw);
    if(ret != 0) {
        throw std::runtime_error("didn't get raw magnetometer values");
    }

    //compute dt in seconds
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    float dt_seconds = std::chrono::duration<float>(currentTime - previousTime).count();

    if(dt_seconds <= 0.0f) {
        return;
    }

    //convert raw to physical units for Fusion
    //fusion expects gyro in degrees/second, accel in g, mag in calibrated units(like gauss)
    const FusionVector gyro_fusion = { .axis = {
        lsm6dsox_from_fs2000_to_mdps(gyro_raw[0]) / 1000.0f,
        lsm6dsox_from_fs2000_to_mdps(gyro_raw[1]) / 1000.0f, 
        lsm6dsox_from_fs2000_to_mdps(gyro_raw[2]) / 1000.0f
    }};

    const FusionVector accel_fusion = { .axis = {
        lsm6dsox_from_fs8_to_mg(accel_raw[0]) / 1000.0f, 
        lsm6dsox_from_fs8_to_mg(accel_raw[1]) / 1000.0f,
        lsm6dsox_from_fs8_to_mg(accel_raw[2]) / 1000.0f
    }};

    const FusionVector mag_fusion = { .axis = {
        lis3mdl_from_fs4_to_gauss(mag_raw[0]),
        lis3mdl_from_fs4_to_gauss(mag_raw[1]),
        lis3mdl_from_fs4_to_gauss(mag_raw[2])
    }};

    //update ahrs and compute euler
    FusionAhrsUpdate(&ahrs, gyro_fusion, accel_fusion, mag_fusion, dt_seconds);
    const FusionEuler euler_local = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    previousTime = currentTime;

    //lock
    {
        std::lock_guard<std::mutex> guard(mtx_);
        accel_ = {accel_raw[0], accel_raw[1], accel_raw[2]};
        gyro_ = {gyro_raw[0], gyro_raw[1], gyro_raw[2]};
        mag_ = {mag_raw[0], mag_raw[1], mag_raw[2]};

        euler_ = euler_local;
    }
}

FusionEuler IMU::readEuler() const {
    std::lock_guard<std::mutex> guard(mtx_);
    return euler_;
}

Vec3f IMU::readAccel() const {
    std::lock_guard<std::mutex> guard(mtx_);
    return accel_;
}

Vec3f IMU::readGyro() const {
    std::lock_guard<std::mutex> guard(mtx_);
    return gyro_;
}

Vec3f IMU::readMag() const {
    std::lock_guard<std::mutex> guard(mtx_);
    return mag_;
}

void IMU::closeDevices_() {

    if(ag_handle_.i2cHandle >= 0) {
        if(lgI2cClose(ag_handle_.i2cHandle) < 0) {
            std::cerr << "Error code freeing ag_handle " << std::endl;
        }
        ag_handle_.i2cHandle = -1;
    }

    if(m_handle_.i2cHandle >= 0) {
        if(lgI2cClose(m_handle_.i2cHandle) < 0) {
            std::cerr << "Error code freeing m_handle " << std::endl;
        }
        m_handle_.i2cHandle = -1;
    }
}

