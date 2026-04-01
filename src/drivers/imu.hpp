#pragma once

#include "lsm6dsox_reg.h"
#include "lis3mdl_reg.h"
#include <cstdint>
#include<stdexcept>
#include "platform_i2c.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

//lsm6dsox is aceleromter and gyroscope
//lis3mdl is magnetometer

class IMU {

    private:
        //hard iron offset vector (from imu.py)
        double B[3] = {-18.49, 46.32, 29.76};

        //soft iron offset matrix (from imu.py)
        double Ainv[3][3] = {
            {32.71410, 0.43184, 0.13793},
            {0.43184, 38.04931, 0.83862},
            {0.13793, 0.83862, 29.27600}
        };

        platformHandleT ag_handle_;
        platformHandleT m_handle_;


    public:
        IMU() {
            #ifndef BUILD_SIMULATION
                //open i2c devices w/ lgpio
                ag_handle_.i2cBus = 1; //assuming we're on I2c bus 1
                ag_handle_.i2cAddress = 0x6A; //assuming lsm6dsox is at address 0x6A
                ag_handle_.i2cHandle = lgI2cOpen(ag_handle_.i2cBus, ag_handle_.i2cAddress, 0);
                if(ag_handle_.i2cHandle < 0) {
                    throw std::runtime_error("failed to open LSM6DSOX I2C");
                }

                m_handle_.i2cBus = 1; //assuming we're on i2c bus 1
                m_handle_.i2cAddress = 0x1E; //assuming lis3mdl is at address 0x1E
                m_handle_.i2cHandle = lgI2cOpen(m_handle_.i2cBus, m_handle_.i2cAddress, 0);
                if(m_handle_.i2cHandle < 0) {
                    throw std::runtime_error("failed to open LIS3MDL I2C");
                }

                //set up st driver contexts from stmdev_ctx_t struct
                stmdev_ctx_t ag_ctx_{};
                ag_ctx_.write_reg = platform_write;
                ag_ctx_.read_reg = platform_read;
                ag_ctx_.mdelay = platform_delay;
                ag_ctx_.handle = &ag_handle_;

                stmdev_ctx_t m_ctx_{};
                m_ctx_.write_reg = platform_write;
                m_ctx_.read_reg = platform_read;
                m_ctx_.mdelay = platform_delay;
                m_ctx_.handle = &m_handle_;

                //check who_am_i for both sensors
                uint8_t who = 0;

                //lsm6dsox_device_id_get returns the id of the sensor; who != LSM6DSOX_ID checks if the id is correct 
                if(lsm6dsox_device_id_get(&ag_ctx_, &who) != 0 || who != LSM6DSOX_ID) {
                    lgI2cCLose(ag_handle_.i2cHandle);
                    throw std::runtime_error("lsm6dsox not found or wrong id");
                }
                //same as above but for lis3mdl
                if(lis3mdl_device_id_get(&m_ctx_, &who) != 0 || who != LIS3MDL_ID) {
                    lgI2cClose(mag_handle_.i2cHandle);
                    throw std::runtime_error("lis3mdl not found or wrong id");
                }

                

            #else
                int ret = 0;
            #endif

            stmdev_ctx_t ctx{};
            platformHandleT platform(.i2cBus = 1; .i2cAddress=0x6A, .i2cHandle = yourLgI2cHandle);

            ctx.write_reg = platform_write;
            ctx.read_reg = platform_read;
            ctx.handle = &platform;

            this->lsm6dsoxAgSensor = new LSM6DS(i2c);
            this->lis3mdlMagSensor = new LIS3MDL(i2c);

            this->lsm6dsoxAgSensorRange = AccelRange.RANGE_8G;
            std::cout << "Accelerometer range is set to: " << AccelRange.string[this->lsm6dsoxAgSensor->accelerometer_range] << std:: endl;

            this->lsm6dsoxAgSensor->gyro_range = GyroRange.RANGE_2000_DPS;
            std::cout << "Gyro range is set to " << GyroRange.string[this->lsm6dsoxAgSensor->gyro_range] << std::endl;

            this->lsm6dsoxAgSensor->accelerometer_data_rate = Rate.RATE_1_66K_HZ;
            std::cout << "Accelerometer data rate is " << Rate.string[this->lsm6dsoxAgSensor->accelerometer_data_rate] << std::endl;

            this->lsm6dsoxAgSensor->gyro_data_rate = Rate.RATE_1_66K_HZ;
            std::cout << "Gyro data rate is " << Rate.string[this->lsm6dsoxAgSensor->gyro_data_rate] << std::endl;

            this->offset = imufusion.Offset(Rate.RATE_1_66K_HZ);
            const std::chrono::zoned_time prevTime{std::chrono::current_zone(), std::chrono::system_clock::now()};

            this->ahrs = imufusion.Ahrs();


            //hardcoded values from imu.py
            this->ahrs.settings = imufusion.Settings(
                imufusion.CONVENTION_NWU, //convention
                0.5, //gain
                2000, //gyroscope range
                10, //acceleration rejection
                10, //magnetic rejection 
                5 * Rate.RATE_1_66K_HZ //recovery trigger period = 5 seconds
            );

            this-> heading = 0.0;
            this-> pitch = 0.0;
            this-> roll = 0.0;
            this-> magX = this-> magY = this->magZ = 0;
            this-> accelX = this-> accelY = this-> accel z = 0;

            this-> lock = std::make_unique<std::mutex>();
            this-> stopEvent = std::make_unique<std::event>(false);
            this-> thread = nullptr;
        }

        void readEuler(float *roll, float *pitch, float *yaw) {

            return roll, pitch, yaw;
        }

};