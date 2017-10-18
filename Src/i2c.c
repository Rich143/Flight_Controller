#include "fc.h"
#include "debug.h"
#include "i2c.h"

/*
 * I2C Defines
 * NOTE: When changing these, also change the defines in hardware.h
 */

#define I2Cx                            I2C1
// Clocks for I2C
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

#define I2Cx_SPEED                      200000

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF4_I2C1

/* Definition for I2Cx's DMA */
#define I2Cx_TX_DMA_CHANNEL             DMA_CHANNEL_1
#define I2Cx_TX_DMA_STREAM              DMA1_Stream6
#define I2Cx_RX_DMA_CHANNEL             DMA_CHANNEL_1
#define I2Cx_RX_DMA_STREAM              DMA1_Stream5

I2C_HandleTypeDef I2cHandle;
SemaphoreHandle_t I2CMutex = NULL;
SemaphoreHandle_t I2C_DMA_CompleteSem = NULL;

void setup_I2C() {
    I2cHandle.Instance             = I2Cx;

    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    I2cHandle.Init.ClockSpeed      = I2Cx_SPEED;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    I2cHandle.Init.OwnAddress1     = 0;
    I2cHandle.Init.OwnAddress2     = 0;

    if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler("I2C init fail");
    }

    // Create I2C bus mutex
    I2CMutex = xSemaphoreCreateMutex();

    if( I2CMutex == NULL )
    {
        Error_Handler("Failed to create I2C mutex");
    }

    I2C_DMA_CompleteSem = xSemaphoreCreateBinary();

    if (I2C_DMA_CompleteSem == NULL)
    {
        Error_Handler("Failed to init I2C DMA Sem\n");
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken;

    xSemaphoreGiveFromISR(I2C_DMA_CompleteSem, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
    LED3_ON
    DEBUG_PRINT("I2C DMA error!\n");
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
    LED3_ON
    DEBUG_PRINT("I2C Abort DMA error!\n");
}

/**
 * @brief I2C MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 *           - DMA configuration for transmission request by peripheral
 *           - NVIC configuration for DMA interrupt request enable
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    static DMA_HandleTypeDef hdma_tx;
    static DMA_HandleTypeDef hdma_rx;

    GPIO_InitTypeDef  GPIO_InitStruct;

    /*##-1- Enable GPIO Clocks #################################################*/
    /* Enable GPIO TX/RX clock */
    I2Cx_SCL_GPIO_CLK_ENABLE();
    I2Cx_SDA_GPIO_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* I2C TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = I2Cx_SCL_AF;

    HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
    GPIO_InitStruct.Alternate = I2Cx_SDA_AF;

    HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Enable DMA peripheral Clock ########################################*/
    /* Enable DMA2 clock */
    DMAx_CLK_ENABLE();

    /*##-4- Configure the DMA streams ##########################################*/
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Instance                 = I2Cx_TX_DMA_STREAM;

    hdma_tx.Init.Channel             = I2Cx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_tx);

    /* Associate the initialized DMA handle to the the I2C handle */
    __HAL_LINKDMA(hi2c, hdmatx, hdma_tx);

    /* Configure the DMA handler for Transmission process */
    hdma_rx.Instance                 = I2Cx_RX_DMA_STREAM;

    hdma_rx.Init.Channel             = I2Cx_RX_DMA_CHANNEL;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_rx);

    /* Associate the initialized DMA handle to the the I2C handle */
    __HAL_LINKDMA(hi2c, hdmarx, hdma_rx);

    /*##-5- Enable peripheral Clock ############################################*/
    /* Enable I2C1 clock */
    I2Cx_CLK_ENABLE();

    /*##-6- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (I2C1_TX) */
    HAL_NVIC_SetPriority(I2Cx_DMA_TX_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(I2Cx_DMA_TX_IRQn);

    /* NVIC configuration for DMA transfer complete interrupt (I2C1_RX) */
    HAL_NVIC_SetPriority(I2Cx_DMA_RX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2Cx_DMA_RX_IRQn);

    /*##-7- Configure the NVIC for I2C #########################################*/
    /* NVIC for I2C1 */

    HAL_NVIC_SetPriority(I2Cx_ER_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(I2Cx_ER_IRQn);
    HAL_NVIC_SetPriority(I2Cx_EV_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(I2Cx_EV_IRQn);
}

/**
 * @brief I2C MSP De-Initialization
 *        This function frees the hardware resources used in this example:
 *          - Disable the Peripheral's clock
 *          - Revert GPIO, DMA and NVIC configuration to their default state
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    static DMA_HandleTypeDef hdma_tx;
    static DMA_HandleTypeDef hdma_rx;

    /*##-1- Reset peripherals ##################################################*/
    I2Cx_FORCE_RESET();
    I2Cx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Configure I2C Tx as alternate function  */
    HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
    /* Configure I2C Rx as alternate function  */
    HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);

    /*##-3- Disable the DMA Streams ############################################*/
    /* De-Initialize the DMA Stream associate to transmission process */
    HAL_DMA_DeInit(&hdma_tx);
    /* De-Initialize the DMA Stream associate to reception process */
    HAL_DMA_DeInit(&hdma_rx);

    /*##-4- Disable the NVIC for DMA ###########################################*/
    HAL_NVIC_DisableIRQ(I2Cx_DMA_TX_IRQn);
    HAL_NVIC_DisableIRQ(I2Cx_DMA_RX_IRQn);

    /*##-5- Disable the NVIC for I2C ###########################################*/
    HAL_NVIC_DisableIRQ(I2Cx_ER_IRQn);
    HAL_NVIC_DisableIRQ(I2Cx_EV_IRQn);
}

