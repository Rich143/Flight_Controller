#ifndef __IMU_H
#define __IMU_H

#include "fc.h"

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

FC_Status getAccel(Accel_t *accelData);
FC_Status getGyro(Gyro_t *gyroData);
void vIMUTask(void *pvParameters);
#endif /*defined(__IMU_H)*/
