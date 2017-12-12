#ifndef __CONTROL_LOOP_H
#define __CONTROL_LOOP_H

#include "fc.h"

#ifndef __UNIT_TEST
#include "freertos.h"
#endif

#define CONTROL_LOOP_PERIOD_TICKS 5 // Note that this also controls IMU task loop time
#define CONTROL_LOOP_PERIOD_MS    (CONTROL_LOOP_PERIOD_TICKS / portTICK_PERIOD_MS)

/*
 * Timeout values, if no data is received within these periods, system failure is
 * assumed
 */
#define PPM_RX_TIMEOUT_MS       1000
#define ATTITUDE_RX_TIMEOUT_MS  (CONTROL_LOOP_PERIOD_MS * 5)
#define GYRO_RX_TIMEOUT_MS      (CONTROL_LOOP_PERIOD_MS * 5)
#define CONTROL_LOOP_TIMEOUT_MS (CONTROL_LOOP_PERIOD_MS * 5)

typedef enum FlightMode_t
{
    INVALID_FLIGHT_MODE = 0,
    RATE_MODE = 1,
    ATTITUDE_MODE = 2,
    MAX_FLIGHT_MODE_PLUS_1,
} FlightMode_t;

void vControlLoopTask(void *pvParameter);

#endif /* defined(__CONTROL_LOOP_H) */
