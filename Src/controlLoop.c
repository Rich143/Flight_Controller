#include <stdbool.h>

#include "fc.h"
#include "freertos.h"
#include "task.h"

#include "ppm.h"
#include "debug.h"
#include "motors.h"

void vControlLoopTask(void *pvParameters)
{
    FC_Status status;
    tPpmSignal ppmSignal = {0};

    DEBUG_PRINT("Control loop start\n");

    if (motorsStart() != FC_OK) {
        // Stop any started motors
        motorsStop();
        // Delay to make sure any motors that started receive low pwm and stop spinning
        HAL_Delay(100);
        // Stop motor pwm
        motorsDeinit();
        Error_Handler("Failed to start motor output");
    }

    DEBUG_PRINT("Waiting for low throttle\n");

    // Wait for throttle to be low before continuing startup
    // This is for safety
    status = FC_ERROR;
    while (status != FC_OK) {
        if (xQueueReceive(ppmSignalQueue, &ppmSignal, 100) == pdTRUE) {
            if (ppmSignal.signals[THROTTLE_CHANNEL] <= THROTTLE_LOW_THRESHOLD) {
                DEBUG_PRINT("Starting, thr %d\n", ppmSignal.signals[THROTTLE_CHANNEL]);
                status = FC_OK;
            }
        }
    }

    bool newPpmAvailable = false;
    bool armed = false;

    DEBUG_PRINT("Starting control loop\n");
    for ( ;; )
    {
        if (xQueueReceive(ppmSignalQueue, &ppmSignal, 0) == pdTRUE) {
            newPpmAvailable = true;

            // Check if we are still armed
            // only disarm if throttle is low, so don't accidentally disarm in
            // flight
            if (ppmSignal.signals[ARMED_SWITCH_CHANNEL] >= SWITCH_HIGH_THRESHOLD) {
                armed = true;
            } else if (ppmSignal.signals[ARMED_SWITCH_CHANNEL] <= SWITCH_LOW_THRESHOLD
                && ppmSignal.signals[THROTTLE_CHANNEL] <= THROTTLE_LOW_THRESHOLD ) {
                armed = false;
            }
        }

        if (armed) {
            if (newPpmAvailable) {
                setMotor(MOTOR_FRONT_LEFT, ppmSignal.signals[THROTTLE_CHANNEL]);
                setMotor(MOTOR_FRONT_RIGHT, ppmSignal.signals[THROTTLE_CHANNEL]);
                setMotor(MOTOR_BACK_LEFT, ppmSignal.signals[THROTTLE_CHANNEL]);
                setMotor(MOTOR_BACK_RIGHT, ppmSignal.signals[THROTTLE_CHANNEL]);
            }
        } else {
            motorsStop();
        }

        vTaskDelay(10);
    }
}
