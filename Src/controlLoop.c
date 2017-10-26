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

FC_Status processPpmSignal(tPpmSignal *ppmSignal, Rates_t *desiredRatesOut,
                           uint32_t *rcThrottleOut, bool *armedOut)
{
    uint32_t rcThrottle = ppmSignal->signals[THROTTLE_CHANNEL];
    rcThrottle = limit(rcThrottle, MOTOR_LOW_VAL_US, MOTOR_HIGH_VAL_US);
    (*rcThrottleOut) = rcThrottle;

    // Check if we are still armed
    // only disarm if throttle is low, so don't accidentally disarm in
    // flight
    if (ppmSignal->signals[ARMED_SWITCH_CHANNEL] >= SWITCH_HIGH_THRESHOLD) {
        (*armed) = true;
    } else if (ppmSignal->signals[ARMED_SWITCH_CHANNEL] <= SWITCH_LOW_THRESHOLD
               && rcThrottle <= THROTTLE_LOW_THRESHOLD ) {
        (*armed) = false;
    }


    /*DEBUG_PRINT("rin: %d, pin: %d, yin: %d\n", ppmSignal->signals[ROLL_CHANNEL],*/
    /*ppmSignal->signals[PITCH_CHANNEL],ppmSignal->signals[YAW_CHANNEL]);*/
    desiredRates->roll = limit(map(ppmSignal->signals[ROLL_CHANNEL],
                                  MIN_RC_VAL, MAX_RC_VAL,
                                  ROTATION_AXIS_OUTPUT_MIN,
                                  ROTATION_AXIS_OUTPUT_MAX),
                              ROTATION_AXIS_OUTPUT_MIN,
                              ROTATION_AXIS_OUTPUT_MAX);
    desiredRates->pitch= limit(map(ppmSignal->signals[PITCH_CHANNEL],
                                  MIN_RC_VAL, MAX_RC_VAL,
                                  ROTATION_AXIS_OUTPUT_MIN,
                                  ROTATION_AXIS_OUTPUT_MAX),
                              ROTATION_AXIS_OUTPUT_MIN,
                              ROTATION_AXIS_OUTPUT_MAX);
    desiredRates->yaw= limit(map(ppmSignal->signals[YAW_CHANNEL],
                                MIN_RC_VAL, MAX_RC_VAL,
                                ROTATION_AXIS_OUTPUT_MIN,
                                ROTATION_AXIS_OUTPUT_MAX),
                            ROTATION_AXIS_OUTPUT_MIN,
                            ROTATION_AXIS_OUTPUT_MAX);
    /*DEBUG_PRINT("rd: %d, pd: %d, yd: %d\n", desiredRates->roll,*/
    /*desiredRates->pitch, desiredRates->yaw);*/

    return FC_OK;
}

void updateMotors(uint32_t rcThrottle, RotationAxisOutputs_t *outputs)
{
    // Note that motor value limiting is done in the setMotor function
    setMotor(MOTOR_FRONT_LEFT,
             rcThrottle + outputs->roll
             - outputs->pitch
             - outputs->yaw);
    setMotor(MOTOR_BACK_LEFT,
             rcThrottle + outputs->roll
             + outputs->pitch
             + outputs->yaw);
    setMotor(MOTOR_FRONT_RIGHT,
             rcThrottle - outputs->roll
             - outputs->pitch
             + outputs->yaw);
    setMotor(MOTOR_BACK_RIGHT,
             rcThrottle - outputs->roll
             + outputs->pitch
             - outputs->yaw);
}

