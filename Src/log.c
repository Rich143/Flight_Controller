#include <string.h>
#include <stdbool.h>

#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "ff.h"

#include "log.h"
#include "debug.h"

#define LOG_QUEUE_LENGTH 20
#define LOG_PERIOD_TICKS 100
#define LOG_SYNC_PERIOD_TICKS LOG_PERIOD_TICKS * 10
#define LOG_NUMBER_LEN_BYTES 1
#define LOG_FILE_NAME_LEN_BYTES 15

FATFS FatFS;
FIL fil;
QueueHandle_t logQueue = NULL;

const char logNumFileName[] = "lognum.txt";

uint32_t lastLogTime = 0;

FC_Status logInit()
{
    printf("Creating queue size %u\n", sizeof(tLogMessage) * LOG_QUEUE_LENGTH);
    logQueue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(tLogMessage));

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
    FILINFO fno;  // File info

    /* Register work area to the default drive */
    fr = f_mount(&FatFS, "", 0);
    if (fr) {
        DEBUG_PRINT("mount failed: %d\n", (int)fr);
        return FC_ERROR;
    } else {
        DEBUG_PRINT("mounted drive\n");
    }

    //
    // Open a file to get the current log file number, then write back the next
    // number to use
    //
    fr = f_stat(logNumFileName, &fno);

    FIL logNumFile;
    uint8_t logFileNum = 0;
    if (fr == FR_OK && fno.fsize > 0) {
        fr = f_open(&logNumFile, logNumFileName, FA_READ|FA_WRITE|FA_OPEN_EXISTING);
        if (fr != FR_OK) {
            DEBUG_PRINT("F_Open failed: %d\n", (int)fr);
            return FC_ERROR;
        }

        UINT bytesRead;
        fr = f_read(&logNumFile, &logFileNum, LOG_NUMBER_LEN_BYTES, &bytesRead);

        if (fr != FR_OK || bytesRead != LOG_NUMBER_LEN_BYTES) {
            DEBUG_PRINT("Failed to get log file number\n");
            return FC_ERROR;
        }
    } else if (fr == FR_NO_FILE) {
        fr = f_open(&logNumFile, logNumFileName, FA_WRITE|FA_CREATE_NEW);
        if (fr != FR_OK) {
            DEBUG_PRINT("F_Open failed: %d\n", (int)fr);
            return FC_ERROR;
        }
    } else {
        DEBUG_PRINT("Error, invalid logFileNames file\n");
        DEBUG_PRINT("FR %d, fno.fsize %lu\n", fr, fno.fsize);
        return FC_ERROR;
    }

    char logFileName[LOG_FILE_NAME_LEN_BYTES];
    snprintf(logFileName, LOG_FILE_NAME_LEN_BYTES, "run_%d.csv", logFileNum);

    logFileNum++;

    fr = f_lseek(&logNumFile, 0); // move to beginning of file
    if (fr != FR_OK) {
        DEBUG_PRINT("Failed to seek to start of file\n");
        return FC_ERROR;
    }

    UINT bytesWritten;
    fr = f_write(&logNumFile, &logFileNum, LOG_NUMBER_LEN_BYTES, &bytesWritten);
    if (fr != FR_OK)
    {
        DEBUG_PRINT("Failed to write log file num to file\n");
        return FC_ERROR;
    }

    fr = f_close(&logNumFile);
    if (fr != FR_OK)
    {
        DEBUG_PRINT("Failed to close file\n");
        return FC_ERROR;
    }

    /* Open the log file */
    fr = f_open(&fil, logFileName, FA_READ|FA_WRITE|FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        DEBUG_PRINT("F_Open failed: %d\n", (int)fr);
        return FC_ERROR;
    } else {
        DEBUG_PRINT("Opened log file %s\n", logFileName);
    }

    return FC_OK;
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

FC_Status sendLogDataToQueue(Attitude_t *desiredAttitude,
                             Attitude_t *actualAttitude,
                             Rates_t *desiredRates, Rates_t *actualRates,
                             RotationAxisOutputs_t *rotationOutputsPtr,
                             PidAllAxis_t *PIDs_RateControl,
                             PidAllAxis_t *PIDs_AttitudeControl,
                             FlightMode_t flightMode)
{
    tLogMessage logMsg;

    if (HAL_GetTick() - lastLogTime > LOG_PERIOD_TICKS) {
        lastLogTime = HAL_GetTick();

        memcpy(&(logMsg.desiredAttitude), desiredAttitude, sizeof(Attitude_t));
        memcpy(&(logMsg.actualAttitude), actualAttitude, sizeof(Attitude_t));
        memcpy(&(logMsg.desiredRates), desiredRates, sizeof(Rates_t));
        memcpy(&(logMsg.actualRates), actualRates, sizeof(Rates_t));
        memcpy(&(logMsg.motorOutputs), rotationOutputsPtr, sizeof(RotationAxisOutputs_t));
        memcpy(&(logMsg.PIDs), PIDs_RateControl, sizeof(PidAllAxis_t));
        memcpy(&(logMsg.PIDs), PIDs_AttitudeControl, sizeof(PidAllAxis_t));
        logMsg.flightMode = flightMode;
        logMsg.timestamp_ms = HAL_GetTick();

        if (xQueueSend(logQueue, &logMsg, 0) != pdTRUE)
        {
            DEBUG_PRINT("[%lu] Failed send log data to queue\n", HAL_GetTick());
            return FC_ERROR;
        }
    }

    return FC_OK;
}

