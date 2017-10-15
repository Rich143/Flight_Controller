#ifndef __IMU_H
#define __IMU_H

#include "fc.h"

typedef struct Accel_t {
    int32_t x;
    int32_t y;
    int32_t z;
} Accel_t;

void vIMUTask(void *pvParameters);
#endif /*defined(__IMU_H)*/
