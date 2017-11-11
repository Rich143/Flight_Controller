#include <string.h>

#include "diskio.h"

#include "FreeRTOS.h"
#include "task.h"

#include "fc.h"

/**
 * @file Src/sd.c
 *
 * @brief Driver for sd card. Implements FatFS hooks
 *
 *
 *  SD card driver. This file implements all the functions that FatFS requires
 *  to be able to communicate with the sd card. Therefore, these functions
 *  shouldn't be called directly
 *
 *
 * Modifications:
 * 3/2/17 Richard Matthews - Initial Creation
 *
 * @author Richard Matthews
 * @date February 3, 2017
 */

/* SD and SPI private defines */

#define SPIx                             SPI1
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define SPIx_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT               GPIOA
#define SPIx_SCK_AF                      GPIO_AF5_SPI1
#define SPIx_MISO_PIN                    GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT              GPIOA
#define SPIx_MISO_AF                     GPIO_AF5_SPI1
#define SPIx_MOSI_PIN                    GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT              GPIOA
#define SPIx_MOSI_AF                     GPIO_AF5_SPI1
#define SPIx_NSS_GPIO_PIN                GPIO_PIN_4
#define SPIx_NSS_GPIO_PORT               GPIOA


#define SD_SPI_TIMEOUT                   5000
// Number of retries after incorrect response
#define SD_SPI_NUM_RETRIES               10

// Length of time to wait for data start
#define SD_READ_TIMEOUT_TICKS            200

// Various SD Card protocol parameters

/* MMC/SD command */
#define CMD0	(0)			/**< GO_IDLE_STATE */
#define CMD1	(1)			/**< SEND_OP_COND (MMC) */
#define CMD8	(8)			/**< SEND_IF_COND */
#define CMD9	(9)			/**< SEND_CSD */
#define CMD10	(10)		/**< SEND_CID */
#define CMD12	(12)		/**< STOP_TRANSMISSION */
#define CMD16	(16)		/**< SET_BLOCKLEN */
#define CMD17	(17)		/**< READ_SINGLE_BLOCK */
#define CMD18	(18)		/**< READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/**< SET_BLOCK_COUNT (MMC) */
#define CMD24	(24)		/**< WRITE_BLOCK */
#define CMD25	(25)		/**< WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/**< ERASE_ER_BLK_START */
#define CMD33	(33)		/**< ERASE_ER_BLK_END */
#define CMD38	(38)		/**< ERASE */
#define CMD55	(55)		/**< APP_CMD */
#define CMD58	(58)		/**< READ_OCR */
/* ACMD, need to send CMD55 first */
#define ACMD41   (0x80+41)
#define ACMD23   (0x80+23)


#define BLOCK_SIZE                       512

// Max number of bytes until response from SD card, one extra byte is added to
// this to account for CM12, where there is an extra stuff byte that is
// discarded
#define SD_NCR_LENGTH_BYTES              9

#define SD_RESPONSE_R7_LENGTH_BYTES      5
#define SD_RESPONSE_R1_LENGTH_BYTES      1

#define SD_CMD_ARG_CRC_LENGTH_BYTES      6

#define SPI_TX_BUFFER_R1_LENGTH_BYTES    (SD_CMD_ARG_CRC_LENGTH_BYTES        \
                                         + SD_NCR_LENGTH_BYTES               \
                                         + SD_RESPONSE_R1_LENGTH_BYTES)

#define SPI_RX_BUFFER_R1_LENGTH_BYTES    (SD_CMD_ARG_CRC_LENGTH_BYTES        \
                                         + SD_NCR_LENGTH_BYTES               \
                                         + SD_RESPONSE_R1_LENGTH_BYTES)

#define SPI_TX_BUFFER_R7_LENGTH_BYTES    (SD_CMD_ARG_CRC_LENGTH_BYTES        \
                                         + SD_NCR_LENGTH_BYTES               \
                                         + SD_RESPONSE_R7_LENGTH_BYTES)

