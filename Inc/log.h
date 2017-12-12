#ifndef __LOG_H
#define __LOG_H

#include "fc.h"
#include "pid.h"
#include "rate_control.h"

#include "freertos.h"
#include "queue.h"

typedef struct tLogMessage {
    Attitude_t desiredAttitude;
    Attitude_t actualAttitude;
    Rates_t desiredRates;
    Rates_t actualRates;
    RotationAxisOutputs_t motorOutputs;
    PidAllAxis_t PIDs_RateControl;
    PidAllAxis_t PIDs_AttitudeControl;
    FlightMode_t flightMode;
    uint32_t timestamp_ms;
} tLogMessage;

extern QueueHandle_t logQueue;

FC_Status logInit();
FC_Status logStart();
FC_Status logFinish();
FC_Status logWriteTest();
FC_Status logDataStruct(tLogMessage *msg);
FC_Status sendLogDataToQueue(Rates_t *desiredRates, Rates_t *actualRates,
                             RotationAxisOutputs_t *rotationOutputsPtr,
                             PidAllAxis_t *PIDs);

void vLogTask(void *pvParameters);

#endif /* defined(__LOG_H) */
