#include "fc.h"
#include "pins.h"
#include "debug.h"
#include "task.h"

void Error_Handler(void)
{
    LED4_ON
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        DEBUG_PRINT("Error occured\n");
    } else {
        printf("Error occured\n");
    }
    while (1);
}
