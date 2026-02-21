#pragma once

/*

#include <thread>
#include <chrono>
#include <pigpio.h>

#include "lsm6dsox_reg.h"
#include "lis3mdl_reg.h"

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

    public:
        IMU() {
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

*/