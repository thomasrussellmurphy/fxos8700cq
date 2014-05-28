#include "FXOS8700CQ.h"

uint8_t status_reg; // Status register contents
uint8_t raw[FXOS8700CQ_READ_LEN]; // Buffer for reading out stored data

// Construct class and its contents
FXOS8700CQ::FXOS8700CQ(PinName sda, PinName scl, int addr) : dev_i2c(sda, scl), dev_addr(addr)
{
    // Initialization of the FXOS8700CQ
    dev_i2c.frequency(I2C_400K); // Use maximum I2C frequency
    uint8_t data[6] = {0, 0, 0, 0, 0, 0}; // target device write address, single byte to write at address

    // TODO: verify WHOAMI?
    
    // TODO: un-magic-number register configuration

    // Place peripheral in standby for configuration, resetting CTRL_REG1
    data[0] = FXOS8700CQ_CTRL_REG1;
    // Keep data[1] as 0x00
    write_regs(data, 2);

    // Now that the device is in standby, configure registers

    // Setup for write-though for CTRL_REG series
    // Keep data[0] as FXOS8700CQ_CTRL_REG1
    data[1] = 0x08; // 400 Hz single rate, 200 Hz hybrid mode

    // FXOS8700CQ_CTRL_REG2;
    data[2] = 0x18; // set low power sleep sampling, no auto sleep, normal sampling

    // No configuration changes from default 0x00 in CTRL_REG3
    // Interrupts will be active low
    data[3] = 0x00;

    // FXOS8700CQ_CTRL_REG4;
    data[4] = 0x01; // enable data ready interrupt

    // No configuration changes from default 0x00 in CTRL_REG5
    // Data ready interrupt will appear on INT2
    data[5] = 0x00;

    // Write to the 5 CTRL_REG registers
    write_regs(data, 6);

    // No configuration changes from default 0x00 in XYZ_DATA_CFG
    // No high pass filter and +/- 2g range for accelerometer
    // Do not write any changes

    // Setup for write-through for M_CTRL_REG series
    data[0] = FXOS8700CQ_M_CTRL_REG1;
    data[1] = 0x9F; // automatic calibration, maximum oversampling, hybrid sampling mode

    // FXOS8700CQ_M_CTRL_REG2
    data[2] = 0x20; // allow automatic read-through from accel to magn data at acc_z (0x05/0x06)

    // FXOS8700CQ_M_CTRL_REG3
    data[3] = 0x70; // use calibration data, sleep oversampling

    // Write to the 3 M_CTRL_REG registers
    write_regs(data, 4);

    // Peripheral is configured, but disabled
}

// Destruct class
FXOS8700CQ::~FXOS8700CQ(void) {}


void FXOS8700CQ::enable(void)
{
    uint8_t data[2];
    read_regs( FXOS8700CQ_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01; // set bit 0, CTRL_REG1:active
    data[0] = FXOS8700CQ_CTRL_REG1;
    write_regs(data, 2); // write back
}

void FXOS8700CQ::disable(void)
{
    uint8_t data[2];
    read_regs( FXOS8700CQ_CTRL_REG1, &data[1], 1);
    data[1] &= 0xFE; // unset bit 0, CTRL_REG1:active
    data[0] = FXOS8700CQ_CTRL_REG1;
    write_regs(data, 2); // write back
}


uint8_t FXOS8700CQ::status(void)
{
    read_regs(FXOS8700CQ_STATUS, &status_reg, 1);
    return status_reg;
}

uint8_t FXOS8700CQ::get_whoami(void)
{
    uint8_t databyte = 0x00;
    read_regs(FXOS8700CQ_WHOAMI, &databyte, 1);
    return databyte;
}

void FXOS8700CQ::get_data(SRAWDATA *accel_data, SRAWDATA *magn_data)
{
    read_regs(FXOS8700CQ_OUT_X_MSB, raw, FXOS8700CQ_READ_LEN); // WRONG, getting accel then magn

    // Pull out 16-bit, 2's complement magnetometer data
    magn_data->x = (raw[0] << 8) | raw[1];
    magn_data->y = (raw[2] << 8) | raw[3];
    magn_data->z = (raw[4] << 8) | raw[5];

    // Below is wrong, using left-justified version
    // Pull out 14-bit, 2's complement, right-justified accelerometer data
    accel_data->x = (raw[6] << 8) | raw[7];
    accel_data->y = (raw[8] << 8) | raw[9];
    accel_data->z = (raw[10] << 8) | raw[11];
}


// Private methods

void FXOS8700CQ::read_regs(int reg_addr, uint8_t* data, int len)
{
    char t[1] = {reg_addr};
    dev_i2c.write(dev_addr, t, 1, true);
    dev_i2c.read(dev_addr, (char *)data, len);
}

void FXOS8700CQ::write_regs(uint8_t* data, int len)
{
    dev_i2c.write(dev_addr, (char*)data, len);
}
