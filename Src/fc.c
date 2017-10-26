#include "fc.h"
#ifndef __UNIT_TEST
#include "pins.h"
#include "debug.h"
#include "task.h"
#endif

int limit(int val, int min, int max)
{
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

int map(int x, int inMin, int inMax, int outMin, int outMax)
{
    // Use floating point values here to improve distribution of inputs to
    // outputs
    float x_f = (float)x;
    float inMin_f = (float)inMin;
    float inMax_f = (float)inMax;
    float outMin_f = (float)outMin;
    float outMax_f = (float)outMax;

    return (int)((x_f - inMin_f) * (outMax_f - outMin_f)
                 / (inMax_f - inMin_f) + outMin_f);
}

#ifndef __UNIT_TEST
void Error_Handler(char *msg)
{
    // Disable interrupts. This ensures we don't get interrupted while
    // printing the error message, and that we can block all tasks
    taskDISABLE_INTERRUPTS();
    LED3_ON
    /*printf("Err:%s\n", msg == NULL ? "" : msg);*/ // This doesn't work if uart already in use
    while (1);
}

void assertFailed(char *file, int line)
{
    char buf[PRINT_QUEUE_STRING_SIZE];
    snprintf(buf, PRINT_QUEUE_STRING_SIZE, "ASSERT:%s:%d", file, line);
    Error_Handler(buf);
}

#else

#include <stdio.h>

void Error_Handler(char *msg)
{
    printf("Err:%s\n", msg == NULL ? "" : msg);
    while(1);
}

#endif

