#include "fc.h"

#include "imu.h"
#include "ImuRegisters.h"

#ifndef __UNIT_TEST

#include "freertos.h"
#include "task.h"

#include "debug.h"
#include "i2c.h"

#endif

// Sensor Sensitivity Constants
// Values set according to the typical specifications provided in
// table 3 of the LSM9DS1 datasheet. (pg 12)
#define SENSITIVITY_ACCELEROMETER_2  0.061f
#define SENSITIVITY_ACCELEROMETER_4  0.122f
#define SENSITIVITY_ACCELEROMETER_8  0.244f
#define SENSITIVITY_ACCELEROMETER_16 0.732f
#define SENSITIVITY_GYROSCOPE_245    8.75f
#define SENSITIVITY_GYROSCOPE_500    1.75f
#define SENSITIVITY_GYROSCOPE_2000   70.0f
#define SENSITIVITY_MAGNETOMETER_4   0.14f
#define SENSITIVITY_MAGNETOMETER_8   0.29f
#define SENSITIVITY_MAGNETOMETER_12  0.43f
#define SENSITIVITY_MAGNETOMETER_16  0.58f

#define ACCELEROMETER_SCALE_2G 00
#define ACCELEROMETER_SCALE_16G 01
#define ACCELEROMETER_SCALE_4G 10
#define ACCELEROMETER_SCALE_8G 11

#define GYRO_SCALE_245DPS 00
#define GYRO_SCALE_500DPS 01
#define GYRO_SCALE_2000DPS 11

// Change this to change scale and associated sensitivity
#define ACCEL_SCALE ACCELEROMETER_SCALE_2G
#define ACCEL_SENSITIVITY SENSITIVITY_ACCELEROMETER_2
//
// Change this to change scale and associated sensitivity
#define GYRO_SCALE GYRO_SCALE_2000DPS
#define GYRO_SENSITIVITY SENSITIVITY_GYROSCOPE_2000

#ifndef __UNIT_TEST

FC_Status AccelGyro_RegRead(uint8_t regAddress, uint8_t *val, int size)
{
    HAL_StatusTypeDef rc;

    if (xSemaphoreTake(I2CMutex, I2C_MUT_WAIT_TICKS) != pdTRUE)
    {
        DEBUG_PRINT("AccelGyro failed to take I2C mut\n");
        return FC_ERROR;
    }

    // Only use DMA if transfer is large enough to make it worthwile
    if (size > 1) {
        rc = HAL_I2C_Mem_Read_DMA(&I2cHandle, ACCEL_GYRO_ADDRESS_HAL, regAddress,
                          I2C_MEMADD_SIZE_8BIT, val, size);

        if (rc == HAL_OK)
        {
            // Wait for dma read to finish
            // I2C_DMA_CompleteSem is posted to by the rx complete callback
            if( xSemaphoreTake( I2C_DMA_CompleteSem, I2C_DMA_SEM_WAIT_TICKS ) != pdTRUE )
            {
                DEBUG_PRINT("I2C DMA timeout\n");
                rc = HAL_ERROR;
            }
        }
    } else {
        rc = HAL_I2C_Mem_Read(&I2cHandle, ACCEL_GYRO_ADDRESS_HAL, regAddress,
                              I2C_MEMADD_SIZE_8BIT, val, size, IMU_I2C_TIMEOUT);
    }

    if (xSemaphoreGive(I2CMutex) != pdTRUE)
    {
        DEBUG_PRINT("AccelGyro failed to give I2C mut\n");
        rc = HAL_ERROR;
    }

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("AccelGyro Reg read fail %d\n", rc);
    }

    return rc;
}

FC_Status AccelGyro_RegWrite(uint8_t regAddress, uint8_t val)
{
    HAL_StatusTypeDef rc;

    if (xSemaphoreTake(I2CMutex, I2C_MUT_WAIT_TICKS) != pdTRUE)
    {
        DEBUG_PRINT("AccelGyro failed to take I2C mut\n");
        return FC_ERROR;
    }

    rc = HAL_I2C_Mem_Write(&I2cHandle, ACCEL_GYRO_ADDRESS_HAL,
                          regAddress,
                          I2C_MEMADD_SIZE_8BIT, &val, 1 /* 1 byte read */,
                          IMU_I2C_TIMEOUT);

    if (xSemaphoreGive(I2CMutex) != pdTRUE)
    {
        DEBUG_PRINT("AccelGyro failed to give I2C mut\n");
        return FC_ERROR;
    }

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("AccelGyro Reg write failed: %d\n", rc);
        return FC_ERROR;
    }

    return FC_OK;
}

