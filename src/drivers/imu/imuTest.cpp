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

#ifndef BUILD_SIMULATION
#include <lgpio.h>
#endif

volatile bool run = true;
void on_sigint(int) { run = false; }

int main() {
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

        lsm6dsox_reset_set(&ag_ctx, PROPERTY_ENABLE);
        platform_delay(10);

        lsm6dsox_block_data_update_set(&ag_ctx, PROPERTY_ENABLE);
        lsm6dsox_xl_full_scale_set(&ag_ctx, LSM6DSOX_8g);
        lsm6dsox_gy_full_scale_set(&ag_ctx, LSM6DSOX_2000dps);
        lsm6dsox_xl_data_rate_set(&ag_ctx, LSM6DSOX_XL_ODR_104Hz);
        lsm6dsox_gy_data_rate_set(&ag_ctx, LSM6DSOX_GY_ODR_104Hz);

        lis3mdl_block_data_update_set(&m_ctx, PROPERTY_ENABLE);
        lis3mdl_full_scale_set(&m_ctx, LIS3MDL_4_GAUSS);
        lis3mdl_data_rate_set(&m_ctx, LIS3MDL_UHP_80Hz);
        lis3mdl_operating_mode_set(&m_ctx, LIS3MDL_CONTINUOUS_MODE);
        platform_delay(50);

        while(run) {
            int16_t accel[3]{};
            int16_t gyro[3]{};
            int16_t mag[3]{};

            lsm6dsox_acceleration_raw_get(&ag_ctx, accel);
            lsm6dsox_angular_rate_raw_get(&ag_ctx, gyro);
            lis3mdl_magnetic_raw_get(&m_ctx, mag);

            std::cout << accel[0] << ' ' << accel[1] << ' ' << accel[2] << ' \n';
            
            std::cout << gyro[0] << ' ' << gyro[1] << ' ' << gyro[2] << ' \n';
            std::cout << mag[0] << ' ' << mag[1] << ' ' << mag[2] << ' \n';

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
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