#ifndef FXOS8700CQ_H
#define FXOS8700CQ_H

#include "mbed.h" // Building this for the mbed platform

#define I2C_400K 400000

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS 0x00
#define FXOS8700CQ_OUT_X_MSB 0x01
#define FXOS8700CQ_WHOAMI 0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1 0x2A
#define FXOS8700CQ_CTRL_REG2 0x2B
#define FXOS8700CQ_CTRL_REG3 0x2C
#define FXOS8700CQ_CTRL_REG4 0x2D
#define FXOS8700CQ_CTRL_REG5 0x2E

#define FXOS8700CQ_M_OUT_X_MSB 0x34

#define FXOS8700CQ_M_CTRL_REG1 0x5B
#define FXOS8700CQ_M_CTRL_REG2 0x5C
#define FXOS8700CQ_M_CTRL_REG3 0x5D

// FXOS8700CQ WHOAMI production register value
#define FXOS8700CQ_WHOAMI_VAL 0xC7

// 6 channels of two bytes = 12 bytes; read from FXOS8700CQ_OUT_X_MSB
#define FXOS8700CQ_READ_LEN 12

// From mbed I2C documentation for complete read/write transactions
#define I2C_SUCCESS 0
#define I2C_FAILURE 1

// For processing the accelerometer data to right-justified 2's complement
#define UINT14_MAX 16383

// TODO: struct to hold the data out of the sensor
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} SRAWDATA;

class FXOS8700CQ
{
public:
    /**
    * FXOS8700CQ constructor
    *
    * @param sda SDA pin
    * @param sdl SCL pin
    * @param addr address of the I2C peripheral in (7-bit << 1) form
    */

    FXOS8700CQ(PinName sda, PinName scl, int addr);

    /**
    * FXOS8700CQ destructor
    */
    ~FXOS8700CQ();

    void enable(void);
    void disable(void);
    uint8_t get_whoami(void);
    uint8_t status(void);
    void get_data(SRAWDATA *accel_data, SRAWDATA *magn_data);



private:
    I2C dev_i2c; // instance of the mbed I2C class
    uint8_t dev_addr; // Device I2C address, in (7-bit << 1) form

    // I2C helper methods
    void read_regs(int reg_addr, uint8_t* data, int len);
    void write_regs(uint8_t* data, int len);

};

#endif
