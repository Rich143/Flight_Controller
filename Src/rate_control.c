#include "pid.h"
#include "rate_control.h"
#include "string.h"
#include "stdio.h"

#define RATE_LOOP_PERIOD_US 100

PID_Gains_t gains = {
    .K_P = 10,
    .K_I = 0.01,
    .K_D = 1
};

Limits_t rateLimits = {
    .min = ROTATION_AXIS_OUTPUT_MIN,
    .max = ROTATION_AXIS_OUTPUT_MAX
};

ControlInfo_t rollInfo = {.dt = RATE_LOOP_PERIOD_US};
ControlInfo_t pitchInfo = {.dt = RATE_LOOP_PERIOD_US};
ControlInfo_t yawInfo = {.dt = RATE_LOOP_PERIOD_US};

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

