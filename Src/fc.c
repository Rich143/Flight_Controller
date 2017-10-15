#ifndef __UNIT_TEST
#include "fc.h"
#include "pins.h"
#include "debug.h"
#include "task.h"

void Error_Handler(char *msg)
{
    // Disable interrupts. This ensures we don't get interrupted while
    // printing the error message, and that we can block all tasks
    taskDISABLE_INTERRUPTS();
    LED1_ON
    printf("Err:%s\n", msg == NULL ? "" : msg);
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

