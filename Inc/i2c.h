#ifndef __I2C_H
#define __I2C_H

#include "fc.h"
#include "freertos.h"
#include "semphr.h"
#include "pins.h"

#define I2C_MUT_WAIT_TICKS 10 // wait 10 ticks for the I2C bus to become available
#define I2C_DMA_SEM_WAIT_TICKS 50 // wait 3 ticks for the I2C DMA transfer to finish

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

extern I2C_HandleTypeDef I2cHandle;
extern SemaphoreHandle_t I2CMutex;
extern SemaphoreHandle_t I2C_DMA_CompleteSem;

void I2C_ClearBusyFlagErratum(uint32_t timeout);
void setup_I2C();
#endif /* defined(__I2C_H) */
