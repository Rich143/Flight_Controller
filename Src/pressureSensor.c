#include "freertos.h"
#include "task.h"

#include "fc.h"
#include "pressureSensor.h"
#include "hardware.h"
#include "debug.h"

#define PRESSURE_SENSOR_I2C_TIMEOUT 1000

#define PRESSURE_SENSOR_ADDRESS 0x5D
#define PRESSURE_SENSOR_ADDRESS_HAL (PRESSURE_SENSOR_ADDRESS<<1) // Left shift it 1 bit, the way the HAL functions want it

#define ODR_HZ 25
#define ODR_PERIOD_MS (1000 / ODR_HZ)

//
// Registers and Values
//
#define PRESSURE_REG_WHOAMI    0x0F
#define PRESSURE_SENSOR_WHOAMI_RESPONSE 0xBD

#define PRESSURE_CTRL_REG1     0x20
#define PD 7
#define ODR2 6
#define ODR1 5
#define ODR0 4
#define DIFF_EN 3
#define BDU 2
#define RESET_AZ 1
#define SIM 0

#define PRESSURE_CTRL_REG2              0x21
#define SWRESET 2

#define PRESSURE_TEMP_OUT_L 0x2B
#define PRESSURE_TEMP_OUT_H 0x2C

FC_Status PressureSensor_RegRead(uint8_t regAddress, uint8_t *val, int size)
{
    HAL_StatusTypeDef rc;
    rc = HAL_I2C_Mem_Read(&I2cHandle, PRESSURE_SENSOR_ADDRESS_HAL,
                          regAddress,
                          I2C_MEMADD_SIZE_8BIT, val, size,
                          PRESSURE_SENSOR_I2C_TIMEOUT);

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("Pressure Reg read failed: %d\n", rc);
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status PressureSensor_RegWrite(uint8_t regAddress, uint8_t val)
{
    HAL_StatusTypeDef rc;
    rc = HAL_I2C_Mem_Write(&I2cHandle, PRESSURE_SENSOR_ADDRESS_HAL,
                          regAddress,
                          I2C_MEMADD_SIZE_8BIT, &val, 1 /* 1 byte read */,
                          PRESSURE_SENSOR_I2C_TIMEOUT);

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("Pressure Reg write failed: %d\n", rc);
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status pressureSensorInit(void)
{
    uint8_t whoami;

    if (PressureSensor_RegRead(PRESSURE_REG_WHOAMI, &whoami, 1 /* 1 byte read */)
        != FC_OK)
    {
        return FC_ERROR;
    }

    if (whoami != PRESSURE_SENSOR_WHOAMI_RESPONSE) {
        DEBUG_PRINT("Whoami error. Got: %X, expected: %X\n", whoami,
                    PRESSURE_SENSOR_WHOAMI_RESPONSE);
        return FC_ERROR;
    }

    if (PressureSensor_RegWrite(PRESSURE_CTRL_REG2, _BIT(SWRESET)) != FC_OK)
    {
        return FC_ERROR;
    }

    // Wait for 1 msec for chip to reset
    HAL_Delay(1);

    // Power on sensor and set output data rate to 25 Hz
    // Enabling BDU lock high and low registers until both read
    uint8_t tmp = _BIT(PD) | _BIT(ODR2) | _BIT(BDU);
    if (PressureSensor_RegWrite(PRESSURE_CTRL_REG1, tmp) != FC_OK)
    {
        return FC_ERROR;
    }

    return FC_OK;
}

/**
 * @brief Get the temperature in ˚C.
 *
 * @param Tout The temperature in 10 x ˚C
 *
 * @return [FC_OK, FC_ERROR]
 */
FC_Status pressureSensor_GetTemp(int16_t *Tout)
{
    int16_t raw_data;
    uint8_t buffer[2];

    if (PressureSensor_RegRead(PRESSURE_TEMP_OUT_L, &(buffer[0]), 1) != FC_OK)
    {
        DEBUG_PRINT("Error reading temp\n");
        return FC_ERROR;
    }
    if (PressureSensor_RegRead(PRESSURE_TEMP_OUT_H, &(buffer[1]), 1) != FC_OK)
    {
        DEBUG_PRINT("Error reading temp\n");
        return FC_ERROR;
    }

    raw_data = (((uint16_t)buffer[1]) << 8) + (uint16_t)buffer[0];


    // Tout(degC) = 42.5 + (raw_data/480)
    // Not sure where 42.5 comes from, range of -30 to 105 divided by 2 gives
    // 67.5 ...
    *Tout = raw_data/48 + 425;

    return FC_OK;
}

void vPressureSensorTask(void *pvParameters)
{
    DEBUG_PRINT("Starting pressure sensor task\n");
    if (pressureSensorInit() != FC_OK)
    {
        Error_Handler();
    }
    DEBUG_PRINT("Initialized pressure sensor\n");

    int16_t Temp;
    for ( ;; )
    {
        if (pressureSensor_GetTemp(&Temp) != FC_OK) {
            // Maybe do something???
        }

        DEBUG_PRINT("Temp: %d\n", Temp);

        /*vTaskDelay(ODR_PERIOD_MS / portTICK_PERIOD_MS);*/
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}