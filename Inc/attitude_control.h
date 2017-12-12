#ifndef __ATTITUDE_CONTROL_H
#define __ATTITUDE_CONTROL_H

#include "pid.h"

Rates_t *controlAttitude(Attitude_t *actualAttitude, Attitude_t *desiredAttitude, bool controlYaw, PidAllAxis_t *PIDs);
void resetAttitudeControlInfo();
#endif
