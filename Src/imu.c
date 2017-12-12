#include "fc.h"

#include "imu.h"
#include "ImuRegisters.h"
#include "rate_control.h"
#include "controlLoop.h"
#include "MadgwickAHRS.h"
#include "stdbool.h"

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

#define ACCELEROMETER_SCALE_2G 0b00
#define ACCELEROMETER_SCALE_16G 0b01
#define ACCELEROMETER_SCALE_4G 0b10
#define ACCELEROMETER_SCALE_8G 0b11

#define GYRO_SCALE_245DPS 0b00
#define GYRO_SCALE_500DPS 0b01
#define GYRO_SCALE_2000DPS 0b11

#define MAG_SCALE_4_GAUSS 0b00
#define MAG_SCALE_8_GAUSS 0b01
#define MAG_SCALE_12_GAUSS 0b10
#define MAG_SCALE_16_GAUSS 0b11

// Change this to change scale and associated sensitivity
#define MAG_SCALE MAG_SCALE_4_GAUSS
#define MAG_SENSITIVITY SENSITIVITY_MAGNETOMETER_4

// Change this to change scale and associated sensitivity
#define ACCEL_SCALE ACCELEROMETER_SCALE_2G
#define ACCEL_SENSITIVITY SENSITIVITY_ACCELEROMETER_2
//
// Change this to change scale and associated sensitivity
#define GYRO_SCALE GYRO_SCALE_2000DPS
#define GYRO_SENSITIVITY SENSITIVITY_GYROSCOPE_2000

#define MAG_READ_PERIOD_TICKS (13) // ODR 80 Hz, so period ~13 ticks

/*
 * Calibration values from magneto
 */
const int32_t magBias[3] = {204, -36, -7}; // Calibrated bias values from magneto
const float   magScaleFactors[3][3] =
                    {{1.066721, 0.057005, -0.002955},
                     {0.057005, 1.035208, 0.010962},
                     {-0.002955, 0.010962, 1.073667}};

const int32_t accelBias[3] = {-12, 18, -68}; // Calibrated bias values from magneto
const float   accelScaleFactors[3][3] =
                    {{0.990691, 0.010089, 0.000054},
                     {0.010089, 1.008581, 0.004460},
                     {0.000054, 0.004460, 0.992238}};
#ifndef __UNIT_TEST

#define RATES_QUEUE_LENGTH 1 // only care about most recent element, so keep it short

QueueHandle_t ratesQueue;

FC_Status Mag_RegRead(uint8_t regAddress, uint8_t *val, int size)
{
    HAL_StatusTypeDef rc;

    if (xSemaphoreTake(I2CMutex, I2C_MUT_WAIT_TICKS) != pdTRUE)
    {
        DEBUG_PRINT("Mag failed to take I2C mut\n");
        return FC_ERROR;
    }

    // Only use DMA if transfer is large enough to make it worthwile
    if (size > 1) {
        rc = HAL_I2C_Mem_Read_DMA(&I2cHandle, MAG_ADDRESS_HAL, regAddress,
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
        rc = HAL_I2C_Mem_Read(&I2cHandle, MAG_ADDRESS_HAL, regAddress,
                              I2C_MEMADD_SIZE_8BIT, val, size, IMU_I2C_TIMEOUT);
    }

    if (xSemaphoreGive(I2CMutex) != pdTRUE)
    {
        DEBUG_PRINT("Mag failed to give I2C mut\n");
        rc = HAL_ERROR;
    }

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("Mag Reg read fail %d\n", rc);
        // Attempt to clear busy flag/timeout error
        I2C_ClearBusyFlagErratum(10);
        DEBUG_PRINT("Attempted to reset i2c\n");
    }

    return rc;
}

