#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "imu.h"
#include "ImuRegisters.h"
#include "debug.h"
#include "hardware.h"

FC_Status AccelGyro_RegRead(uint8_t regAddress, uint8_t *val, int size)
{
    HAL_StatusTypeDef rc;

    if (size > 1)
    {
        ASSERT(0); // Not yet implemented
    }

    rc = HAL_I2C_Mem_Read(&I2cHandle, ACCEL_GYRO_ADDRESS_HAL, regAddress,
                          I2C_MEMADD_SIZE_8BIT, val, size, IMU_I2C_TIMEOUT);

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("AccelGyro Reg read fail %d\n", rc);
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status AccelGyro_RegWrite(uint8_t regAddress, uint8_t val)
{
    HAL_StatusTypeDef rc;

    rc = HAL_I2C_Mem_Write(&I2cHandle, ACCEL_GYRO_ADDRESS_HAL,
                          regAddress,
                          I2C_MEMADD_SIZE_8BIT, &val, 1 /* 1 byte read */,
                          IMU_I2C_TIMEOUT);

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("AccelGyro Reg write failed: %d\n", rc);
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status IMU_Init(void)
{
    uint8_t whoami = 0;

    if (AccelGyro_RegRead(WHO_AM_I_XG, &whoami, 1 /* 1 byte read */) != FC_OK)
    {
        DEBUG_PRINT("Failed to read xg whoami\n");
        return FC_ERROR;
    }

    if (whoami != WHO_AM_I_AG_RSP)
    {
        DEBUG_PRINT("Whoami error. Got 0x%X, expected 0x%X\n", whoami, WHO_AM_I_AG_RSP);
        return FC_ERROR;
    }

    return FC_OK;
}

void vIMUTask(void *pvParameters)
{
    DEBUG_PRINT("Starting IMU Task\n");
    if (IMU_Init() != FC_OK)
    {
        DEBUG_PRINT("IMU init failed\n");
        while(1);
    }
    DEBUG_PRINT("Initialized IMU\n");

    for ( ;; )
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
