// Waterloo Hybrid 2016

#include <stdio.h>
#include <string.h>

#include "stm32f0xx_hal.h"

#include "pins_common.h"

UART_HandleTypeDef huart;
DMA_HandleTypeDef hdma_usart;

#define UART_PRIORITY (6)
#define UART_RX_SUBPRIORITY (0)

#ifdef USE_NUCLEO
#define UART_BAUD_RATE (115200)
#define DATA_SEND_TIMEOUT (100)
#else
#define UART_BAUD_RATE (460800)
#define DATA_SEND_TIMEOUT (100)
#endif

#define BUFFER_SIZE (100)

uint8_t rxBuffer = '\000';
static char rxString[BUFFER_SIZE];
static int rxindex = 0;

static char commandStr[BUFFER_SIZE];
static int commandLen = 0;

// create a global variable in your program called "PS" to overwrite this,
// DO NOT change this
__weak char PS[] = "PROC";

void DMA1_Channel4_5_IRQHandler(void);
__weak void executeSerialCommand(char str[]);

void print_now(char str[]);

void debug_init(void) {
    /* Initialize the GPIO pins in UART alternate function
     * Note: This needs to be done before initializing the UART peripheral
     */
    GPIO_InitTypeDef GPIO_InitStruct;
#ifdef USE_NUCLEO
    __HAL_RCC_USART2_CLK_ENABLE();
#else
    __HAL_RCC_USART3_CLK_ENABLE();
#endif
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* (nucleo) USART2 GPIO config:
     PA2 -> USART2_TX
     PA3 -> USART2_RX
     */

    /** (procboard) USART3 GPIO Configuration
      PB10    ------> USART3_TX
      PB11    ------> USART3_RX
      */

    GPIO_InitStruct.Pin = UART_RX | UART_TX;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
#ifdef USE_NUCLEO
    GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
#else    
    GPIO_InitStruct.Alternate = GPIO_AF4_USART3;
#endif
    HAL_GPIO_Init(UART_PORT, &GPIO_InitStruct);

#ifdef USE_NUCLEO
    huart.Instance = USART2;
#else
    huart.Instance = USART3;
#endif
    huart.Init.BaudRate = UART_BAUD_RATE;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&huart);

    // DMA strategy lifted from: https://stackoverflow.com/questions/24875873/stm32f4-uart-hal-driver

    hdma_usart.Instance = DMA1_Channel5;
    hdma_usart.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart.Init.MemInc = DMA_MINC_DISABLE;
    hdma_usart.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart.Init.Mode = DMA_CIRCULAR;
    hdma_usart.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart);

    __HAL_LINKDMA(&huart, hdmarx, hdma_usart);

    HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, UART_PRIORITY, UART_RX_SUBPRIORITY);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

    __HAL_UART_FLUSH_DRREGISTER(&huart);
    HAL_UART_Receive_DMA(&huart, &rxBuffer, 1 /* size */);
}

void DMA1_Channel4_5_6_7_IRQHandler(void) {
  HAL_NVIC_ClearPendingIRQ(DMA1_Channel4_5_6_7_IRQn);
  HAL_DMA_IRQHandler(&hdma_usart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    __HAL_UART_FLUSH_DRREGISTER(huart); // Clear the buffer to prevent overrun

    int i = 0;

    print_now((char *)&rxBuffer); // Echo the character that caused this callback so the user can see what they are typing

    if (rxBuffer == 8 || rxBuffer == 127) // If Backspace or del
    {
        print_now(" \b"); // "\b space \b" clears the terminal character. Remember we just echoced a \b so don't need another one here, just space and \b
        rxindex--; 
        if (rxindex < 0) rxindex = 0;
    }

    else if (rxBuffer == '\n' || rxBuffer == '\r') // If Enter
    {
        strlcpy(commandStr, rxString, rxindex + 1);
        commandLen = rxindex;
        executeSerialCommand(rxString);
        rxString[rxindex] = 0;
        rxindex = 0;
        for (i = 0; i < BUFFER_SIZE; i++) rxString[i] = 0; // Clear the string buffer
        print_now("\r\n");
        print_now(PS);
        print_now("> ");
    }

    else
    {
        rxString[rxindex] = (char) rxBuffer; // Add that character to the string
        rxindex++;
        if (rxindex > BUFFER_SIZE) // User typing too much, we can't have commands that big
        {
            rxindex = 0;
            for (i = 0; i < BUFFER_SIZE; i++) rxString[i] = 0; // Clear the string buffer
            print_now("\r\n");
            print_now(PS);
            print_now("> ");
        }
    }
}

void print_now(char str[]) {
  HAL_UART_Transmit(&huart, (uint8_t *)str, strlen(str), 5);
}

// implement this function in your code to act on commands received from the
// serial port
// DO NOT change this empty implementation
__weak void executeSerialCommand(char str[]) {

}

// Function to enable printf for debugging
int _write(int file, char* data, int len) {
    HAL_UART_Transmit(&huart, (uint8_t*)data, len, DATA_SEND_TIMEOUT);
    return len;
}

