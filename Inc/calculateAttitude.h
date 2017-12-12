#ifndef __CALCULATE_ATTITUDE_H
#define __CALCULATE_ATTITUDE_H

#include "imu.h"

FC_Status calculateAttitude(Accel_t *accel, Attitude_t *attitudeOut);

#endif /* defined(__CALCULATE_ATTITUDE_H) */
