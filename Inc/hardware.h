#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "freertos.h"
#include "semphr.h"
#include "pins.h"

#define I2C_MUT_WAIT_TICKS 10 // wait 10 ticks for the I2C bus to become available

typedef enum RGB_Colour {
    RGB_RED   = 1,
    RGB_BLUE  = 2,
    RGB_GREEN = 3,
} RGB_Colour;

void Clock_Config();
void ClockHSE_Config();
void hardware_init();
void rgbSetColour(RGB_Colour colour);

extern I2C_HandleTypeDef I2cHandle;
extern SemaphoreHandle_t I2CMutex;

#endif /* HARDWARE_H_ */