#define SPI_RX_BUFFER_R7_LENGTH_BYTES    (SD_CMD_ARG_CRC_LENGTH_BYTES        \
                                         + SD_NCR_LENGTH_BYTES               \
                                         + SD_RESPONSE_R7_LENGTH_BYTES)

// Macros to select and deselect SD Card, note that NSS is active low
#define CS_ENABLE() (HAL_GPIO_WritePin(SPIx_NSS_GPIO_PORT, SPIx_NSS_GPIO_PIN,\
                                       GPIO_PIN_RESET))
#define CS_DISABLE() (HAL_GPIO_WritePin(SPIx_NSS_GPIO_PORT, SPIx_NSS_GPIO_PIN,\
                                       GPIO_PIN_SET))


/* Private variables */

SPI_HandleTypeDef SpiHandle;

// A array of BLOCK_SIZE bytes, initialized to 0xFF in SD_Init, used to keep
// the DOUT line high during block reads
uint8_t txBufferRead[BLOCK_SIZE];
// Dummy buffer to pass to HAL_SPI_TransmitReceive as rxBuffer
// If we don't read anything, the hardware rx FIFO will store what we received
// and mess up the next read
uint8_t rxBufferWrite[BLOCK_SIZE];

static volatile DSTATUS Stat = STA_NOINIT;



/* Private Functions */

void SPI_Init(void) {
  SpiHandle.Instance               = SPIx;

  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;

  SpiHandle.Init.Mode = SPI_MODE_MASTER;

  if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
  {
    /* Initialization Error */
  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  if(hspi->Instance == SPIx)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI clock */
    SPIx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    // MISO requires a pullup for proper SD Card initialization
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    /* SPI NSS GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_NSS_GPIO_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Mode      = GPIO_PULLUP;
    HAL_GPIO_Init(SPIx_NSS_GPIO_PORT, &GPIO_InitStruct);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance == SPIx)
  {
    /*##-1- Reset peripherals ##################################################*/
    SPIx_FORCE_RESET();
    SPIx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Configure SPI SCK as alternate function  */
    HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
    /* Configure SPI MISO as alternate function  */
    HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
    /* Configure SPI MOSI as alternate function  */
    HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);
  }
}

/**
 * @brief Initialize the SD Card Driver
 *
 * @return Indicates if the SD Card was succesfully initialized
 * +     0  = Success
 */
int8_t SD_Init() {
    memset(txBufferRead, 0xFF, BLOCK_SIZE);
    SPI_Init();
    return 0;
}

/**
 * @brief wrapper function to facilitate spi transactions of single byte length
 *
 * @param send The byte to send
 *
 * @return The received byte
 */
BYTE xchg_spi(BYTE send) {
    BYTE rx;

    HAL_SPI_TransmitReceive(&SpiHandle, &send, &rx, 1, SD_SPI_TIMEOUT);

    return rx;
}

/**
 * @brief Wait for the card to be ready
 *
 * @param wait The timeout in milliseconds
 *
 * @return  1: ready, 0: timeout
 */
int wait_ready(uint32_t wait) {
    uint8_t response = 0;


    TickType_t startTime = HAL_GetTick();

    // Wait for data start
    while (HAL_GetTick() < startTime + wait) {
        response = xchg_spi(0xFF);
        if (response == 0xFF) {
            break;
        }
    }

    return (response == 0xFF) ? 1 : 0;
}

/**
 * @brief Deselect card and release spi
 */
void deselect_card() {
    CS_DISABLE();
    // Dummy clock to force DO hi-z for multiple clock cycles
    xchg_spi(0xFF);
}

/**
 * @brief Select the card and wait for ready
 *
 * @return 1: OK, 0: Timeout
 */
int select_card() {
    CS_ENABLE();
    // Dummy clock (force DO enables)
    xchg_spi(0xFF);

    if (wait_ready(500)) return 1;

    deselect_card();
    return 0;
}

