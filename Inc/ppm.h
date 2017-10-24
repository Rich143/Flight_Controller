#ifndef __PPM_H
#define __PPM_H

#include "fc.h"
#include "freertos.h"
#include "queue.h"
#include "rc.h"

typedef struct tPpmSignal {
    uint16_t signals[RC_CHANNEL_IN_COUNT];
} tPpmSignal;

extern TIM_HandleTypeDef htim5;
extern QueueHandle_t ppmSignalQueue;

void ppmInit(void);
void vRCTask(void *pvParameters);
#endif /* defined(__PPM_H) */
