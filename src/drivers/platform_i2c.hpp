#pragma once

#include <cstdint>

//platform handle structure for i2c communication
typedef struct {
    int i2cBus;
    uint8_t i2cAddress;
    int i2cHandle;
} platformHandleT;

//platform i2c functions for st driver compatability
//these match the function pointer types expected by stmdev_ctx_t

int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);

int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

void platform_delay(uint32_t millisec);