/**
 * @brief Send a command to the SD card, and expect a R1 response back
 *
 * Sends the specified command and arguments to the SD card over spi, and read
 * back the response
 *
 * @param[in] cmd  The SD card command to send, this gets transformed into
 *                 8 bits to send to the SD card by adding the prefix
 *                 0b01. In the protocol, it
 *                 specifies commands indices as being 6 bits,
 *                 preceded by 0b01
 * @param[in] arg  The 4 bytes of arguments to the command
 * @param[in] crc  The CRC to send with the command. For SPI mode, the SD Card
 *                 doesn't typically care about the crc, so it can be an
 *                 arbitrary value.However, it does care when being initialized
 *                 into SPI mode, as this is when it is in SD and SPI mode, so
 *                 needs a valid CRC
 * @return         The SD card's repsonse, or 0xFF if no response (or a SPI Error)
 */
uint8_t SD_Command_R1(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t i, ret = 0xFF;
    uint8_t rxBuffer[SD_CMD_ARG_CRC_LENGTH_BYTES] = {0};
    uint8_t txBuffer[SD_CMD_ARG_CRC_LENGTH_BYTES];

    if (cmd & 0x80) {
        cmd &= 0x7F;
        ret = SD_Command_R1(CMD55, 0, 0xFF);
        if (ret > 1) return ret;
    }
    if (cmd > 63) {
        // Invalid command
        return -1;
    }

    /* Select the card and wait for ready, except to stop multiple block
     * read */
    if (cmd != CMD12) {
        deselect_card();
        if (!select_card()) {
            printf("Failed to select card\n");
            return 0xFF;
        }
    }

    txBuffer[0] = cmd | 0x40;
    txBuffer[1] = arg>>24;
    txBuffer[2] = arg>>16;
    txBuffer[3] = arg>>8;
    txBuffer[4] = arg;
    txBuffer[5] = crc;


    if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                            SD_CMD_ARG_CRC_LENGTH_BYTES,
                            SD_SPI_TIMEOUT) != 0) {
        return 0xFF;
    }


    if (cmd == CMD12) xchg_spi(0xFF);

    // wait for response (10 bytes max);
    i = 10;
    do {
        ret = xchg_spi(0xFF);
    } while ((ret & 0x80) && --i);

    return ret;
}

/**
 * @brief Send a command to the SD card, and expect a R7 response back
 *
 * Sends the specified command and arguments to the SD card over spi, and read
 * back the response
 *
 * @param[in] cmd     The SD card command to send, this gets transformed into
 *                    8 bits to send to the SD card by adding the prefix
 *                    0b01. In the protocol, it
 *                    specifies commands indices as being 6 bits,
 *                    preceded by 0b01
 * @param[in] arg     The 4 bytes of arguments to the command
 * @param[in] crc     The CRC to send with the command. For SPI mode, the SD Card
 *                    doesn't typically care about the crc, so it can be an
 *                    arbitrary value.However, it does care when being initialized
 *                    into SPI mode, as this is when it is in SD and SPI mode, so
 *                    needs a valid CRC
 * @param[out] response A pointer to a buffer to return the SD Card response.
 *                    This is only written to if the SD card returns a response
 * @return Indicates if the command succeeded
 * +     0  = Success
 * +     -1 = Parameter Error
 * +     -2 = No response found
 * +     -3 = SPI Error
 */
