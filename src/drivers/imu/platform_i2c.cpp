

//3 functions so that the ST LSM6DSOX and LIS3MDL driver can talk to the i2c
#include <cstdint>
#include <unistd.h>
#include <stdlib.h>
#include "platform_i2c.hpp"

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {

    platformHandleT * ctx = (platformHandleT *) handle;

    if(len > 32 || len < 1) {
        return -1;
    }


    #ifndef BUILD_SIMULATION
        int ret = lgI2cWriteI2cBlockData(ctx->i2cHandle, reg, (const char *)bufp, len);
    #else
        int ret = 0;
    #endif

    return (ret >= 0) ? 0 : -1;
}

int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    platformHandleT *ctx = (platformHandleT *) handle;

    if(len > 32 || len < 1) {
        return -1;
    }

    #ifndef BUILD_SIMULATION
        int n = lgI2cReadI2cBlockData(ctx->i2cHandle, reg, (char *)bufp, len);
    #else
        int n = 0;
    #endif

    if (n < 0) {
        return -1;
    }
    return (n == (int)len) ? 0 : -1;
}

void platform_delay(uint32_t millisec) {
    usleep(millisec * 1000);
}