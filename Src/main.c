/* Waterloo Formula Electric 2017 */
#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdint.h>
#include "stdio.h"

#include "freertos.h"
#include "task.h"

#include "main.h"
#include "hardware.h"
#include "debug.h"
#include "pressureSensor.h"

void vPrintTask1( void *pvParameters )
{
    for( ;; )
    {
        /*printf("Test String\n");*/
        DEBUG_PRINT("Test String1\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void vPrintTask2( void *pvParameters )
{
    for( ;; )
    {
        /*printf("Test String\n");*/
        DEBUG_PRINT("Test String2\n");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void vBlinkTask( void *pvParameters )
{
    LED3_ON
    for( ;; )
    {
        HAL_GPIO_TogglePin(LED_PORT, LED3_PIN);
        /*HAL_GPIO_TogglePin(LED_PORT, LED4_PIN);*/
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    /* Tasks must not attempt to return from their implementing
    function or otherwise exit.  In newer FreeRTOS port
    attempting to do so will result in an configASSERT() being
    called if it is defined.  If it is necessary for a task to
    exit then have the task call vTaskDelete( NULL ) to ensure
    its exit is clean. */
    vTaskDelete( NULL );
}

int32_t setup(void){
    // System Clock config
    Clock_Config();

    // Reset of all peripherals, Initializes teh Flash interface and the Systick
    HAL_Init();

    // System interrupt init
    HAL_MspInit();

    hardware_init();

    return 0;
}

int main(void)
{
    setup();

    printf("Started up. Switching to HSE_Clock\n");
    ClockHSE_Config();
    printf("Switched to HSE Clock\n");

    xTaskCreate(vBlinkTask, "blinkTask", 100, NULL, 3 /* priority */, NULL);
    xTaskCreate(vPrintTask1, "printTask1", 300, NULL, 2 /* priority */, NULL);
    xTaskCreate(vPrintTask2, "printTask2", 300, NULL, 2 /* priority */, NULL);
    xTaskCreate(vDebugTask, "debugTask", 300, NULL, 1 /* priority */, NULL);
    xTaskCreate(vPressureSensorTask, "pressureSensorTask", 300, NULL, 3 /* priority */, NULL);

    vTaskStartScheduler();

    return 0;
}
