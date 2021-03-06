#include "FXOS8700CQ.h"

uint8_t status_reg; // Status register contents
uint8_t raw[FXOS8700CQ_READ_LEN]; // Buffer for reading out stored data

// Construct class and its contents
FXOS8700CQ::FXOS8700CQ(PinName sda, PinName scl, int addr) : dev_i2c(sda, scl), dev_addr(addr)
{
    // Initialization of the FXOS8700CQ
    dev_i2c.frequency(I2C_400K); // Use maximum I2C frequency
    uint8_t data[6] = {0, 0, 0, 0, 0, 0}; // to write over I2C: device register, up to 5 bytes data

    // TODO: verify WHOAMI?

    // Place peripheral in standby for configuration, resetting CTRL_REG1
    data[0] = FXOS8700CQ_CTRL_REG1;
    data[1] = 0x00; // this will unset CTRL_REG1:active
    write_regs(data, 2);

    // Now that the device is in standby, configure registers at will

    // Setup for write-though for CTRL_REG series
    // Keep data[0] as FXOS8700CQ_CTRL_REG1
    data[1] =
        FXOS8700CQ_CTRL_REG1_ASLP_RATE2(1) | // 0b01 gives sleep rate of 12.5Hz
        FXOS8700CQ_CTRL_REG1_DR3(1); // 0x001 gives ODR of 400Hz/200Hz hybrid

    // FXOS8700CQ_CTRL_REG2;
    data[2] =
        FXOS8700CQ_CTRL_REG2_SMODS2(3) | // 0b11 gives low power sleep oversampling mode
        FXOS8700CQ_CTRL_REG2_MODS2(1); // 0b01 gives low noise, low power oversampling mode

    // No configuration changes from default 0x00 in CTRL_REG3
    // Interrupts will be active low, their outputs in push-pull mode
    data[3] = 0x00;

    // FXOS8700CQ_CTRL_REG4;
    data[4] =
        FXOS8700CQ_CTRL_REG4_INT_EN_DRDY; // Enable the Data-Ready interrupt

    // No configuration changes from default 0x00 in CTRL_REG5
    // Data-Ready interrupt will appear on INT2
    data[5] = 0x00;

    // Write to the 5 CTRL_REG registers
    write_regs(data, 6);

    // FXOS8700CQ_XYZ_DATA_CFG
    data[0] = FXOS8700CQ_XYZ_DATA_CFG;
    data[1] =
        FXOS8700CQ_XYZ_DATA_CFG_FS2(1); // 0x01 gives 4g full range, 0.488mg/LSB
    write_regs(data, 2);

    // Setup for write-through for M_CTRL_REG series
    data[0] = FXOS8700CQ_M_CTRL_REG1;
    data[1] =
        FXOS8700CQ_M_CTRL_REG1_M_ACAL | // set automatic calibration
        FXOS8700CQ_M_CTRL_REG1_MO_OS3(7) | // use maximum magnetic oversampling
        FXOS8700CQ_M_CTRL_REG1_M_HMS2(3); // enable hybrid sampling (both sensors)

    // FXOS8700CQ_M_CTRL_REG2
    data[2] =
        FXOS8700CQ_M_CTRL_REG2_HYB_AUTOINC_MODE;

    // FXOS8700CQ_M_CTRL_REG3
    data[3] =
        FXOS8700CQ_M_CTRL_REG3_M_ASLP_OS3(7); // maximum sleep magnetic oversampling

    // Write to the 3 M_CTRL_REG registers
    write_regs(data, 4);

    // Peripheral is configured, but disabled
    enabled = false;
}

// Destruct class
FXOS8700CQ::~FXOS8700CQ(void) {}


void FXOS8700CQ::enable(void)
{
    uint8_t data[2];
    read_regs( FXOS8700CQ_CTRL_REG1, &data[1], 1);
    data[1] |= FXOS8700CQ_CTRL_REG1_ACTIVE;
    data[0] = FXOS8700CQ_CTRL_REG1;
    write_regs(data, 2); // write back

    enabled = true;
}

void FXOS8700CQ::disable(void)
{
    uint8_t data[2];
    read_regs( FXOS8700CQ_CTRL_REG1, &data[1], 1);
    data[0] = FXOS8700CQ_CTRL_REG1;
    data[1] &= ~FXOS8700CQ_CTRL_REG1_ACTIVE;
    write_regs(data, 2); // write back

    enabled = false;
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

uint8_t FXOS8700CQ::get_data(SRAWDATA *accel_data, SRAWDATA *magn_data)
{
    if(!enabled) {
        return 1;
    }

    read_regs(FXOS8700CQ_M_OUT_X_MSB, raw, FXOS8700CQ_READ_LEN);

    // Pull out 16-bit, 2's complement magnetometer data
    magn_data->x = (raw[0] << 8) | raw[1];
    magn_data->y = (raw[2] << 8) | raw[3];
    magn_data->z = (raw[4] << 8) | raw[5];

    // Pull out 14-bit, 2's complement, right-justified accelerometer data
    accel_data->x = (raw[6] << 8) | raw[7];
    accel_data->y = (raw[8] << 8) | raw[9];
    accel_data->z = (raw[10] << 8) | raw[11];

    // Have to apply corrections to make the int16_t correct
    if(accel_data->x > UINT14_MAX/2) {
        accel_data->x -= UINT14_MAX;
    }
    if(accel_data->y > UINT14_MAX/2) {
        accel_data->y -= UINT14_MAX;
    }
    if(accel_data->z > UINT14_MAX/2) {
        accel_data->z -= UINT14_MAX;
    }

    return 0;
}

uint8_t FXOS8700CQ::get_accel_scale(void)
{
    uint8_t data = 0x00;
    read_regs(FXOS8700CQ_XYZ_DATA_CFG, &data, 1);
    data &= FXOS8700CQ_XYZ_DATA_CFG_FS2(3); // mask with 0b11

    // Choose output value based on masked data
    switch(data) {
        case FXOS8700CQ_XYZ_DATA_CFG_FS2(0):
            return 2;
        case FXOS8700CQ_XYZ_DATA_CFG_FS2(1):
            return 4;
        case FXOS8700CQ_XYZ_DATA_CFG_FS2(2):
            return 8;
        default:
            return 0;
    }
}

// Private methods

// Excepting the call to dev_i2c.frequency() in the constructor,
// the use of the mbed I2C class is restricted to these methods
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
