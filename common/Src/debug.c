// Waterloo Hybrid 2016

#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "pins_common.h"
#include "pins.h"

UART_HandleTypeDef UartHandle;

void debug_init(void) {
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Enable USARTx clock */
    __HAL_RCC_USART6_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = GPIO_PIN_6;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    UartHandle.Instance          = USART6;

    UartHandle.Init.BaudRate     = 9600;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX; // TODO: Change to TX_RX if necessary
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    /*if(HAL_UART_DeInit(&UartHandle) != HAL_OK)*/
    /*{*/
        /*// TODO: Error HANDLE*/
        /*while(1);*/
    /*}*/

    if(HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        // TODO: Error HANDLE
        while(1);
    }

}
// Function to enable printf for debugging
int _write(int file, char* data, int len) {
    if (*data) LED3_TOGGLE
    char stuff[] = "Test\n";
    if (HAL_UART_Transmit(&UartHandle, (uint8_t*)stuff, 5, 5000) != HAL_OK)
    {
        // TODO: Error handle
        while(1);
    }
    /*HAL_UART_Transmit(&UartHandle, (uint8_t*)data, len, DATA_SEND_TIMEOUT);*/
    return len;
}
