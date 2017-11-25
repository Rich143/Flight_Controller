#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdint.h>
#include "stdio.h"

#include "freertos.h"
#include "task.h"

#include "fc.h"
#include "main.h"
#include "hardware.h"
#include "debug.h"
#include "imu.h"
#include "pressureSensor.h"
#include "ppm.h"
#include "motors.h"
#include "controlLoop.h"
#include "log.h"

void vPrintTask1( void *pvParameters )
{
    for( ;; )
    {
        DEBUG_PRINT("Test String1\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void vPrintTask2( void *pvParameters )
{
    for( ;; )
    {
        DEBUG_PRINT("Test String2\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void vStatsTask( void *pvParameter)
{
    /*statsTimerInit();*/

    for ( ;; )
    {
        /*runTimeStatsGetTimerVal();*/
        // According to docs, ~40 bytes * num tasks is enough for the buffer
        unsigned int statsBufLen = 45 * uxTaskGetNumberOfTasks();
        char statsBuf[statsBufLen];

        vTaskGetRunTimeStats(statsBuf);
        writeDebugMessage(statsBuf);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


void vBlinkTask( void *pvParameters )
{
    for( ;; )
    {
        rgbSetColour(RGB_GREEN);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        rgbSetColour(RGB_BLUE);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        rgbSetColour(RGB_RED);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    /* Tasks must not attempt to return from their implementing
    function or otherwise exit.  In newer FreeRTOS port
    attempting to do so will result in an configASSERT() being
    called if it is defined.  If it is necessary for a task to
    exit then have the task call vTaskDelete( NULL ) to ensure
    its exit is clean. */
    vTaskDelete( NULL );
}

void setup(void){
    // System Clock config
    ClockHSE_Config();

    // Reset of all peripherals, Initializes teh Flash interface and the Systick
    HAL_Init();

    // System interrupt init
    HAL_MspInit();

    hardware_init();

    HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 ); // see http://www.freertos.org/RTOS-Cortex-M3-M4.html

    printf("Free heap size after setup: %d\n", xPortGetFreeHeapSize());

    return;
}

int main(void)
{
    setup();
    printf("System start up. Hardware initialized.\n");

    xTaskCreate(vBlinkTask, "blinkTask", 50, NULL, 2 /* priority */, NULL);
    /*xTaskCreate(vPrintTask1, "printTask1", 300, NULL, 2 [> priority <], NULL);*/
    /*xTaskCreate(vPrintTask2, "printTask2", 300, NULL, 2 [> priority <], NULL);*/
    xTaskCreate(vDebugTask, "debugTask", 300, NULL, 1 /* priority */, NULL);
    /*xTaskCreate(vPressureSensorTask, "pressureSensorTask", 300, NULL, 3 [> priority <], NULL);*/
    /*xTaskCreate(vIMUTask, "IMUTask", 300, NULL, 4 [> priority <], NULL);*/
    /*xTaskCreate(vRCTask, "RCTask", 200, NULL, 4 [> priority <], NULL);*/
    /*xTaskCreate(vControlLoopTask, "ControlLoopTask", 400, NULL, 3 [> priority <], NULL);*/
    xTaskCreate(vLogTask, "LogTask", 800, NULL, 3 /* priority */, NULL);
    xTaskCreate(vStatsTask, "StatsTask", 300, NULL, 1 /* priority */, NULL);

    vTaskStartScheduler();

    return 0;
}