FC_Status checkControlLoopStatus(TickType_t lastPpmRxTime,
                                 TickType_t lastGyroRxTime,
                                 TickType_t lastLoopTime)
{
    FC_Status systemStatus = FC_OK;
    TickType_t curTick = xTaskGetTickCount();

    if (curTick - lastPpmRxTime > PPM_RX_TIMEOUT_MS)
    {
        DEBUG_PRINT("PPM timeout cur %d last %d\n", curTick, lastPpmRxTime);
        systemStatus = FC_ERROR;
    }
    if (curTick - lastGyroRxTime > GYRO_RX_TIMEOUT_MS)
    {
        DEBUG_PRINT("Gyro timeout cur %d last %d\n", curTick, lastGyroRxTime);
        systemStatus = FC_ERROR;
    }
    if (curTick - lastLoopTime > CONTROL_LOOP_TIMEOUT_MS)
    {
        DEBUG_PRINT("CtrlLp timeout cur %d last %d\n", curTick, lastLoopTime);
        systemStatus = FC_ERROR;
    }

    if (systemStatus != FC_OK)
    {
        // System failure, stop the motors and reset
        motorsStop();
        NVIC_SystemReset();
    }
    else
    {
        // TODO: add wdt and kick it here
    }

    return systemStatus;
}

void vControlLoopTask(void *pvParameters)
{
    tPpmSignal ppmSignal = {0};

    bool armed = false;
    uint32_t rcThrottle = 1000;

    Rates_t actualRates;
    Rates_t desiredRates;
    RotationAxisOutputs_t *rotationOutputsPtr;

    DEBUG_PRINT("Control loop start\n");
    controlLoopInit();

    DEBUG_PRINT("Waiting for low throttle\n");
    // Wait for throttle to be low before continuing startup
    // This is for safety
    while (1) {
        if (xQueueReceive(ppmSignalQueue, &ppmSignal, 100) == pdTRUE) {
            rcThrottle = ppmSignal.signals[THROTTLE_CHANNEL];
            if (rcThrottle <= THROTTLE_LOW_THRESHOLD) {
                DEBUG_PRINT("Starting, thr %d\n", ppmSignal.signals[THROTTLE_CHANNEL]);
                break;
            }
        }
        vTaskDelay(PPM_FRAME_PERIOD_MS/portTICK_PERIOD_MS);
    }


    DEBUG_PRINT("Starting control loop\n");

    TickType_t lastPpmRxTime  = xTaskGetTickCount();
    TickType_t lastLoopTime   = xTaskGetTickCount();
    TickType_t lastGyroRxTime = xTaskGetTickCount();
    // This is seperate from lastLoopTime because vTaskDelayUntil sets this
    // to the last wake time + increment, even if there is some unforseen delay
    // that causes the task to miss its deadline
    TickType_t lastWakeTime   = xTaskGetTickCount();

    for ( ;; )
    {
        if (xQueueReceive(ppmSignalQueue, &ppmSignal, 0) == pdTRUE) {
            lastPpmRxTime = xTaskGetTickCount();

            if (processPpmSignal(&ppmSignal, &desiredRates,
                                 &rcThrottleOut, &armed) != FC_OK)
            {
                DEBUG_PRINT("Failed to process ppm signal\n");
            }
        }


        if (armed && rcThrottle >= THROTTLE_LOW_THRESHOLD) {
            if (xQueueReceive(ratesQueue, &actualRates, 0) == pdTRUE) {
                lastGyroRxTime = xTaskGetTickCount();

                /*DEBUG_PRINT("ra: %d, pa: %d, ya: %d\n", actualRates.roll,*/
                            /*actualRates.pitch, actualRates.yaw);*/

                rotationOutputsPtr = controlRates(&actualRates, &desiredRates);

                /*DEBUG_PRINT("ro: %d, po: %d, yo: %d\n", rotationOutputsPtr->roll,*/
                            /*rotationOutputsPtr->pitch, rotationOutputsPtr->yaw);*/

                updateMotors(rcThrottle, rotationOutputsPtr);
            } else {
                DEBUG_PRINT("Failed to receive gyro data\n");
            }
        } else {
            resetRateInfo(); // reset integral terms while on ground
            motorsStop();
        }

        checkControlLoopStatus(lastPpmRxTime, lastGyroRxTime, lastLoopTime);
        lastLooptime = xTaskGetTickCount();

        vTaskDelayUntil(&lastWakeTime, CONTROL_LOOP_PERIOD_TICKS);
    }
}