int8_t SD_Command_R7(uint8_t cmd, uint32_t arg, uint8_t crc,
                      uint8_t *response)
{
    uint8_t i;
    uint8_t rxBuffer[SPI_RX_BUFFER_R7_LENGTH_BYTES] = {0};
    uint8_t txBuffer[SPI_TX_BUFFER_R7_LENGTH_BYTES];

    if (response == NULL) {
        return -1;
    }

    if (cmd > 63) {
        // Invalid command
        return -1;
    } else {
        cmd |= 0x40;
    }

    txBuffer[0] = cmd;
    txBuffer[1] = arg>>24;
    txBuffer[2] = arg>>16;
    txBuffer[3] = arg>>8;
    txBuffer[4] = arg;
    txBuffer[5] = crc;

    for (i = 0; i<(SD_NCR_LENGTH_BYTES+SD_RESPONSE_R7_LENGTH_BYTES); i++) {
        // Read portion of the transaction, keep MOSI high
        txBuffer[i+SD_CMD_ARG_CRC_LENGTH_BYTES] = 0xFF;
    }

    /* Select the card and wait for ready, except to stop multiple block
     * read */
    if (cmd != CMD12) {
        deselect_card();
        if (!select_card()) {
            return 0xFF;
        }
    }

    if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                            SPI_TX_BUFFER_R7_LENGTH_BYTES,
                            SD_SPI_TIMEOUT) != 0) {
        return -3;
    }

    // Look through the receive buffer, starting just after sending the CRC, to
    // find the response, which is the first byte that isn't 0xFF
    for (i=SD_CMD_ARG_CRC_LENGTH_BYTES;
         (i-SD_CMD_ARG_CRC_LENGTH_BYTES)<(SD_NCR_LENGTH_BYTES+1);
         i++)
    {
        if (rxBuffer[i] != 0xFF) {
            memcpy(response, &(rxBuffer[i]), SD_RESPONSE_R7_LENGTH_BYTES);
            break;
        }
    }

    if (i == SD_NCR_LENGTH_BYTES) {
        return -2; // no response found
    }

    return 0;
}

/**
 * @brief Read a block from the SD Card
 *
 * Reads back a 512 byte data block from the SD Card
 *
 * @param[in] block The block address of the block to read (SDHC cards are indexed by
 * block, so the byte address would be block*512? //TODO: Check this)
 * @param[out] data A pointer to a portion of memory to store the read block
 * @return Indicates if the SD Card was succesfully initialized
 * +     0  = Success
 * +     -1 = R1 contains error, or timeout error
 * +     -2 = SPI Error
 */
int8_t SD_Read_Block(uint32_t block, uint8_t *data) {
    uint8_t i, ret;
    uint8_t rxBuffer[1] = {0xFF};
    uint8_t txBuffer[1] = {0xFF};

    ret = SD_Command_R1(CMD17, block, 0xFF);
    if (ret != 0x00) {
        // Error
        printf("CMD17 R1 error during single block read: %d\n", ret);
        return -1;
    }

    TickType_t startTime = HAL_GetTick();

    // Wait for data start
    while (HAL_GetTick() < startTime + SD_READ_TIMEOUT_TICKS) {
        if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                            sizeof(rxBuffer), SD_SPI_TIMEOUT) != 0)
        {
            printf("Spi send failed\n");
            return -2;
        }
        if (rxBuffer[0] != 0xFF) {
            break;
        }
    }

    if (rxBuffer[0] != 0xFE) {
        printf("Timed out waiting for data start token\n");
        return -1;
    }

    // Read in the data
    if (HAL_SPI_TransmitReceive(&SpiHandle, txBufferRead, data, BLOCK_SIZE,
                            SD_SPI_TIMEOUT) != 0) {
        printf("Spi send failed\n");
        return -2;
    }

    // Read in the checksum
    for (i=0; i<2; i++) {
        if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                                    sizeof(rxBuffer), SD_SPI_TIMEOUT) != 0)
        {
            printf("Spi send failed\n");
            return -2;
        }
    }

    return 0;
}


/**
 * @brief Read multiple blocks from the SD Card
 *
 * @param[in] block    The block address of the block to start reading from
 * (SDHC cards are indexed by block, so the byte address would be block*512?
 * //TODO: Check this)
 * @param[out] data    A pointer to a portion of memory to store the read block
 * @param[in]  count   The number of sectors to read
 * @return Indicates if the SD Card was succesfully initialized
 * +     0  = Success
 * +     -1 = R1 contains error, or timeout
 * +     -2 = SPI Error
 */
