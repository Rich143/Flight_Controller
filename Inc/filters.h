#ifndef __FILTERS_H
#define __FILTERS_H

typedef struct LPFInfo_t {
    uint32_t alpha; // lpf smoothing factor, constant after init
    int lastValue; // Store last output value to perform filtering
} LPFInfo_t;


FC_Status lpfInit(uint32_t filterCutoff, uint32_t sampleRate, LPFInfo_t *filterInfo);
int lpf(uint32_t newValue, LPFInfo_t *filterInfo);

#endif /* define(__FILTERS_H) */