#else
// Declare these as extern to allow for mocking in unit tests
FC_Status AccelGyro_RegRead(uint8_t regAddress, uint8_t *val, int size);
FC_Status AccelGyro_RegWrite(uint8_t regAddress, uint8_t val);
#endif

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

    // CTRL_REG1_G set ODR 952, and max lpf
    // CTRL_REG2_G set INT_SEL 0 (disable interrupts) and OUT_SEL 0 (disable
    // hpf and lpf2
    // CTRL_REG3_G set to 0 (no low power or hpf)
    uint8_t tempreg = 0;
    tempreg |= 0x6 << 5; // set ODR 952 Hz
    tempreg |= GYRO_SCALE << 3; // set full scale 2000 dps
    if (AccelGyro_RegWrite(CTRL_REG1_G, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to gyro\n");
        return FC_ERROR;
    }

    // CTRL_REG5_XL set ODR 952, full scale set to 2g
    // CTRL_REG6_XL set no high resolution mode
    tempreg = 0;
    tempreg |= 0x6 << 5; // set ODR 952
    tempreg |= ACCEL_SCALE << 3; // set the scale
    if (AccelGyro_RegWrite(CTRL_REG6_XL, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to accel\n");
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status getAccel(Accel_t *accelData)
{
	uint8_t temp[6]; // read six bytes from the accelerometer into temp
    AccelRaw_t raw;
	if (AccelGyro_RegRead(OUT_X_L_XL, temp, 6) != FC_OK) // Read 6 bytes, beginning at OUT_X_L_XL
	{
        DEBUG_PRINT("Failed to read from accel\n");
        return FC_ERROR;
    }

    raw.x = (temp[1] << 8) | temp[0]; // Store x-axis values
    raw.y = (temp[3] << 8) | temp[2]; // Store y-axis values
    raw.z = (temp[5] << 8) | temp[4]; // Store z-axis values

    float x = ((float)(raw.x)) * ACCEL_SENSITIVITY;
    float y = ((float)(raw.y)) * ACCEL_SENSITIVITY;
    float z = ((float)(raw.z)) * ACCEL_SENSITIVITY;

    accelData->x = x;
    accelData->y = y;
    accelData->z = z;

    return FC_OK;
}

FC_Status getGyro(Gyro_t *gyroData)
{
	uint8_t temp[6]; // read six bytes from the accelerometer into temp
    GyroRaw_t raw;
	if (AccelGyro_RegRead(OUT_X_L_G, temp, 6) != FC_OK) // Read 6 bytes, beginning at OUT_X_L_G
	{
        DEBUG_PRINT("Failed to read from Gyro\n");
        return FC_ERROR;
    }

    raw.x = (temp[1] << 8) | temp[0]; // Store x-axis values
    raw.y = (temp[3] << 8) | temp[2]; // Store y-axis values
    raw.z = (temp[5] << 8) | temp[4]; // Store z-axis values

    float x = ((float)(raw.x)) * GYRO_SENSITIVITY;
    float y = ((float)(raw.y)) * GYRO_SENSITIVITY;
    float z = ((float)(raw.z)) * GYRO_SENSITIVITY;

    gyroData->x = x;
    gyroData->y = y;
    gyroData->z = z;

    return FC_OK;
}

#ifndef __UNIT_TEST
void vIMUTask(void *pvParameters)
{
    DEBUG_PRINT("Starting IMU Task\n");
    if (IMU_Init() != FC_OK)
    {
        DEBUG_PRINT("IMU init failed\n");
        while(1);
    }
    DEBUG_PRINT("Initialized IMU\n");

    Accel_t accel = {0};
    Gyro_t gyro = {0};
    for ( ;; )
    {
        if (getAccel(&accel) != FC_OK) {
            // Do something
        }
        if (getGyro(&gyro) != FC_OK) {
            // Do something
        }
        DEBUG_PRINT("Ax: %ld, Ay: %ld, Az: %ld\n", accel.x, accel.y, accel.z);
        DEBUG_PRINT("Gx: %ld, Gy: %ld, Gz: %ld\n", gyro.x, gyro.y, gyro.z);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
#endif
