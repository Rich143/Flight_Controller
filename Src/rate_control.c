#include "pid.h"
#include "rate_control.h"
#include "string.h"
#include "stdio.h"

#define RATE_LOOP_PERIOD_US 100

PID_Gains_t gains = {
    10, // K_P
    0.01, // K_I
    1 // K_D
};

Limits_t rateLimits = {
    ROTATION_AXIS_OUTPUT_MIN, // MIN
    ROTATION_AXIS_OUTPUT_MAX // MAX
};

ControlInfo_t rollInfo = {
    RATE_LOOP_PERIOD_US,
    0,
    0,
    0
};
ControlInfo_t pitchInfo = {
    RATE_LOOP_PERIOD_US,
    0,
    0,
    0
};
ControlInfo_t yawInfo = {
    RATE_LOOP_PERIOD_US,
    0,
    0,
    0
};

RotationAxisOutputs_t rotationOutputs = {0};

void resetRateInfo()
{
    memset(&rollInfo, 0, sizeof(rollInfo));
    memset(&pitchInfo, 0, sizeof(pitchInfo));
    memset(&yawInfo, 0, sizeof(yawInfo));
    memset(&rotationOutputs, 0, sizeof(rotationOutputs));
}

RotationAxisOutputs_t* controlRates(Rates_t* actualRates, Rates_t* desiredRates)
{
    int rollError = desiredRates->roll - actualRates->roll;
    int pitchError = desiredRates->pitch - actualRates->pitch;
    int yawError =  desiredRates->yaw - actualRates->yaw;

    rotationOutputs.roll = controlLoop(rollError, &rollInfo, &gains, &rateLimits);
    rotationOutputs.pitch = controlLoop(pitchError, &pitchInfo, &gains, &rateLimits);
    rotationOutputs.yaw = controlLoop(yawError, &yawInfo, &gains, &rateLimits);

    return &rotationOutputs;
}

