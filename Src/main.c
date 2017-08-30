/* Waterloo Formula Electric 2017 */
#include <stm32f4xx.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos.h"
#include "task.h"

#include "main.h"
#include "hardware.h"

void vBlinkTask( void *pvParameters )
{
    for( ;; )
    {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
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

    xTaskCreate(vBlinkTask, "blinkTask", 200, NULL, 1 /* [> priority <] */, NULL);

    vTaskStartScheduler();

    return 0;
}
