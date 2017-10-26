#include <stdbool.h>

#include "fc.h"
#include "freertos.h"
#include "task.h"

#include "ppm.h"
#include "debug.h"
#include "motors.h"
#include "rate_control.h"
#include "imu.h"

FC_Status controlLoopInit()
{
    if (motorsStart() != FC_OK) {
        // Stop any started motors
        motorsStop();
        // Delay to make sure any motors that started receive low pwm and stop spinning
        HAL_Delay(100);
        // Stop motor pwm
        motorsDeinit();
        Error_Handler("Failed to start motor output");
    }

    return FC_OK;
}


void vControlLoopTask(void *pvParameters)
{
    FC_Status status;
    tPpmSignal ppmSignal = {0};

    DEBUG_PRINT("Control loop start\n");
    controlLoopInit();

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

    bool armed = false;
    uint32_t rcThrottle = 1000;

    Rates_t actualRates;
    Rates_t desiredRates;
    RotationAxisOutputs_t *rotationOutputsPtr;

    DEBUG_PRINT("Starting control loop\n");
    for ( ;; )
    {
        if (xQueueReceive(ppmSignalQueue, &ppmSignal, 0) == pdTRUE) {
            rcThrottle = ppmSignal.signals[THROTTLE_CHANNEL];
            rcThrottle = limit(rcThrottle, MOTOR_LOW_VAL_US, MOTOR_HIGH_VAL_US);

            // Check if we are still armed
            // only disarm if throttle is low, so don't accidentally disarm in
            // flight
            if (ppmSignal.signals[ARMED_SWITCH_CHANNEL] >= SWITCH_HIGH_THRESHOLD) {
                armed = true;
            } else if (ppmSignal.signals[ARMED_SWITCH_CHANNEL] <= SWITCH_LOW_THRESHOLD
                && rcThrottle <= THROTTLE_LOW_THRESHOLD ) {
                armed = false;
            }


            /*DEBUG_PRINT("rin: %d, pin: %d, yin: %d\n", ppmSignal.signals[ROLL_CHANNEL],*/
                        /*ppmSignal.signals[PITCH_CHANNEL],ppmSignal.signals[YAW_CHANNEL]);*/
            desiredRates.roll = limit(map(ppmSignal.signals[ROLL_CHANNEL],
                                           MIN_RC_VAL, MAX_RC_VAL,
                                           ROTATION_AXIS_OUTPUT_MIN,
                                           ROTATION_AXIS_OUTPUT_MAX),
                                       ROTATION_AXIS_OUTPUT_MIN,
                                       ROTATION_AXIS_OUTPUT_MAX);
            desiredRates.pitch= limit(map(ppmSignal.signals[PITCH_CHANNEL],
                                           MIN_RC_VAL, MAX_RC_VAL,
                                           ROTATION_AXIS_OUTPUT_MIN,
                                           ROTATION_AXIS_OUTPUT_MAX),
                                       ROTATION_AXIS_OUTPUT_MIN,
                                       ROTATION_AXIS_OUTPUT_MAX);
            desiredRates.yaw= limit(map(ppmSignal.signals[YAW_CHANNEL],
                                           MIN_RC_VAL, MAX_RC_VAL,
                                           ROTATION_AXIS_OUTPUT_MIN,
                                           ROTATION_AXIS_OUTPUT_MAX),
                                       ROTATION_AXIS_OUTPUT_MIN,
                                       ROTATION_AXIS_OUTPUT_MAX);
            /*DEBUG_PRINT("rd: %d, pd: %d, yd: %d\n", desiredRates.roll,*/
                        /*desiredRates.pitch, desiredRates.yaw);*/
        }


        if (armed && rcThrottle >= THROTTLE_LOW_THRESHOLD) {
            if (xQueueReceive(ratesQueue, &actualRates, 0) == pdTRUE) {
                /*DEBUG_PRINT("ra: %d, pa: %d, ya: %d\n", actualRates.roll,*/
                            /*actualRates.pitch, actualRates.yaw);*/

                rotationOutputsPtr = controlRates(&actualRates, &desiredRates);

                /*DEBUG_PRINT("ro: %d, po: %d, yo: %d\n", rotationOutputsPtr->roll,*/
                            /*rotationOutputsPtr->pitch, rotationOutputsPtr->yaw);*/

                setMotor(MOTOR_FRONT_LEFT,
                         rcThrottle + rotationOutputsPtr->roll
                         - rotationOutputsPtr->pitch
                         - rotationOutputsPtr->yaw);
                setMotor(MOTOR_BACK_LEFT,
                         rcThrottle + rotationOutputsPtr->roll
                         + rotationOutputsPtr->pitch
                         + rotationOutputsPtr->yaw);
                setMotor(MOTOR_FRONT_RIGHT,
                         rcThrottle - rotationOutputsPtr->roll
                         - rotationOutputsPtr->pitch
                         + rotationOutputsPtr->yaw);
                setMotor(MOTOR_BACK_RIGHT,
                         rcThrottle - rotationOutputsPtr->roll
                         + rotationOutputsPtr->pitch
                         - rotationOutputsPtr->yaw);
            }
        } else {
            resetRateInfo(); // reset integral terms while on ground
            motorsStop();
        }

        vTaskDelay(5);
    }
}
