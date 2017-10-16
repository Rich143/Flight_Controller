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
void vIMUTask(void *pvParameters);
#endif /*defined(__IMU_H)*/
