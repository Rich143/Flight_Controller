#include "stdbool.h"

#include "fc.h"

#include "pid.h"
#include "attitude_control.h"
#include "rate_control.h"

Limits_t rotationRatesLimit = {RATES_MIN, RATES_MAX};

PID_Gains_t attitudeGains = {
    2, // K_P
    0, // K_I
    0, // K_D
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

Rates_t ratesOutput = {0,0,0};

void resetAttitudeControlInfo()
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
Rates_t *controlAttitude(Attitude_t *actualAttitude, Attitude_t *desiredAttitude, bool controlYaw, PidAllAxis_t *PIDs)
{
    ASSERT(actualAttitude);
    ASSERT(desiredAttitude);
    ASSERT(PIDs);

    int rollError = desiredAttitude->roll - actualAttitude->roll;
    int pitchError = desiredAttitude->pitch - actualAttitude->pitch;
    if (controlYaw) {
        int yawError = desiredAttitude->yaw - actualAttitude->yaw;
    }

    rotationOutputs.roll = controlLoop(rollError, &rollInfo, &attitudeGains, &rotationRatesLimit, &(PIDs->roll));
    rotationOutputs.pitch = controlLoop(pitchError, &pitchInfo, &attitudeGains, &rotationRatesLimit, &(PIDs->pitch));
    if (controlYaW) {
        rotationOutputs.yaw = controlLoop(yawError, &yawInfo, &attitudeGains, &rotationRatesLimit, &(PIDs->yaw));
    }

    return &ratesOutput;
}
