/*
reconfigure cmake
cmake -B build -DCMAKE_BUILD_TYPE=Release


cmake --build build --target imu_test
sudo ./build/imu_test
*/

/*
pins
3v3 to 1
SDA to 3
SCL to 5
GND to 6 (any ground)
*/

//seems to understand no movement and movement
//reacts fairly quickly


/*
we can probably get gyro bias
we can get a still one and one for in the pool

hard and soft iron offsets will be difficult to get for ASV
not sure how different jesus will be from the ASV in terms of positioning and metal
*/

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

#include <fstream>



#ifndef BUILD_SIMULATION
#include <lgpio.h>
#endif

#define LOOP_TIME (0.1f) //100 milliseconds

volatile bool run = true;
void on_sigint(int) { run = false; }

int main() {

    std::ofstream outFile("poolImuTest.txt");
    outFile << "write to file";
    outFile.close();

    FusionAhrs ahrs;
    FusionAhrsInitialise(&ahrs);

    //basic settings from library
    const FusionAhrsSettings settings = {
        .convention = FusionConventionNwu,
        .gain = 0.5f,
        .gyroscopeRange = 2000.0f, /* replace with actual gyroscope range */
        .accelerationRejection = 10.0f,
        .magneticRejection = 10.0f,
        .recoveryTriggerPeriod = 50, /* 50 for 100ms loop or 520 for 104Hz sensor rate */
    };

    FusionAhrsSetSettings(&ahrs, &settings);

    std::signal(SIGINT, on_sigint);

    platformHandleT ag_handle{};
    platformHandleT m_handle_{};

    ag_handle.i2cHandle = -1;
    m_handle_.i2cHandle = -1;

    stmdev_ctx_t ag_ctx{};
    stmdev_ctx_t m_ctx{};

    try{
        ag_handle.i2cBus = 1;
        ag_handle.i2cAddress = 0x6A;
        ag_handle.i2cHandle = lgI2cOpen(1, 0x6A, 0);

        if(ag_handle.i2cHandle < 0) {
            throw std::runtime_error("i2c for ag didn't open");
        }

        m_handle_.i2cBus = 1;
        m_handle_.i2cAddress = 0x1C;
        m_handle_.i2cHandle = lgI2cOpen(1, 0x1C, 0);

        if(m_handle_.i2cHandle < 0) {
            throw std::runtime_error("i2c for m didn't open");
        }

        ag_ctx = {};
        ag_ctx.write_reg = platform_write;
        ag_ctx.read_reg = platform_read;
        ag_ctx.mdelay = platform_delay;
        ag_ctx.handle = &ag_handle;

        m_ctx = {};
        m_ctx.write_reg = platform_write;
        m_ctx.read_reg = platform_read;
        m_ctx.mdelay = platform_delay;
        m_ctx.handle = &m_handle_;

        //resets IMU
        lsm6dsox_reset_set(&ag_ctx, PROPERTY_ENABLE);
        //waits 10ms so reset can finish before more register writes
        platform_delay(10);

        // x/y/z registers update together
        lsm6dsox_block_data_update_set(&ag_ctx, PROPERTY_ENABLE);
        //sets sensitivity for accelerometer to +-8g; should match converter that is used later
        //might want to make it less sensitive
        lsm6dsox_xl_full_scale_set(&ag_ctx, LSM6DSOX_8g);
        //gyro range is +- 2000 degress per second
        lsm6dsox_gy_full_scale_set(&ag_ctx, LSM6DSOX_2000dps);
        //acceleration output rate is 104Hz so new samples every ~9.6ms
        lsm6dsox_xl_data_rate_set(&ag_ctx, LSM6DSOX_XL_ODR_104Hz);
        //gyro output rate is also 104Hz
        lsm6dsox_gy_data_rate_set(&ag_ctx, LSM6DSOX_GY_ODR_104Hz);

        lis3mdl_block_data_update_set(&m_ctx, PROPERTY_ENABLE);
        lis3mdl_full_scale_set(&m_ctx, LIS3MDL_4_GAUSS);
        ////80Hz is ultra high performance (not sure if that level is really needed but good yaw would be useful i think)
        lis3mdl_data_rate_set(&m_ctx, LIS3MDL_UHP_80Hz);
        //run magnometer continuously
        lis3mdl_operating_mode_set(&m_ctx, LIS3MDL_CONTINUOUS_MODE);
        platform_delay(50);

        int count = 0;

        int16_t accel0 = 0;
        int16_t accel1 = 0;
        int16_t accel2 = 0;

        int16_t gyro0 = 0;
        int16_t gyro1 = 0;
        int16_t gyro2 = 0;

        int16_t mag0 = 0;
        int16_t mag1 = 0;
        int16_t mag2 = 0;

        while(run) {
            //0 is for x, 1 is for y, and 2 is for z for following arrays
            int16_t accel[3]{};
            int16_t gyro[3]{};
            int16_t mag[3]{};

            lsm6dsox_acceleration_raw_get(&ag_ctx, accel);
            lsm6dsox_angular_rate_raw_get(&ag_ctx, gyro);
            lis3mdl_magnetic_raw_get(&m_ctx, mag);

            //lsb to millidegrees/second to degrees/second
            const FusionVector gyroscope = {lsm6dsox_from_fs2000_to_mdps(gyro[0]) / 1000.0f,
                lsm6dsox_from_fs2000_to_mdps(gyro[1]) / 1000.0f,
                lsm6dsox_from_fs2000_to_mdps(gyro[2]) / 1000.0f};

            //lsb to milligrams to grams
            const FusionVector accelerometer = {lsm6dsox_from_fs8_to_mg(accel[0]) / 1000.0f, 
                lsm6dsox_from_fs8_to_mg(accel[1]) / 1000.0f,
                lsm6dsox_from_fs8_to_mg(accel[2]) / 1000.0f};

            //lsb to gauss
            const FusionVector magnetometer = {lis3mdl_from_fs4_to_gauss(mag[0]),
                lis3mdl_from_fs4_to_gauss(mag[1]),
                lis3mdl_from_fs4_to_gauss(mag[2])};

            //for fusionvector stuff we'll likely need to account for bias and stuff later
            //not sure how exactly they calculated all of that for the AUV

            FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, LOOP_TIME);

            std::cout << 'a' << accel[0] << ' ' << accel[1] << ' ' << accel[2] << '\n';
            std::cout << 'g' << gyro[0] << ' ' << gyro[1] << ' ' << gyro[2] << '\n';
            std::cout << 'm' << mag[0] << ' ' << mag[1] << ' ' << mag[2] << '\n';

            const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));

            std::cout << "roll: " << euler.angle.roll << " pitch: " << euler.angle.pitch << " yaw: " << euler.angle.yaw << "\n";

            accel0 += accel[0];
            accel1 += accel[1];
            accel2 += accel[2];

            gyro0 += gyro[0];
            gyro1 += gyro[1];
            gyro2 += gyro[2];

            mag0 += mag[0];
            mag1 += mag[1];
            mag2 += mag[2];

            count++;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        accel0 /= count;
        accel1 /= count;
        accel2 /= count;

        gyro0 /= count;
        gyro1 /= count;
        gyro2 /= count;

        mag0 /= count;
        mag1 /= count;
        mag2 /= count;

        std::cout << "a0 " << accel0 << '\n';
        std::cout << "a1 " << accel1 << '\n';
        std::cout << "a2 " << accel2 << '\n';

        std::cout << "g0 " << gyro0 << '\n';
        std::cout << "g1 " << gyro1 << '\n';
        std::cout << "g2 " << gyro2 << '\n';
        
        std::cout << "m0 " << mag0 << '\n';
        std::cout << "m1 " << mag1 << '\n';
        std::cout << "m2 " << mag2 << '\n';

    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        lgI2cClose(ag_handle.i2cHandle);
        lgI2cClose(m_handle_.i2cHandle);
        return 1;
    }

    lgI2cClose(ag_handle.i2cHandle);
    lgI2cClose(m_handle_.i2cHandle);
    return 0;
}