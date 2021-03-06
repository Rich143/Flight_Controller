#include "pid.h"
#include "fc.h"

int satLimit(int val, int min, int max, int *saturated)
{
    ASSERT(saturated);

    if (val < min) {
        (*saturated) = -1;
        return min;
    } else if (val > max) {
        (*saturated) = 1;
        return max;
    } else {
        (*saturated) = 0;
        return val;
    }
}


int controlLoop(int error, ControlInfo_t *info, PID_Gains_t *gain,
                Limits_t* limits)
{
    ASSERT(gain);
    ASSERT(info);
    ASSERT(limits);

    if ((info->saturated > 0 && error > 0)
        || (info->saturated < 0 && error < 0)) {
        // Do Nothing
    } else {
        info->integratedError += error * gain->K_I * info->dt;
        info->integratedError = satLimit(info->integratedError,
                                         limits->min,
                                         limits->max, &info->saturated);
    }

    int ret = error * gain->K_P + info->integratedError
        + (error - info->lastError) * gain->K_D * info->dt;

    ret = limit(ret, limits->min, limits->max);

    info->lastError = error;

    return ret;
}
