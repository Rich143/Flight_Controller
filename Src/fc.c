#include "fc.h"
#include "pins.h"
#include "debug.h"

void Error_Handler(void)
{
    LED4_ON
    DEBUG_PRINT("Error occured\n");
    while (1);
}
