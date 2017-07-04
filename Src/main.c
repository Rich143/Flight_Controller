/* Waterloo Formula Electric 2017 */
#include <stm32f0xx.h>
#include <main.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "debug.h"
#include "can.h"
#include "can_heartbeat.h"
#include "debounce.h"
#include "hardware.h"
#include "pins_common.h"

#include "fake_logic.h"

char PS[] = "BLANKPROJECT";

#define MAIN_LOOP_PERIOD_MS (5)

void executeSerialCommand(char str[]) {
    if (strcmp(str, "marco") == 0) {
        printf("polo\n");
    } else if (strncmp(str, "add", 3) == 0) {
        int a, b;
        sscanf(str, "add %d %d", &a, &b);
        printf("sum is %d\n", add(a, b));
    }
}

void vBlinkTask( void *pvParameters )
{
    for( ;; )
    {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);

        // 100ms period
        vTaskDelay(100 / portTICK_PERIOD_MS);
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
    SystemClock_Config();

    // Reset of all peripherals, Initializes teh Flash interface and the Systick
    HAL_Init();

    // System interrupt init
    HAL_MspInit();

    // Initialize peripherals GPIO
    MX_GPIO_Init();

    // Initialize UART for printf
    debug_init();

    return 0;
}

// challenge: without changing the source file and only using the debugger, change the
// code to print "Hello from PB"
char str[] = "Helxo from PB\n";

int main(void)
{
    setup();
    xTaskCreate(vBlinkTask, "blinkTask", 200, NULL, 1 /* [> priority <] */, NULL);

    printf(str);
    vTaskStartScheduler();
    return 0;
}
