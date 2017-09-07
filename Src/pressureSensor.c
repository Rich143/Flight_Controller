#include "freertos.h"
#include "task.h"

#include "fc.h"
#include "pressureSensor.h"
#include "hardware.h"
#include "debug.h"

#define PRESSURE_SENSOR_I2C_TIMEOUT 1000

#define PRESSURE_SENSOR_ADDRESS 0x5D
#define PRESSURE_SENSOR_ADDRESS_HAL (PRESSURE_SENSOR_ADDRESS<<1) // Left shift it 1 bit, the way the HAL functions want it
#define PRESSURE_SENSOR_WHOAMI_RESPONSE 0xBD

#define PRESSURE_REG_WHOAMI 0x0F

FC_Status pressureSensorInit(void)
{
    uint8_t whoami;
    HAL_StatusTypeDef rc;
    rc = HAL_I2C_Mem_Read(&I2cHandle, PRESSURE_SENSOR_ADDRESS_HAL,
                          PRESSURE_REG_WHOAMI,
                          I2C_MEMADD_SIZE_8BIT, &whoami, 1 /* 1 byte read */,
                          PRESSURE_SENSOR_I2C_TIMEOUT);
    if (rc != HAL_OK) {
        DEBUG_PRINT("I2C Mem read failed: %d\n", rc);
        return FC_ERROR;
    }

    if (whoami != PRESSURE_SENSOR_WHOAMI_RESPONSE) {
        DEBUG_PRINT("Whoami error. Got: %X, expected: %X\n", whoami,
                    PRESSURE_SENSOR_WHOAMI_RESPONSE);
        return FC_ERROR;
    }

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

    for ( ;; )
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}



