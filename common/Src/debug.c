// Waterloo Hybrid 2016

#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "pins_common.h"
#include "pins.h"

#define UARTx_BAUD_RATE            115200
#define UARTx                      USART6

#define UARTx_CLK_ENABLE           __HAL_RCC_USART6_CLK_ENABLE

#define UARTx_TX_GPIO_CLK_ENABLE   __HAL_RCC_GPIOC_CLK_ENABLE
#define UARTx_RX_GPIO_CLK_ENABLE   __HAL_RCC_GPIOC_CLK_ENABLE

#define UARTx_ALTERNATE_FUNCTION   GPIO_AF8_USART6

#define UARTx_TX_GPIO_PIN          GPIO_PIN_6
#define UARTx_TX_GPIO_PORT         GPIOC

#define UARTx_RX_GPIO_PIN          GPIO_PIN_7
#define UARTx_RX_GPIO_PORT         GPIOC


UART_HandleTypeDef UartHandle;

void debug_init(void) {
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    UARTx_TX_GPIO_CLK_ENABLE();
    UARTx_RX_GPIO_CLK_ENABLE();

    /* Enable USARTx clock */
    UARTx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = UARTx_TX_GPIO_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = UARTx_ALTERNATE_FUNCTION;

    HAL_GPIO_Init(UARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = UARTx_RX_GPIO_PIN;
    GPIO_InitStruct.Alternate = UARTx_ALTERNATE_FUNCTION;

    HAL_GPIO_Init(UARTx_RX_GPIO_PORT, &GPIO_InitStruct);

    UartHandle.Instance          = UARTx;

    UartHandle.Init.BaudRate     = UARTx_BAUD_RATE;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        // TODO: Error HANDLE
        while(1);
    }

}
// Function to enable printf for debugging
int _write(int file, char* data, int len) {
    if (HAL_UART_Transmit(&UartHandle, (uint8_t*)data, len, 5000) != HAL_OK)
    {
        // TODO: Error handle
        while(1);
    }
    /*HAL_UART_Transmit(&UartHandle, (uint8_t*)data, len, DATA_SEND_TIMEOUT);*/
    return len;
}
