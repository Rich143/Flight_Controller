#ifndef __PRESSURE_SENSOR_REGISTERS_H
#define __PRESSURE_SENSOR_REGISTERS_H

#define PRESSURE_SENSOR_I2C_TIMEOUT 1000

#define PRESSURE_SENSOR_ADDRESS 0x5D
#define PRESSURE_SENSOR_ADDRESS_HAL (PRESSURE_SENSOR_ADDRESS<<1) // Left shift it 1 bit, the way the HAL functions want it

#define ODR_HZ 25
#define ODR_PERIOD_MS (1000 / ODR_HZ)

//
// Registers and Values
//
#define PRESSURE_REG_WHOAMI             0x0F
#define PRESSURE_SENSOR_WHOAMI_RESPONSE 0xBD

#define PRESSURE_CTRL_REG1              0x20
#define PD                          7
#define ODR2                        6
#define ODR1                        5
#define ODR0                        4
#define DIFF_EN                     3
#define BDU                         2
#define RESET_AZ                    1
#define SIM                         0

#define PRESSURE_CTRL_REG2              0x21
#define SWRESET                     2

#define PRESSURE_TEMP_OUT_L             0x2B
#define PRESSURE_TEMP_OUT_H             0x2C

#define PRESSURE_PRESS_OUT_XL           0x28
#define PRESSURE_PRESS_OUT_L            0x29
#define PRESSURE_PRESS_OUT_H            0x2A

#endif /* defined(__PRESSURE_SENSOR_REGISTERS_H)*/
