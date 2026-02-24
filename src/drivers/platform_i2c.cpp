/*

//3 functions so that the ST LSM6DSOX and LIS3MDL driver can talk to the i2c

#include <lgpio.h>
#include <unistd.h>
#include <stdlib.h>


typedef struct {
    int i2cBus;
    uint8_t i2cAddress; //chip address
    int i2cHandle; //open connection to the chip. all actual i2c traffic uses this handle
} platformHandleT;


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
    //allocates a buffer for "reg + data"
    //we need 1 + len bytes (1 for reg, len for data)
    //malloc(len + 1) allocates that many bytes onto the heap
    //char * casts it so we can use it as a byte buffer
    //a byte in memory is the same as a char in c so a buffer of chars = buffer of bytes
    //out holds the full i2c write payload
    char *out = (char *)malloc(len + 1);
    //if malloc fails it returns null
    //!out means null ig
    //if its null we return -1 so driver knows the write failed and pigpio isn't called
    if(!out) {return -1;}
    //first byte sent over i2c must be the register address
    out[0] = (char)reg;
    //copies the data given by the driver bufp[0 to len-1] into out[1 to len]
    //so out[0] is reg and out[everything else] is the data
    for(uint16_t i = 0; i < len; i++) {
        out[i+1] = bufp[i];
    }

    int handle =lgI2cOpen(i2cBus, i2cAddress, 0);
    int ret = lgI2cWrite(handle, reg, out);

    //after getting the memory with malloc we give it back with free to not leak memory
    //we're done with out after writing
    free(out);

    lgI2cClose(handle);

    //lgI2cWrite should return 0 on a success and -1 on failure 
    return (ret >= 0) ? 0 : -1;
}




//set specific register then read number of bytes from that register on the ST drivers
int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    platformHandleT *ctx = (platformHandleT *) handle;
    int wret = i2cWriteDevice(ctx->i2cHandle, (char *)&reg, 1);
    if(wret != 0) {
        return -1;
    }

    int handle = lgI2cOpen(i2cBus, i2cAddress, 0);
    int ret = lgI2cRead(handle, reg, bufp);

    lgI2cClose(handle);
    return (ret == (int)len) ? 0 : -1;
}

void platform_delay(uint32_t millisec) {
    usleep(millisec * 1000);
}

*/