int8_t SD_Read_Multiple_Blocks(uint32_t block, uint8_t *data, uint32_t count) {
    uint8_t i, ret;
    uint8_t rxBuffer[1] = {0xFF};
    uint8_t txBuffer[1] = {0xFF};

    ret = SD_Command_R1(CMD18, block, 0xFF);
    if (ret != 0x00) {
        // Error
        printf("CMD18 R1 error during multi block read: %d\n", ret);
        return -1;
    }

    TickType_t startTime = HAL_GetTick();

    // Wait for data start
    while (HAL_GetTick() < startTime + SD_READ_TIMEOUT_TICKS) {
        if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                            sizeof(rxBuffer), SD_SPI_TIMEOUT) != 0)
        {
            printf("Spi send failed\n");
            return -2;
        }
        if (rxBuffer[0] != 0xFF) {
            break;
        }
    }

    if (rxBuffer[0] != 0xFE) {
        printf("Timed out waiting for data start token\n");
        return -1;
    }

    // TODO: Maybe could use the CRC peripheral to discard the CRC bytes?
    // This means this could all be done in one transfer
    while (count != 0) {
        // Read in the data
        if (HAL_SPI_TransmitReceive(&SpiHandle, txBufferRead, data, BLOCK_SIZE,
                            SD_SPI_TIMEOUT) != 0) {
            printf("SPI error during multi block read\n");
            return -2;
        }

        // Read in the checksum
        for (i=0; i<2; i++) {
            if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                                sizeof(rxBuffer), SD_SPI_TIMEOUT) != 0)
            {
                printf("SPI error during multi block read\n");
                return -2;
            }
        }

        count--;
        data += BLOCK_SIZE;
    }

    printf("Sending command 12\n");
    if (SD_Command_R1(CMD12, 0, 0xFF) == 0xFF) {
        printf("Stop command error\n"); // Stop receiving
        return -1;
    }

    return 0;
}

/**
 * @brief Write a block to the SD Card
 *
 * @param[in] block    The block address of the block to start reading from
 * (SDHC cards are indexed by block, so the byte address would be block*512?
 * //TODO: Check this)
 * @param[in]  data    A pointer to a portion of memory to store the read block
 * @return Indicates if the SD Card was succesfully initialized
 * +     0  = Success
 * +     -1 = R1 contains error, data response contains error, or timeout
 * +     -2 = SPI Error
 */
int8_t SD_Write_Block(uint32_t block, const uint8_t *data) {
    uint8_t i, ret;
    HAL_StatusTypeDef rc;
    uint8_t rxBuffer[1] = {0xFF};
    uint8_t txBuffer[1] = {0xFF};

    ret = SD_Command_R1(CMD24, block, 0xFF);
    if (ret != 0x00) {
        // Error
        printf("CMD24 R1 error during single block write: %d\n", ret);
        return -1;
    }

    if (!wait_ready(500)) return 0;


    // Send data start token
    txBuffer[0] = 0xFE;
    rc = HAL_SPI_TransmitReceive(&SpiHandle, txBuffer,
                                 rxBuffer,
                                 sizeof(txBuffer),
                                 SD_SPI_TIMEOUT);
    if (rc != HAL_OK) {
        printf("1 SPI error during single block write %d\n", rc);
        return -2;
    }


    // Write the data
    rc = HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t *)data,
                                 rxBufferWrite, BLOCK_SIZE,
                                 SD_SPI_TIMEOUT);
    if (rc != HAL_OK) {
        printf("2 SPI error during single block write %d\n", rc);
        return -2;
    }

    txBuffer[0] = 0xFF;
    // Send dummy checksum
    for (i=0; i<2; i++) {
        rc = HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                                    sizeof(rxBuffer), SD_SPI_TIMEOUT);
        if (rc != HAL_OK) {
            printf("3 SPI error during single block write %d\n", rc);
            return -2;
        }
    }

    rc = HAL_SPI_TransmitReceive(&SpiHandle, txBuffer,
                                 rxBuffer,
                                 sizeof(rxBuffer),
                                 SD_SPI_TIMEOUT);
    if (rc != HAL_OK) {
        printf("4 SPI error during single block write %d\n", rc);
        return -2;
    }

    if ((rxBuffer[0] & 0x1F) != 0x05) {
        // data response contains error
        printf("Data response of single block write contains error: %d\n",
               rxBuffer[0] & 0x1F);
        return -1;
    }

    return 0;
}

