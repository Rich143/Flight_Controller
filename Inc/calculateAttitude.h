#ifndef __CALCULATE_ATTITUDE_H
#define __CALCULATE_ATTITUDE_H

#include "imu.h"

/** 
 * @brief Struct to hold the attitude of the quad
 * The values are in degrees * 100
 */
typedef struct Attitude_t {
    int32_t roll;
    int32_t pitch;
    int32_t yaw;
} Attitude_t;

FC_Status calculateAttitude(Accel_t *accel, Attitude_t *attitudeOut);

#endif /* defined(__CALCULATE_ATTITUDE_H) */
