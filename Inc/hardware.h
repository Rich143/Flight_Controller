#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "freertos.h"
#include "semphr.h"
#include "pins.h"

#define I2C_MUT_WAIT_TICKS 10 // wait 10 ticks for the I2C bus to become available

/* Definition for I2Cx's DMA NVIC */
#define I2Cx_DMA_TX_IRQn                DMA1_Stream6_IRQn
#define I2Cx_DMA_RX_IRQn                DMA1_Stream5_IRQn
#define I2Cx_DMA_TX_IRQHandler          DMA1_Stream6_IRQHandler
#define I2Cx_DMA_RX_IRQHandler          DMA1_Stream5_IRQHandler

/* Definition for I2Cx's NVIC */
#define I2Cx_EV_IRQn                    I2C1_EV_IRQn
#define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
#define I2Cx_ER_IRQn                    I2C1_ER_IRQn
#define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler


typedef enum RGB_Colour {
    RGB_RED   = 1,
    RGB_BLUE  = 2,
    RGB_GREEN = 3,
} RGB_Colour;

void Clock_Config();
void ClockHSE_Config();
void hardware_init();
void rgbSetColour(RGB_Colour colour);

#endif /* HARDWARE_H_ */
