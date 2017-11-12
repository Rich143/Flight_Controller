#include "fc.h"

#include "freertos.h"
#include "task.h"

#include "ff.h"

#include "log.h"
#include "debug.h"

FATFS FatFS;
FIL fil;

FC_Status logInit()
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

void vLogTask(void *pvParameters)
{
   DEBUG_PRINT("Init log\n");
   logInit();

   for ( ;; )
   {
       logWriteTest();
       vTaskDelay(1000/ portTICK_PERIOD_MS);
   }

   logFinish();
}
