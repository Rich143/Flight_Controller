#include <stdio.h>

#include "fc.h"

#include "pid.h"
#include "rate_control.h"
#include "string.h"
#include "controlLoop.h"

#ifndef __UNIT_TEST
#include "debug.h"
#endif

#define RATE_LOOP_PERIOD_MS CONTROL_LOOP_PERIOD_MS

// Testing on bench
PID_Gains_t gains = {
    2, // K_P
    0.01, // K_I
    1, // K_D
};

// Real
/*PID_Gains_t gains = {*/
    /*10, // K_P*/
    /*0.01, // K_I*/
    /*1 // K_D*/
/*};*/

Limits_t rateLimits = {
    ROTATION_AXIS_OUTPUT_MIN, // MIN
    ROTATION_AXIS_OUTPUT_MAX // MAX
};

ControlInfo_t rollInfo = {
    .dt = RATE_LOOP_PERIOD_MS,
    .integratedError = 0,
    .saturated = 0,
    .lastError = 0
};
ControlInfo_t pitchInfo = {
    .dt = RATE_LOOP_PERIOD_MS,
    .integratedError = 0,
    .saturated = 0,
    .lastError = 0
};
ControlInfo_t yawInfo = {
    .dt = RATE_LOOP_PERIOD_MS,
    .integratedError = 0,
    .saturated = 0,
    .lastError = 0
};

RotationAxisOutputs_t rotationOutputs = {0,0,0};

void resetRateInfo()
{
    rollInfo.integratedError = 0;
    rollInfo.saturated = 0;
    rollInfo.lastError = 0;

    pitchInfo.integratedError = 0;
    pitchInfo.saturated = 0;
    pitchInfo.lastError = 0;

    yawInfo.integratedError = 0;
    yawInfo.saturated = 0;
    yawInfo.lastError = 0;
}

RotationAxisOutputs_t* controlRates(Rates_t* actualRates, Rates_t* desiredRates,
                                    PidAllAxis_t *PIDs)
{
    ASSERT(actualRates);
    ASSERT(desiredRates);
    ASSERT(PIDs);

    int rollError = desiredRates->roll - actualRates->roll;
    int pitchError = desiredRates->pitch - actualRates->pitch;
    int yawError =  desiredRates->yaw - actualRates->yaw;

    rotationOutputs.roll = controlLoop(rollError, &rollInfo, &gains, &rateLimits, &(PIDs->roll));
    rotationOutputs.pitch = controlLoop(pitchError, &pitchInfo, &gains, &rateLimits, &(PIDs->pitch));
    rotationOutputs.yaw = controlLoop(yawError, &yawInfo, &gains, &rateLimits, &(PIDs->yaw));

    return &rotationOutputs;
}

