

//3 functions so that the ST LSM6DSOX and LIS3MDL driver can talk to the i2c
#include <cstdint>
#include <unistd.h>
#include <stdlib.h>

#ifndef BUILD_SIMULATION
    #include <lgpio.h>
#endif

typedef struct {
    int i2cBus;
    uint8_t i2cAddress; //chip address
    int i2cHandle; //open connection to the chip. all actual i2c traffic uses this handle
} platformHandleT;

//each byte sent is written to one register, then the next, etc
//look at the datasheet for registers and the number of bytes they send/store
//if more bytes are written then the documented field they can fall into the next registers
//don't write in read-only data registers


//functions are named with _ so C driver can link to them

//writes bytes to specific register on i2c device (lsm6dsox)
//int32_t is the return type; 0 is success -1 is error; its for the ST driver to check
//void *handle: pointer that is passed back to us. We cast it to the struct that holds everything needed to talk to i2c(bus, address, open handle)
//uint8_t reg: its the register address
//const uint8_t *bufp: pointer to the data we want to write into the register. Memory isn't owned it's read
//uint16_t len: how many bytes we want to write from bufp. we send reg(1 byte) + len bytes total
int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    //handle is the given pointer from the driver
    //we cast handle to platformHandleT * since we stored a pointer to our struct there
    //essentially turn the generic handle back into our struct to use i2cHandle and anything else
    //ctx is short for context. In this it's the i2c context so it's a pointer to the struct that holds everything
    platformHandleT * ctx = (platformHandleT *) handle;

    if(len > 32 || len < 1) {
        return -1;
    }

    //writes up to 32 bytes of data to the register
    //if we're writing over 32 bytes break it up into smaller writes
    #ifndef BUILD_SIMULATION
        int ret = lgI2cWriteI2cBlockData(ctx->i2cHandle, reg, (const char *)bufp, len);
    #else
        int ret = 0;
    #endif

    //lgI2cWrite should return 0 on a success and -1 on failure 
    return (ret >= 0) ? 0 : -1;
}




//set specific register then read number of bytes from it
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    platformHandleT *ctx = (platformHandleT *) handle;

    if(len > 32 || len < 1) {
        return -1;
    }

    //reads up to a max of 32 bytes of data from the register
    //data is put into the bufp pointer
    #ifndef BUILD_SIMULATION
        int n = lgI2cReadI2cBlockData(ctx->i2cHandle, reg, (char *)bufp, len);
    #else
        int n = 0;
    #endif

    //-1 lets ST driver know read failed
    if (n < 0) {
        return -1;
    }
    return (n == (int)len) ? 0 : -1;
}

void platform_delay(uint32_t millisec) {
    usleep(millisec * 1000);
}