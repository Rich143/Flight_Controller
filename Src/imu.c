#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "imu.h"
#include "ImuRegisters.h"
#include "debug.h"
#include "hardware.h"

// Sensor Sensitivity Constants
// Values set according to the typical specifications provided in
// table 3 of the LSM9DS1 datasheet. (pg 12)
#define SENSITIVITY_ACCELEROMETER_2  0.061f
#define SENSITIVITY_ACCELEROMETER_4  0.122f
#define SENSITIVITY_ACCELEROMETER_8  0.244f
#define SENSITIVITY_ACCELEROMETER_16 0.732f
#define SENSITIVITY_GYROSCOPE_245    8.75f
#define SENSITIVITY_GYROSCOPE_500    1.75f
#define SENSITIVITY_GYROSCOPE_2000   70f
#define SENSITIVITY_MAGNETOMETER_4   0.14f
#define SENSITIVITY_MAGNETOMETER_8   0.29f
#define SENSITIVITY_MAGNETOMETER_12  0.43f
#define SENSITIVITY_MAGNETOMETER_16  0.58f

FC_Status AccelGyro_RegRead(uint8_t regAddress, uint8_t *val, int size)
{
    HAL_StatusTypeDef rc;

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

    // CTRL_REG1_G set ODR 952, and max lpf
    // CTRL_REG2_G set INT_SEL 0 (disable interrupts) and OUT_SEL 0 (disable
    // hpf and lpf2
    // CTRL_REG3_G set to 0 (no low power or hpf)
    uint8_t tempreg = 0;
    tempreg |= 0x6 << 5; // set ODR 952 Hz
    tempreg |= 0x3 << 3; // set full scale 2000 dps
    if (AccelGyro_RegWrite(CTRL_REG1_G, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to gyro\n");
        return FC_ERROR;
    }

    // CTRL_REG5_XL set ODR 952, full scale set to 2g
    // CTRL_REG6_XL set no high resolution mode
    tempreg = 0;
    tempreg |= 0x6 << 5; // set ODR 952
                         // 0 sets scale 2g
    if (AccelGyro_RegWrite(CTRL_REG1_G, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to accel\n");
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status getAccel(AccelRaw_t *accelData)
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

    float x = ((float)(raw.x)) * SENSITIVITY_ACCELEROMETER_2;
    float y = ((float)(raw.y)) * SENSITIVITY_ACCELEROMETER_2;
    float z = ((float)(raw.z)) * SENSITIVITY_ACCELEROMETER_2;

    accelData->x = x;
    accelData->y = y;
    accelData->z = z;

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

    AccelRaw_t accel = {0};
    for ( ;; )
    {
        if (getAccel(&accel) != FC_OK) {
            // Do something
        }
        DEBUG_PRINT("Ax: %d, Ay: %d, Az: %d\n", accel.x, accel.y, accel.z);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
