#include "fc.h"
#include "filters.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

FC_Status lpfInit(uint32_t filterCutoff, uint32_t sampleRate, LPFInfo_t *filterInfo)
{
    filterInfo->alpha = (2 * M_PI * sampleRate * filterCutoff)
                        / (2 * M_PI * sampleRate * filterCutoff + 1);

    return FC_OK;
}

int lpf(uint32_t newValue, LPFInfo_t *filterInfo)
{
    int out = filterInfo->lastValue
        + filterInfo->alpha * (newValue - filterInfo->lastValue);

    return out;
}