/**
 * @brief Write multiple blocks to the SD Card
 *
 * @param[in] block    The block address of the block to start reading from
 * (SDHC cards are indexed by block, so the byte address would be block*512?
 * //TODO: Check this)
 * @param[in]  data    A pointer to a portion of memory to store the read block
 * @param[in]  count   The number of sectors to read
 * @return Indicates if the SD Card was succesfully initialized
 * +     0  = Success
 * +     -1 = R1 contains error, or timeout
 * +     -2 = SPI Error
 */
int8_t SD_Write_Multiple_Blocks(uint32_t block, const uint8_t *data, uint32_t count) {
    uint8_t i, ret;
    uint8_t rxBuffer[1] = {0xFF};
    uint8_t txBuffer[1] = {0xFF};

    ret = SD_Command_R1(CMD25, block, 0xFF);
    if (ret != 0x00) {
        // Error
        printf("CMD25 R1 error during multi block write: %d\n", ret);
        return -1;
    }

    while (count) {
        // Wait for the card to become ready
        if (!wait_ready(500)) return -1;


        // Send data start token
        txBuffer[0] = 0xFC;
        if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer, sizeof(txBuffer),
                                    SD_SPI_TIMEOUT) != 0) {
            printf("SPI error during multi block write\n");
            return -2;
        }

        // Write the data
        if (HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t *)data, rxBufferWrite, BLOCK_SIZE,
                                    SD_SPI_TIMEOUT) != 0) {
            printf("SPI error during multi block write\n");
            return -2;
        }

        txBuffer[0] = 0xFF;
        // Send dummy checksum
        for (i=0; i<2; i++) {
            if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                                        sizeof(rxBuffer), SD_SPI_TIMEOUT) != 0)
            {
                printf("SPI error during multi block write\n");
                return -2;
            }
        }

        // Receive data response
        if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                                    sizeof(rxBuffer), SD_SPI_TIMEOUT) != 0)
        {
            printf("SPI error during multi block write\n");
            return -2;
        }

        if ((rxBuffer[0] & 0x1F) != 0x05) {
            // data response contains error
            printf("Data response of multi block write contains error: %d\n",
                   rxBuffer[0] & 0x1F);
            return -1;
        }

        count--;
        data += 512;
    }

    if (!wait_ready(500)) return -1;

    // Send data end token
    txBuffer[0] = 0xFD;
    if (HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer, sizeof(txBuffer),
                                SD_SPI_TIMEOUT) != 0) {
        printf("SPI error during multi block write\n");
        return -2;
    }

    /*if (!wait_ready(500)) return -1;*/


    return 0;
}

/* Public functions used by FatFS */

