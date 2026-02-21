//3 functions so that the ST LSM6DSOX and LIS3MDL driver can talk to the i2c

#include <pigpio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    int i2cBus;
    uint8_t i2cAddress;
    int i2cHandle;
} platformHandleT;

//writes bytes to specific register on ST drivers
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    platformHandleT * ctx = (platformHandleT *) handle;
    char *out = (char *)malloc(len + 1);
    if(!out) {return -1;}
    out[0] = (char)reg;
    for(uint16_t i = 0; i < len; i++) {
        out[i+1] = bufp[i];
    }
    int ret = i2cWriteDevice(ctx->i2cHandle, out, len + 1);
    free(out);
    return (ret >= 0) ? 0 : -1;
}

//set specific register then read number of bytes from that register on the ST drivers
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    platformHandleT *ctx = (platformHandleT *) handle;
    int wret = i2cWriteDevice(ctx->i2cHandle, (char *)&reg, 1);
    if(wret != 0) {
        return -1;
    }
    int ret = i2cReadDevice(ctx->i2cHandle, (char *)bufp,len);
    return (ret == (int)len) ? 0 : -1;
}

void platform_delay(uint32_t millisec) {
    usleep(millisec * 1000);
}

