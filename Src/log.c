#include <string.h>
#include <stdbool.h>

#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "ff.h"

#include "log.h"
#include "debug.h"

#define LOG_QUEUE_LENGTH 10
#define LOG_SYNC_PERIOD_MS 100

FATFS FatFS;
FIL fil;
QueueHandle_t logQueue = NULL;

FC_Status logInit()
{
    logQueue = xQueueCreate(sizeof(tLogMessage), LOG_QUEUE_LENGTH);

    if (logQueue == NULL)
    {
        Error_Handler("Failed to create log queue\n");
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status logStart()
{
    FRESULT fr;    /* FatFs return code */

    /* Register work area to the default drive */
    fr = f_mount(&FatFS, "", 0);
    if (fr) {
        DEBUG_PRINT("mount failed: %d\n", (int)fr);
        return FC_ERROR;
    } else {
        DEBUG_PRINT("mounted drive\n");
    }

    /* Open a text file */
    fr = f_open(&fil, "test.txt", FA_READ|FA_WRITE|FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        DEBUG_PRINT("F_Open failed: %d\n", (int)fr);
        return FC_ERROR;
    } else {
        DEBUG_PRINT("Opened file\n");
    }

    return FC_ERROR;
}

FC_Status logFinish()
{
    FRESULT fr;    /* FatFs return code */

    /* Close the file */
    fr = f_close(&fil);
    if (fr != FR_OK) {
        DEBUG_PRINT("Failed to close file: %d\n", (int)fr);
        return FC_ERROR;
    } else {
        DEBUG_PRINT("Closed file\n");
    }
    return FC_OK;
}

FC_Status logWriteTest()
{
    FRESULT fr;

    if (f_error(&fil) != 0)
    {
        DEBUG_PRINT("File error %d\n", f_error(&fil));
        return FC_ERROR;
    }

    DEBUG_PRINT("Writing to file\n");
    int written = f_printf(&fil, "Hello to file\n");

    if (written < 0)
    {
        DEBUG_PRINT("Error writing to file %d\n", written);
        return FC_ERROR;
    } else {
        DEBUG_PRINT("Wrote %d bytes to file\n", written);
    }

    fr = f_sync(&fil);
    if (fr != FR_OK) {
        DEBUG_PRINT("Failed to sync file: %d\n", (int)fr);
        return FC_ERROR;
    } else {
        DEBUG_PRINT("Sync file\n");
    }

    return FC_OK;
}

FC_Status sendLogDataToQueue(Rates_t *desiredRates, Rates_t *actualRates,
                             RotationAxisOutputs_t *rotationOutputsPtr,
                             PidAllAxis_t *PIDs)
{
    tLogMessage logMsg;

    memcpy(&(logMsg.desiredRates), desiredRates, sizeof(Rates_t));
    memcpy(&(logMsg.actualRates), actualRates, sizeof(Rates_t));
    memcpy(&(logMsg.motorOutputs), rotationOutputsPtr, sizeof(RotationAxisOutputs_t));
    memcpy(&(logMsg.PIDs), PIDs, sizeof(PidAllAxis_t));
    logMsg.timestamp_ms = HAL_GetTick();

    if (xQueueSend(logQueue, &logMsg, 0) != pdTRUE)
    {
        DEBUG_PRINT("Failed send log data to queue\n");
        return FC_ERROR;
    }

    return FC_OK;
}

FC_Status logDataStruct(tLogMessage *msg)
{
    // Log format in csv
    // desiredRates (roll,pitch,yaw)
    if (f_printf(&fil, "%d,%d,%d,", msg->desiredRates.roll, msg->desiredRates.pitch, msg->desiredRates.yaw) < 0)
    {
        return FC_ERROR;
    }
    // actualRates (roll,pitch,yaw)
    if (f_printf(&fil, "%d,%d,%d,", msg->actualRates.roll, msg->actualRates.pitch, msg->actualRates.yaw) < 0)
    {
        return FC_ERROR;
    }
    // motor outputs (roll,pitch,yaw)
    if (f_printf(&fil, "%d,%d,%d,", msg->motorOutputs.roll, msg->motorOutputs.pitch, msg->motorOutputs.yaw) < 0)
    {
        return FC_ERROR;
    }
    // pid roll (p,i,d)
    if (f_printf(&fil, "%d,%d,%d,", msg->PIDs.roll.p, msg->PIDs.roll.i, msg->PIDs.roll.d) < 0)
    {
        return FC_ERROR;
    }
    // pid pitch (p,i,d)
    if (f_printf(&fil, "%d,%d,%d,", msg->PIDs.pitch.p, msg->PIDs.pitch.i, msg->PIDs.pitch.d) < 0)
    {
        return FC_ERROR;
    }
    // pid yaw (p,i,d)
    if (f_printf(&fil, "%d,%d,%d,", msg->PIDs.yaw.p, msg->PIDs.yaw.i, msg->PIDs.yaw.d) < 0)
    {
        return FC_ERROR;
    }
    // timestamp milliseconds
    if (f_printf(&fil, "%lu\n", msg->timestamp_ms) < 0)
    {
        return FC_ERROR;
    }

    return FC_OK;
}

void vLogTask(void *pvParameters)
{
    tLogMessage msg;
    uint32_t    lastWriteMs = 0;
    bool        logBuffered = false;

    DEBUG_PRINT("Init log\n");
    logStart();
    DEBUG_PRINT("Initialized log\n");

    for ( ;; )
    {
        if (xQueueReceive(logQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            if (logDataStruct(&msg) != FC_OK)
            {
                DEBUG_PRINT("Failed to log data struct\n");
            } else {
                logBuffered = true;
            }
        }

        if (logBuffered && (HAL_GetTick() - lastWriteMs > LOG_SYNC_PERIOD_MS))
        {
            // periodically sync the file, don't need to do this too often since
            // write period is enough to fill up and flush file buffer often
            f_sync(&fil);
            logBuffered = false;
            lastWriteMs = HAL_GetTick();
        }
    }

    logFinish();
}