FC_Status Mag_RegWrite(uint8_t regAddress, uint8_t val)
{
    HAL_StatusTypeDef rc;

    if (xSemaphoreTake(I2CMutex, I2C_MUT_WAIT_TICKS) != pdTRUE)
    {
        DEBUG_PRINT("Mag failed to take I2C mut\n");
        return FC_ERROR;
    }

    rc = HAL_I2C_Mem_Write(&I2cHandle, MAG_ADDRESS_HAL,
                          regAddress,
                          I2C_MEMADD_SIZE_8BIT, &val, 1 /* 1 byte read */,
                          IMU_I2C_TIMEOUT);

    if (xSemaphoreGive(I2CMutex) != pdTRUE)
    {
        DEBUG_PRINT("Mag failed to give I2C mut\n");
        return FC_ERROR;
    }

    if (rc != HAL_OK)
    {
        DEBUG_PRINT("Mag Reg write failed: %d\n", rc);
        I2C_ClearBusyFlagErratum(10);
        DEBUG_PRINT("Attempted to reset i2c\n");
        return FC_ERROR;
    }

    return FC_OK;
}

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
        // Attempt to clear busy flag/timeout error
        I2C_ClearBusyFlagErratum(10);
        DEBUG_PRINT("Attempted to reset i2c\n");
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
        I2C_ClearBusyFlagErratum(10);
        DEBUG_PRINT("Attempted to reset i2c\n");
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
        DEBUG_PRINT("XG Whoami error. Got 0x%X, expected 0x%X\n", whoami, WHO_AM_I_AG_RSP);
        return FC_ERROR;
    }

    /*************
     *
     * Accelerometer and Gyro Configuration
     *
     *************/

    // CTRL_REG1_G set ODR 952, and max lpf
    // CTRL_REG2_G set INT_SEL 0 (disable interrupts) and OUT_SEL 0 (disable
    // hpf and lpf2
    // CTRL_REG3_G set to 0 (no low power or hpf)
    uint8_t tempreg = 0;
    tempreg |= 0x6 << 5; // set ODR 952 Hz
    tempreg |= GYRO_SCALE << 3; // set full scale 2000 dps
    tempreg |= 0x3; // set BW bits to 0b11 to set gryo lpf to 100 Hz (assumes ODR 952 Hz)
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

    /*************
     *
     * Magnetometer Configuration
     *
     *************/
    if (Mag_RegRead(WHO_AM_I_M, &whoami, 1) != FC_OK)
    {
        DEBUG_PRINT("Failed to read mag whoami\n");
        return FC_ERROR;
    }

    if (whoami != WHO_AM_I_M_RSP)
    {
        DEBUG_PRINT("Mag Whoami error. Got 0x%X, expected 0x%X\n", whoami, WHO_AM_I_M_RSP);
        return FC_ERROR;
    }

    tempreg = 0;
    tempreg |= _BIT(7); // Enable temperature compensation
    tempreg |= _BIT(6) | _BIT(5); // Enable ultra high performance mode
    tempreg |= _BIT(4) | _BIT(3) | _BIT(2); // ODR 80 Hz
    if (Mag_RegWrite(CTRL_REG1_M, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to mag\n");
        return FC_ERROR;
    }

    tempreg = 0;
    tempreg |= MAG_SCALE << 5;
    if (Mag_RegWrite(CTRL_REG2_M, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to mag\n");
        return FC_ERROR;
    }

    tempreg = 0; // write 0 to enable continuous conversion
    if (Mag_RegWrite(CTRL_REG3_M, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to mag\n");
        return FC_ERROR;
    }

    tempreg = _BIT(3) | _BIT(2); // enable ultra high performance z axis
                                 // set little endian (default)
    if (Mag_RegWrite(CTRL_REG4_M, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to mag\n");
        return FC_ERROR;
    }

    tempreg = _BIT(6); // enable block data update. Data registers are not
                       // updated until the MSB and LSB have been read
    if (Mag_RegWrite(CTRL_REG5_M, tempreg) != FC_OK)
    {
        DEBUG_PRINT("Failed to write to mag\n");
        return FC_ERROR;
    }

#ifndef __UNIT_TEST
    ratesQueue = xQueueCreate(RATES_QUEUE_LENGTH, sizeof(Rates_t));

    if (ratesQueue == NULL)
    {
        DEBUG_PRINT("Failed to create rates queue\n");
    }
#endif

    return FC_OK;
}

FC_Status getMag(Mag_t *mag)
{
	uint8_t temp[6]; // read six bytes from the accelerometer into temp

	if (Mag_RegRead(OUT_X_L_M, temp, 6) != FC_OK) // Read 6 bytes, beginning at OUT_X_L_XL
	{
        DEBUG_PRINT("Failed to read from mag\n");
        return FC_ERROR;
    }

    int16_t raw_x = (temp[1] << 8) | temp[0]; // Store x-axis values
    int16_t raw_y = (temp[3] << 8) | temp[2]; // Store y-axis values
    int16_t raw_z = (temp[5] << 8) | temp[4]; // Store z-axis values

    float x = ((float)(raw_x)) * MAG_SENSITIVITY;
    float y = ((float)(raw_y)) * MAG_SENSITIVITY;
    float z = ((float)(raw_z)) * MAG_SENSITIVITY;

    x = x - magBias[0];
    y = y - magBias[1];
    z = z - magBias[2];

    float x_cal = magScaleFactors[0][0] * x + magScaleFactors[0][1] * y + magScaleFactors[0][2] * z;
    float y_cal = magScaleFactors[1][0] * x + magScaleFactors[1][1] * y + magScaleFactors[1][2] * z;
    float z_cal = magScaleFactors[2][0] * x + magScaleFactors[2][1] * y + magScaleFactors[2][2] * z;

    /*mag->x = x;*/
    /*mag->y = -1*y;*/
    /*mag->z = -1*z;*/
    mag->x = x_cal;
    mag->y = y_cal;
    mag->z = z_cal;

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

    x = x - accelBias[0];
    y = y - accelBias[1];
    z = z - accelBias[2];

    float x_cal = accelScaleFactors[0][0] * x + accelScaleFactors[0][1] * y + accelScaleFactors[0][2] * z;
    float y_cal = accelScaleFactors[1][0] * x + accelScaleFactors[1][1] * y + accelScaleFactors[1][2] * z;
    float z_cal = accelScaleFactors[2][0] * x + accelScaleFactors[2][1] * y + accelScaleFactors[2][2] * z;

    accelData->x = x_cal;
    accelData->y = y_cal;
    accelData->z = z_cal;
    /*accelData->x = x;*/
    /*accelData->y = y;*/
    /*accelData->z = z;*/

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

    /*gyroData->x = -1 * x;*/
    /*gyroData->y = -1 * y; // Pitch seems to be reversed for gyro, simple fix is to negate it here*/
    /*gyroData->z = -1 * z; // Yaw seems to be reversed for gyro, simple fix is to negate it here*/
    gyroData->x = x;
    gyroData->y = y; // Pitch seems to be reversed for gyro, simple fix is to negate it here
    gyroData->z = z; // Yaw seems to be reversed for gyro, simple fix is to negate it here

    return FC_OK;
}

FC_Status getRates(Rates_t *rates)
{
    Gyro_t gyro;

    if (getGyro(&gyro) != FC_OK) {
        DEBUG_PRINT("Error reading gyro\n");
        return FC_ERROR;
    }

    /*DEBUG_PRINT("gx: %ld, gy: %ld, gz: %ld\n", gyro.x, gyro.y, gyro.z);*/
    rates->roll = gyro.x / 1000;
    rates->pitch = gyro.y / 1000;
    rates->yaw = gyro.z / 1000;

    return FC_OK;
}

TickType_t lastMagRead  = 0;
float mx;
float my;
float mz;
FC_Status getAttitude(Attitude_t *attitude)
{
    Gyro_t gyro;
    Accel_t accel;
    Mag_t mag;

    if (getGyro(&gyro) != FC_OK)
    {
        DEBUG_PRINT("Failed to get gyro data\n");
        return FC_ERROR;
    }

    float gx = gyro.y / 1000.0;
    float gy = gyro.x / 1000.0;
    float gz = gyro.z / 1000.0;

    if (getAccel(&accel) != FC_OK)
    {
        DEBUG_PRINT("Failed to get accel data\n");
        return FC_ERROR;
    }

    float ax = accel.y;
    float ay = accel.x;
    float az = accel.z;

    if (xTaskGetTickCount() - lastMagRead >= MAG_READ_PERIOD_TICKS) {
        if (getMag(&mag) != FC_OK)
        {
            DEBUG_PRINT("Failed to get mag data\n");
            return FC_ERROR;
        }

        mx = mag.y;
        my = -1 * mag.x;
        mz = mag.z;
    }

    for (int i=0; i < 20; i++) {
        madgwickUpdate(gx,gy,gz,ax,ay,az,mx,my,mz);
    }

    attitude->roll = -1*getRoll();
    attitude->pitch = getPitch();
    attitude->yaw = -1*getYaw();

    return FC_OK;
}


#ifndef __UNIT_TEST
void vIMUTask(void *pvParameters)
{
    DEBUG_PRINT("Starting IMU Task\n");
    if (IMU_Init() != FC_OK)
    {
        DEBUG_PRINT("IMU init failed\n");
        vTaskSuspend(NULL); 
    }
    DEBUG_PRINT("Initialized IMU\n");

    madgwickInit(4000);

    /*Rates_t rates = {0};*/
    Attitude_t attitude = {0};
    TickType_t lastWakeTime = xTaskGetTickCount();
    TickType_t lastPrintTime  = xTaskGetTickCount();
    /*TickType_t lastMagRead  = xTaskGetTickCount();*/
    for ( ;; )
    {
        /*if (getRates(&rates) == FC_OK) {*/
            /*xQueueOverwrite(ratesQueue, (void *)&rates);*/
        /*}*/
        /*else {*/
            /*DEBUG_PRINT("Error getting rates\n");*/
        /*}*/

        /*if (xTaskGetTickCount() - lastMagRead >= MAG_READ_PERIOD_TICKS)*/
        /*{*/
            if (getAttitude(&attitude) == FC_OK)
            {
                /*DEBUG_PRINT("roll: %ld, pitch: %ld, yaw: %ld\n", attitude.roll,*/
                            /*attitude.pitch, attitude.yaw);*/
                /*DEBUG_PRINT("Orientation: %ld %ld %ld\n", attitude.roll,*/
                            /*attitude.pitch, attitude.yaw);*/
            } else {
                DEBUG_PRINT("Failed to get attitude\n");
            }

            if (xTaskGetTickCount() - lastPrintTime > 50) {
                DEBUG_PRINT("Orientation: %ld %ld %ld\n", attitude.roll,
                            attitude.pitch, attitude.yaw);
                lastPrintTime = xTaskGetTickCount();
            }
            /*lastMagRead = xTaskGetTickCount();*/
        /*}*/

        /*Mag_t mag;*/
        /*if (getMag(&mag) == FC_OK)*/
        /*{*/
            /*DEBUG_PRINT("x: %ld, y: %ld, z: %ld\n", mag.x, mag.y, mag.z);*/
        /*} else {*/
            /*DEBUG_PRINT("Failed to read mag\n");*/
        /*}*/

        // This should run at the same rate as the control loop
        // as there is no point running the control loop without new data
        /*vTaskDelayUntil(&lastWakeTime, CONTROL_LOOP_PERIOD_TICKS);*/
        vTaskDelayUntil(&lastWakeTime, 5);
    }
}
#endif