DSTATUS disk_initialize(BYTE drv) {
    uint8_t i;
    uint8_t txBuffer[20];
    uint8_t rxBuffer[20] = {0};
    uint8_t response[SD_RESPONSE_R7_LENGTH_BYTES];

    if (drv != 0) return STA_NOINIT;

    // TODO: Check for disk insertion
    if (0) return STA_NODISK;

    SD_Init();

    CS_DISABLE();

    // Send 10 bytes with DO High. This is just sending 80 clock cycles to let
    // the SD card initialize
    for (i=0; i<sizeof(txBuffer); i++) {
        txBuffer[i] = 0xFF;
    }

    HAL_SPI_TransmitReceive(&SpiHandle, txBuffer, rxBuffer,
                            sizeof(txBuffer), SD_SPI_TIMEOUT);

    for (i=0; i<SD_SPI_NUM_RETRIES; i++) {
        if (0xFF != SD_Command_R1(CMD0, 0x00000000, 0x95))
        {
            break; // Card is ready
        }
        HAL_Delay(100);
    }

    if (i == SD_SPI_NUM_RETRIES) {
        // Card did not respond to initialization
        printf("Card did not respond to CMD0 initialization\n");
        return STA_NOINIT;
    }

    for (i=0; i<SD_SPI_NUM_RETRIES; i++) {
        if (SD_Command_R7(CMD8, 0x000001AA, 0x87, response) != 0)
        {
            // error
            printf("Command R7 error sending CMD8\n");
            return STA_NOINIT;
        }

        if (response[0] != 0xFF) {
            break;
        }
        HAL_Delay(100);
    }

    if (i == SD_SPI_NUM_RETRIES) {
        // Card did not respond to initialization
        printf("Card did not respond to CMD8 initialization\n");
        return STA_NOINIT;
    }

    if (response[0] != 0x01) {
        // Card does not support CMD8 (not necessarily an error, but not
        // implemented in this code)
        // TODO: check if this is illegal command, and if so this
        // is an old SD protocol so should act differently
        printf("Card does not support CMD8\n");
        return STA_NOINIT;
    } else if (response[3] != 0x01) {
        // Incorrect voltage level support
        printf("Incorrect voltage level support\n");
        return STA_NOINIT;
    } else if (response[4] != 0xAA) {
        // Error, this is not supposed to happen
        printf("Other CMD8 error\n");
        return STA_NOINIT;
    }

    // We know know the SD card is Version 2 or later and compatible with our
    // voltage supply

    for (i=0; i<SD_SPI_NUM_RETRIES; i++) {
        if (0x00 == SD_Command_R1(ACMD41, 0x40000000, 0xFF)) {
            break;
        }
        HAL_Delay(100);
    }

    if (i == SD_SPI_NUM_RETRIES) {
        // Card did not come out of idle
        printf("Card did not come out of idle\n");
        return STA_NOINIT;
    }

    if (SD_Command_R7(CMD58, 0x00000000, 0xFF, response) !=0) {
        // error
        printf("CMD R7 error sending CMD 58\n");
        return STA_NOINIT;
    }

    if (response[1] & 0x40) {
        // SDHC
        printf("Card type SDHC\n");
    } else {
        // SDSC
        printf("Card type SDSC\n");
    }

    deselect_card();

    // Successfully initialized
    Stat &= ~STA_NOINIT; // Clear STA_NOINIT flag
    printf("Succesfully initialized\n");
    return Stat;
}

DSTATUS disk_status(BYTE drv)
{
    if (drv != 0) return STA_NOINIT; // Only support drive 0

    return Stat;
}

DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, UINT count)
{
    if (drv != 0 || count == 0) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    // TODO: Check if card inserted

    if (count == 1) {
        if (SD_Read_Block(sector, buff) != 0) {
            return RES_ERROR;
        }
        count = 0;
    } else {
        if (SD_Read_Multiple_Blocks(sector, buff, count) != 0) {
            return RES_ERROR;
        }
    }
    deselect_card();

    return RES_OK;
}

DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, UINT count)
{
    if (drv != 0 || count == 0) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    if (count == 1) {
        if (SD_Write_Block(sector, buff) != 0) {
            return RES_ERROR;
        }
        count = 0;
    } else {
        SD_Command_R1(ACMD23, count, 0xFF);
        if (SD_Write_Multiple_Blocks(sector, buff, count) != 0) {
            return RES_ERROR;
        }
    }
    deselect_card();

    return RES_OK;
}

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE cmd,		/* Control command code */
	void *buff		/* Pointer to the conrtol data */
)
{
	DRESULT res;

	if (drv) return RES_PARERR;					/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		if (select_card()) res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	deselect_card();

	return res;
}
