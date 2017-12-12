#ifndef __IMU_H
#define __IMU_H

#include "fc.h"

#ifndef __UNIT_TEST
#include "freertos.h"
#include "queue.h"

extern QueueHandle_t ratesQueue;
extern QueueHandle_t attitudeQueue;
#endif

typedef struct Mag_t {
    int32_t x;
    int32_t y;
    int32_t z;
} Mag_t;

typedef struct Accel_t {
    int32_t x;
    int32_t y;
    int32_t z;
} Accel_t;

typedef struct AccelRaw_t {
    int16_t x;
    int16_t y;
    int16_t z;
} AccelRaw_t;

typedef struct Gyro_t {
    int32_t x;
    int32_t y;
    int32_t z;
} Gyro_t;

typedef struct GyroRaw_t {
    int16_t x;
    int16_t y;
    int16_t z;
} GyroRaw_t;

/** 
 * @brief Struct to hold the attitude of the quad
 * The values are in degrees
 */
typedef struct Attitude_t {
    int32_t roll;
    int32_t pitch;
    int32_t yaw;
} Attitude_t;

FC_Status getAccel(Accel_t *accelData);
FC_Status getGyro(Gyro_t *gyroData);
void vIMUTask(void *pvParameters);
#endif /*defined(__IMU_H)*/