FC_Status logDataStruct(tLogMessage *msg)
{
    int written;
    int totalBytesWritten = 0;

    // Log format in csv
    // desiredAttitude (roll,pitch,yaw)
    written = f_printf(&fil, "%d,%d,%d,", msg->desiredAttitude.roll, msg->desiredAttitude.pitch, msg->desiredAttitude.yaw);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->desiredAttitude.roll, msg->desiredAttitude.pitch, msg->desiredAttitude.yaw);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // actualAttitude (roll,pitch,yaw)
    written = f_printf(&fil, "%d,%d,%d,", msg->actualAttitude.roll, msg->actualAttitude.pitch, msg->actualAttitude.yaw);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->actualAttitude.roll, msg->actualAttitude.pitch, msg->actualAttitude.yaw);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // desiredRates (roll,pitch,yaw)
    written = f_printf(&fil, "%d,%d,%d,", msg->desiredRates.roll, msg->desiredRates.pitch, msg->desiredRates.yaw);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->desiredRates.roll, msg->desiredRates.pitch, msg->desiredRates.yaw);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // actualRates (roll,pitch,yaw)
    written = f_printf(&fil, "%d,%d,%d,", msg->actualRates.roll, msg->actualRates.pitch, msg->actualRates.yaw);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->actualRates.roll, msg->actualRates.pitch, msg->actualRates.yaw);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // motor outputs (roll,pitch,yaw)
    written = f_printf(&fil, "%d,%d,%d,", msg->motorOutputs.roll, msg->motorOutputs.pitch, msg->motorOutputs.yaw);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->motorOutputs.roll, msg->motorOutputs.pitch, msg->motorOutputs.yaw);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    /*
     * Rate PIDs
     */
    // pid roll (p,i,d)
    written = f_printf(&fil, "%d,%d,%d,", msg->PIDs.roll.p, msg->PIDs.roll.i, msg->PIDs.roll.d);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->PIDs.roll.p, msg->PIDs.roll.i, msg->PIDs.roll.d);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // pid pitch (p,i,d)
    written = f_printf(&fil, "%d,%d,%d,", msg->PIDs.pitch.p, msg->PIDs.pitch.i, msg->PIDs.pitch.d);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->PIDs.pitch.p, msg->PIDs.pitch.i, msg->PIDs.pitch.d);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // pid yaw (p,i,d)
    written = f_printf(&fil, "%d,%d,%d,", msg->PIDs.yaw.p, msg->PIDs.yaw.i, msg->PIDs.yaw.d);
    /*DEBUG_PRINT("%d,%d,%d | ", msg->PIDs.yaw.p, msg->PIDs.yaw.i, msg->PIDs.yaw.d);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    /*
     * Attitude PIDs
     * only P is used for now
     */
    // pid roll (p)
    written = f_printf(&fil, "%d,", msg->PIDs.roll.p);
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // pid pitch (p)
    written = f_printf(&fil, "%d,", msg->PIDs.pitch.p);
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // pid yaw (p)
    written = f_printf(&fil, "%d,", msg->PIDs.yaw.p);
    if (written < 0)
    {
        return FC_ERROR;
    }

    // flight mode
    written = f_printf(&fil, "%d,", msg->flightMode);
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    // timestamp milliseconds
    written = f_printf(&fil, "%lu\n", msg->timestamp_ms);
    /*DEBUG_PRINT("%lu\n", msg->timestamp_ms);*/
    if (written < 0)
    {
        return FC_ERROR;
    }
    totalBytesWritten += written;

    /*DEBUG_PRINT("Wrote %d bytes to file\n", totalBytesWritten);*/
    return FC_OK;
}

void vLogTask(void *pvParameters)
{
    tLogMessage msg;
    uint32_t    lastWriteMs = 0;
    bool        logBuffered = false;

    DEBUG_PRINT("Init log\n");
    if (logStart() != FC_OK) {
        DEBUG_PRINT("Failed to start logging\n");
        LED3_ON
        vTaskSuspend(NULL);
    }
    DEBUG_PRINT("Initialized log\n");

    for ( ;; )
    {
        memset(&msg, 0, sizeof(tLogMessage));

        if (xQueueReceive(logQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            /*uint32_t startTime = portGET_RUN_TIME_COUNTER_VALUE();*/
            if (logDataStruct(&msg) != FC_OK)
            {
                DEBUG_PRINT("Failed to log data struct\n");
            } else {
                logBuffered = true;
            }
            /*DEBUG_PRINT("[%lu] Took %lu to log\n", HAL_GetTick(), portGET_RUN_TIME_COUNTER_VALUE() - startTime);*/
        }

        if (logBuffered && (HAL_GetTick() - lastWriteMs > LOG_SYNC_PERIOD_TICKS))
        {
            // periodically sync the file, don't need to do this too often since
            // write period is enough to fill up and flush file buffer often
            /*DEBUG_PRINT("Sync file\n");*/
            f_sync(&fil);
            logBuffered = false;
            lastWriteMs = HAL_GetTick();
        }
    }

    logFinish();
}
