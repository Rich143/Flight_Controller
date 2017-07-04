/*
 * can.c
 *
 *  Created on: Mar 29, 2015
 *      Author: KabooHahahein
 */

#include "stm32f0xx_hal.h"
#include "can.h"
#include "can_heartbeat.h"
#include "pins_common.h"
#include "stdbool.h"
#include "interrupt.h"

CanRxMsgTypeDef currentRxMessage;
CAN_HandleTypeDef canHandle =
{
	.Instance = CAN,
	.Init = {
		.Prescaler = 6,        // For 48 MHz
		.Mode = CAN_MODE_NORMAL,
		.SJW = CAN_SJW_1TQ,
		.BS1 = CAN_BS1_12TQ,
		.BS2 = CAN_BS2_3TQ,
		.TTCM = DISABLE,
		.ABOM = DISABLE,
		.AWUM = DISABLE,
		.NART = DISABLE,
		.RFLM = DISABLE,
		.TXFP = DISABLE
	}
};

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	if (hcan->Instance == CAN)
	{

		/* Peripheral clock enable */
		__CAN_CLK_ENABLE();

		/**CAN GPIO Configuration
		 PB8     ------> CAN_RX
		 PB9     ------> CAN_TX
		 */
		GPIO_InitStruct.Pin = CAN_N_PIN | CAN_P_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
		HAL_GPIO_Init(CAN_PORT, &GPIO_InitStruct);

		/* System interrupt init*/
		HAL_NVIC_SetPriority(CAN_INTERRUPT, CAN_PRIORITY_BASE, 0);
		HAL_NVIC_EnableIRQ(CAN_INTERRUPT);
	}
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{

	if (hcan->Instance == CAN)
	{
		/* Peripheral clock disable */
		__CAN_CLK_DISABLE();

		/**CAN GPIO Configuration
		 PB8     ------> CAN_RX
		 PB9     ------> CAN_TX
		 */
		HAL_GPIO_DeInit(CAN_PORT, CAN_N_PIN | CAN_P_PIN);

		/* Peripheral interrupt DeInit*/
		HAL_NVIC_DisableIRQ(CAN_INTERRUPT);

	}
}

void setupAllCanFilters()
{
	setupHeartbeatFilters();
	setupCanFilters();      // You must implement this in can_data.c
}

HAL_StatusTypeDef initStatus;
void canInit(const uint16_t thisIdBase)
{
	initStatus = HAL_CAN_Init(&canHandle);
	initHeartBeat(thisIdBase);
	setupAllCanFilters();

	canHandle.pRxMsg = &currentRxMessage;
	HAL_CAN_Receive_IT(&canHandle, CAN_FIFO0);
}

uint16_t currentFilterNumber = 0;
void addCanFilter(uint16_t addresses[CAN_NUM_ADDR_PER_FILTER])
{

	// In list-16 bit mode, you can add a chain of 4 addresses
	CAN_FilterConfTypeDef canFilter =
	{
		.BankNumber = 0,
		.FilterNumber = currentFilterNumber,
		.FilterMode = CAN_FILTERMODE_IDLIST,
		.FilterScale = CAN_FILTERSCALE_16BIT,
		.FilterFIFOAssignment = CAN_FILTER_FIFO0,
		.FilterActivation = ENABLE,
		.FilterIdLow = addresses[0] << NUM_ID_BIT_SHIFT,
		.FilterIdHigh = addresses[1] << NUM_ID_BIT_SHIFT,
		.FilterMaskIdLow = addresses[2] << NUM_ID_BIT_SHIFT,
		.FilterMaskIdHigh = addresses[3] << NUM_ID_BIT_SHIFT
	};

	HAL_CAN_ConfigFilter(&canHandle, &canFilter);

	if (currentFilterNumber < 27)   // Max number of filters
		currentFilterNumber ++;
}

bool sendCanMessageTimeoutMs(const uint16_t id, const uint8_t *data,
                             const uint8_t length, const uint32_t timeout)
{
	uint32_t beginTime = HAL_GetTick();
	bool success = false;
	while (HAL_GetTick() < beginTime + timeout && !success)
	{
		success = sendCanMessage(id, data, length);
	}
	return success;
}

bool sendCanMessage(const uint16_t id, const uint8_t *data, const uint8_t length)
{
	if (length > CAN_MAX_BYTE_LEN)
		return false;    // Programmer error
	if (initStatus != HAL_OK)
		return false;
	if (canHandle.Instance->MSR == CAN_MCR_RESET)
		return false;

	HAL_Delay(1);	// Wait for a previous message to flush out HW

	CanTxMsgTypeDef txMessage =
	{
		.StdId = id,
		.IDE = CAN_ID_STD,
		.RTR = CAN_RTR_DATA,
		.DLC = length
	};

	uint8_t i;
	for (i = 0; i < length; i++)
		txMessage.Data[i] = data[i];

	canHandle.pTxMsg = &txMessage;
	HAL_StatusTypeDef status = HAL_CAN_Transmit_IT(&canHandle);

	if (status == HAL_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
* @brief This function handles HDMI_CEC and CAN interrupts (combined with EXTI line 27).
*/
void CEC_CAN_IRQHandler(void)
{
	currentRxMessage = (CanRxMsgTypeDef)
	{
		0
	};
	HAL_CAN_IRQHandler(&canHandle);
	receiveHeartBeat(&currentRxMessage);
	HAL_CAN_Receive_IT(&canHandle, CAN_FIFO0);
}

uint32_t error = HAL_CAN_ERROR_NONE;
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	// Deal with error

	error = hcan->ErrorCode;
//#define HAL_CAN_ERROR_NONE              ((uint32_t)0x00000000)  /*!< No error             */
//#define HAL_CAN_ERROR_EWG               ((uint32_t)0x00000001)  /*!< EWG error            */
//#define HAL_CAN_ERROR_EPV               ((uint32_t)0x00000002)  /*!< EPV error            */
//#define HAL_CAN_ERROR_BOF               ((uint32_t)0x00000004)  /*!< BOF error            */
//#define HAL_CAN_ERROR_STF               ((uint32_t)0x00000008)  /*!< Stuff error          */
//#define HAL_CAN_ERROR_FOR               ((uint32_t)0x00000010)  /*!< Form error           */
//#define HAL_CAN_ERROR_ACK               ((uint32_t)0x00000020)  /*!< Acknowledgment error */
//#define HAL_CAN_ERROR_BR                ((uint32_t)0x00000040)  /*!< Bit recessive        */
//#define HAL_CAN_ERROR_BD                ((uint32_t)0x00000080)  /*!< LEC dominant         */
//#define HAL_CAN_ERROR_CRC               ((uint32_t)0x00000100)  /*!< LEC transfer error   */

	__HAL_CAN_CANCEL_TRANSMIT(hcan, CAN_TXMAILBOX_0);
	__HAL_CAN_CANCEL_TRANSMIT(hcan, CAN_TXMAILBOX_1);
	__HAL_CAN_CANCEL_TRANSMIT(hcan, CAN_TXMAILBOX_2);
	hcan->Instance->MSR |= CAN_MCR_RESET;

}

