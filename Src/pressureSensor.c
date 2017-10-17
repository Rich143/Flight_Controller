#include <math.h>
#include "fc.h"
#include "pressureSensorRegisters.h"

#ifndef __UNIT_TEST

#include "freertos.h"
#include "task.h"

#include "pressureSensor.h"
#include "hardware.h"
#include "debug.h"

FC_Status PressureSensor_RegRead(uint8_t regAddress, uint8_t *val, int size)
{
    HAL_StatusTypeDef rc;

    if (size > 1)
    {
        // Enable auto increment
        regAddress |= _BIT(7);
    }

    if (xSemaphoreTake(I2CMutex, I2C_MUT_WAIT_TICKS) != pdTRUE)
    {
        DEBUG_PRINT("Pressure failed to take I2C mut\n");
        return FC_ERROR;
    }

    rc = HAL_I2C_Mem_Read(&I2cHandle, PRESSURE_SENSOR_ADDRESS_HAL,
                          regAddress,
                          I2C_MEMADD_SIZE_8BIT, val, size,
                          PRESSURE_SENSOR_I2C_TIMEOUT);

    if (xSemaphoreGive(I2CMutex) != pdTRUE)
    {
        DEBUG_PRINT("Pressure failed to give I2C mut\n");
        return FC_ERROR;
    }

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

    if (xSemaphoreTake(I2CMutex, I2C_MUT_WAIT_TICKS) != pdTRUE)
    {
        DEBUG_PRINT("Pressure failed to take I2C mut\n");
        return FC_ERROR;
    }

    rc = HAL_I2C_Mem_Write(&I2cHandle, PRESSURE_SENSOR_ADDRESS_HAL,
                          regAddress,
                          I2C_MEMADD_SIZE_8BIT, &val, 1 /* 1 byte read */,
                          PRESSURE_SENSOR_I2C_TIMEOUT);

    if (xSemaphoreGive(I2CMutex) != pdTRUE)
    {
        DEBUG_PRINT("Pressure failed to give I2C mut\n");
        return FC_ERROR;
    }

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
        DEBUG_PRINT("Failed to read pressure whoami\n");
        return FC_ERROR;
    }

    if (whoami != PRESSURE_SENSOR_WHOAMI_RESPONSE) {
        DEBUG_PRINT("Whoami error. Got: %X, expected: %X\n", whoami,
                    PRESSURE_SENSOR_WHOAMI_RESPONSE);
        return FC_ERROR;
    }

    if (PressureSensor_RegWrite(PRESSURE_CTRL_REG2, _BIT(SWRESET)) != FC_OK)
    {
        DEBUG_PRINT("Failed to write pressure reg2\n");
        return FC_ERROR;
    }

    // Wait for 1 msec for chip to reset
    HAL_Delay(1);

    // Power on sensor and set output data rate to 25 Hz
    // Enabling BDU lock high and low registers until both read
    uint8_t tmp = _BIT(PD) | _BIT(ODR2) | _BIT(BDU);
    if (PressureSensor_RegWrite(PRESSURE_CTRL_REG1, tmp) != FC_OK)
    {
        DEBUG_PRINT("Failed to write pressure reg1\n");
        return FC_ERROR;
    }

    return FC_OK;
}

#else
// Prototypes for functions we want to mock
FC_Status PressureSensor_RegRead(uint8_t regAddress, uint8_t *val, int size);
FC_Status PressureSensor_RegWrite(uint8_t regAddress, uint8_t val);
#endif /* ndefined(__UNIT_TEST) */

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

    if (PressureSensor_RegRead(PRESSURE_TEMP_OUT_L, buffer, 2) != FC_OK)
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


/**
 * @brief Get the Pressure value in hPa
 *
 * @param Pout The pressure values in 100 x hPa, aka Pa
 *
 * @return Status [FC_OK, FC_ERROR]
 */
FC_Status pressureSensor_GetPressure(int32_t *Pout)
{
    ASSERT(Pout);

    uint8_t buffer[3];
    uint32_t tmp = 0;
    int32_t raw_pressure;

    if (PressureSensor_RegRead(PRESSURE_PRESS_OUT_XL, buffer, 3) != FC_OK)
    {
        DEBUG_PRINT("Error reading pressure\n");
        return FC_ERROR;
    }

    for (uint8_t i=0; i<3; i++)
    {
        tmp |= (((uint32_t)buffer[i]) << (8*i));
    }

    if (tmp & 0x00800000)
    {
        tmp |= 0xFF000000;
    }

    raw_pressure = ((int32_t)tmp);

    // Resolution 4096 LSB / hPa
    *Pout = (raw_pressure*100)/4096;

    return FC_OK;
}

/**
 * @brief Get the pressure from the pressure sensor, and convert to altitude
 *
 * @param altitude_out The altitude in meters x 100
 *
 * @return Status [FC_OK, FC_ERROR]
 */
FC_Status pressureSensor_GetAltitude(int32_t *altitude_out)
{
    ASSERT(altitude_out);

    /*const float T0 = 288.15;*/
    const float P0 = 101325.0;
    /*const float g  = 9.80655;*/
    /*const float L  = -0.0065;*/
    /*const float R  = 287.053;*/

    const float term1 = -44330.76923;
    const float term2 = 0.1902651289;

    float P = 0.0;
    int32_t Pressure;

    if (pressureSensor_GetPressure(&Pressure) != FC_OK)
    {
        return FC_ERROR;
    }

    P = (float)Pressure;

    float altitude = term1*(powf((P/P0),term2)-1);
    /*float altitude = (T0/L)*(powf((P/P0),((-L*R)/g))-1);*/ // Full equation

    altitude *= 100;

    (*altitude_out) = (int32_t)altitude;

    return FC_OK;
}

#ifndef __UNIT_TEST
void vPressureSensorTask(void *pvParameters)
{
    DEBUG_PRINT("Starting pressure sensor task\n");
    if (pressureSensorInit() != FC_OK)
    {
        Error_Handler("pressure init fail\n");
    }
    DEBUG_PRINT("Initialized pressure sensor\n");

    int16_t Temp;
    int32_t Pressure;
    int32_t Altitude;
    for ( ;; )
    {
        if (pressureSensor_GetTemp(&Temp) != FC_OK) {
            // Maybe do something???
        }
        if (pressureSensor_GetPressure(&Pressure) != FC_OK) {
            // Maybe do something???
        }
        if (pressureSensor_GetAltitude(&Altitude) != FC_OK) {
            // Maybe do something???
        }

        DEBUG_PRINT("Temp: %d\n", Temp);
        DEBUG_PRINT("Pessure: %ld\n", Pressure);
        DEBUG_PRINT("Altitude: %ld\n", Altitude);

        /*vTaskDelay(ODR_PERIOD_MS / portTICK_PERIOD_MS);*/
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

#endif /* ndefined(__UNIT_TEST) */
