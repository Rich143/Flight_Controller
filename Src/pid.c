#include "pid.h"

int satLimit(int val, int min, int max, int *saturated)
{
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

int limit(int val, int min, int max)
{
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

int controlLoop(int error, ControlInfo_t *info)
{
    if ((info->saturated > 0 && error > 0)
        || (info->saturated < 0 && error < 0)) {
        // Do Nothing
    } else {
        info->integratedError += error * info->K_I * info->dt;
        info->integratedError = satLimit(info->integratedError,
                                         info->minValue,
                                         info->maxValue, &info->saturated);
    }

    int ret = error * info->K_P + info->integratedError
        + (error - info->lastError) * info->K_D * info->dt;

    ret = limit(ret, info->minValue, info->maxValue);

    info->lastError = error;

    return ret;
}
