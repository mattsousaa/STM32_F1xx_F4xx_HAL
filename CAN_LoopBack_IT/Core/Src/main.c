/*
 * main.c
 *
 *  Created on: 16 de Junho de 2020
 *      Author: Mateus Sousa
 */

#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "main.h"

void GPIO_Init(void);
void Error_handler(void);
void UART2_Init(void);
void SystemClock_Config_HSE(uint8_t clock_freq);
void CAN1_Init(void);
void CAN1_Tx(void);
void CAN_Filter_Config(void);

UART_HandleTypeDef huart2;
CAN_HandleTypeDef hcan1;

int main(void){

	HAL_Init();

	SystemClock_Config_HSE(SYS_CLOCK_FREQ_50_MHZ);

	GPIO_Init();

	UART2_Init();

	CAN1_Init();

	/* In order to config the CAN filter configuration, you need to do the initialization of this function before CAN Start;
	 * That's why because CAN Start actually leaves the initialization mode.  */
	CAN_Filter_Config();

	/* Enabling CAN_IER control bits */
	if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_BUSOFF) != HAL_OK){
		Error_handler();
	}

	/* In order to do the normal operation of Tx and Rx, CAN has to be in normal mode;
	 * This is according to the controller state machine in the reference manual;
	 * This function moves the CAN controller from initialization mode to the normal mode. */
	if(HAL_CAN_Start(&hcan1) != HAL_OK){
		Error_handler();
	}

	CAN1_Tx();

	while(1);

	return 0;
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config_HSE(uint8_t clock_freq){

	RCC_OscInitTypeDef Osc_Init;
	RCC_ClkInitTypeDef Clock_Init;
    uint8_t flash_latency=0;

	Osc_Init.OscillatorType = RCC_OSCILLATORTYPE_HSE ;
	Osc_Init.HSEState = RCC_HSE_ON;
	Osc_Init.PLL.PLLState = RCC_PLL_ON;
	Osc_Init.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	switch(clock_freq){

	  case SYS_CLOCK_FREQ_50_MHZ:
		  Osc_Init.PLL.PLLM = 4;
		  Osc_Init.PLL.PLLN = 50;
		  Osc_Init.PLL.PLLP = RCC_PLLP_DIV2;
		  Osc_Init.PLL.PLLQ = 2;
		  Osc_Init.PLL.PLLR = 2;
		  Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		  Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		  Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		  Clock_Init.APB1CLKDivider = RCC_HCLK_DIV2;
		  Clock_Init.APB2CLKDivider = RCC_HCLK_DIV1;
          flash_latency = 1;
          break;

	  case SYS_CLOCK_FREQ_84_MHZ:
		  Osc_Init.PLL.PLLM = 4;
		  Osc_Init.PLL.PLLN = 84;
		  Osc_Init.PLL.PLLP = RCC_PLLP_DIV2;
		  Osc_Init.PLL.PLLQ = 2;
		  Osc_Init.PLL.PLLR = 2;
		  Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		  Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		  Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		  Clock_Init.APB1CLKDivider = RCC_HCLK_DIV2;
		  Clock_Init.APB2CLKDivider = RCC_HCLK_DIV1;
          flash_latency = 2;
          break;

	  case SYS_CLOCK_FREQ_120_MHZ:
		  Osc_Init.PLL.PLLM = 4;
		  Osc_Init.PLL.PLLN = 120;
		  Osc_Init.PLL.PLLP = RCC_PLLP_DIV2;
		  Osc_Init.PLL.PLLQ = 2;
		  Osc_Init.PLL.PLLR = 2;
		  Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		  Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		  Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		  Clock_Init.APB1CLKDivider = RCC_HCLK_DIV4;
		  Clock_Init.APB2CLKDivider = RCC_HCLK_DIV2;
          flash_latency = 3;
          break;

	  default:

		  return ;
	 }

	if(HAL_RCC_OscConfig(&Osc_Init) != HAL_OK){
		Error_handler();
	}

	if(HAL_RCC_ClockConfig(&Clock_Init, flash_latency) != HAL_OK){
		Error_handler();
	}

	/*Configure the systick timer interrupt frequency (for every 1 ms) */
	uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
	HAL_SYSTICK_Config(hclk_freq/1000);

	/**Configure the Systick
	*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

}

void GPIO_Init(void){

    __HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef ledgpio;
	ledgpio.Pin = GPIO_PIN_5;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA,&ledgpio);
}

void CAN1_Tx(void){

	CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;

	uint8_t our_message[5] = {'H','E','L','L','O'};

	TxHeader.DLC = 5;				/* Specifies the length of the frame that will be transmitted */
	TxHeader.StdId = 0x65D;			/* Specifies the standard identifier */
	TxHeader.IDE = CAN_ID_STD;		/* Specifies the type of identifier for the message that will be transmitted - 11 bits */
	TxHeader.RTR = CAN_RTR_DATA;	/* Specifies the type of frame for the message that will be transmitted - Data frame*/

	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, our_message, &TxMailbox) != HAL_OK){
		Error_handler();
	}
}

void CAN_Filter_Config(void){

	CAN_FilterTypeDef can1_filter_init;

	/* Examples:
	 *
	 * Accept frames only if first 3 MSBs of the standard identifier are 0s and last 2 LSBs are 1s;
	 * Standard identifier = 0x65D = 0110 0101 1101;
	 * Mask mode: Use ID register and masking register;
	 * Mask register: STID[10:3] = 111xxxxx and STID[2:0] = 11x;
	 * The filter banking will reject this frame.
	 *
	 * Accept frames only if first 3 MSBs of the standard identifier are 1s;
	 * Standard identifier = 0x65D = 0110 0101 1101;
	 * Mask mode: Use ID register and masking register;
	 * Mask register: STID[10:3] = 111xxxxx;
	 * The filter banking will reject this frame.
	 *
	 * Accept frames only if standard identifier value is exactly 0x65D or 0x651;
	 * Standard identifier = 0x65D = 0110 0101 1101;
	 * List/ID mode: Use 2 identifier registers;
	 * Identifier register 1 = 0x65D (if match, the frame will be allowed, if it's not, then check the Identifier register 2);
	 * Identifier register 2 = 0x651;
	 * The filter banking will accept this frame.
	 *
	 * Accept only request frames;
	 * Mask mode: Use ID register and masking register;
	 * Mask register: RTR = 1;
	 * When a frame is received the RTR bit will be checked in Identifier register.
	 *
	 * Accept only extended ID frames;
	 * Mask mode: Use ID register and masking register;
	 * Mask register: IDE = 1;
	 * When a frame is received the IDE bit will be checked in Identifier register.
	 *
	 * Accept all frames (below configuration):
	 * You need not to do the configuration filter at all;
	 * Just make sure to which FIFO the message should go;
	 * Make all as 0, so none of the bits will be compared with the packet frame received.
	 **/

	can1_filter_init.FilterActivation = ENABLE;
	can1_filter_init.FilterBank = 0;
	can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
	can1_filter_init.FilterIdHigh = 0x0000;
	can1_filter_init.FilterIdLow = 0x0000;
	can1_filter_init.FilterMaskIdHigh = 0x0000;
	can1_filter_init.FilterMaskIdLow = 0x0000;
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;

	if(HAL_CAN_ConfigFilter(&hcan1, &can1_filter_init) != HAL_OK){
		Error_handler();
	}
}

void UART2_Init(void){

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;

	if (HAL_UART_Init(&huart2) != HAL_OK){
		//There is a problem
		Error_handler();
	}
}

void Error_handler(void){
	while(1);
}

void CAN1_Init(void){

	hcan1.Instance = CAN1;
	hcan1.Init.Mode = CAN_MODE_LOOPBACK;
	/* The Bus-Off state is reached when TEC (transmit error counter) is greater than 255, this state is indicated by BOFF bit in CAN_ESR register.
	 * In Bus-Off state, the bxCAN is no longer able to transmit and receive messages.
	 * If ABOM is set, the bxCAN will start the recovering sequence automatically after it has entered Bus-Off state. */
	hcan1.Init.AutoBusOff = ENABLE;
	hcan1.Init.AutoRetransmission = ENABLE;		// No need for CAN loopback
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;		// See CAN master control register (CAN_MCR) - bit 3
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;	// See CAN master control register (CAN_MCR) - bit 2

	// Settings related to CAN bit timings
	// Visit http://www.bittiming.can-wiki.info/
	// Bit Rate (Bits/s) = 500kbps (2us) and number of time quanta = 10;
	// One bit = Sync + (Prop_Seg + Phase_Seg1) + Seg2 = 10;
	// (Prop_Seg + Phase_Seg1) = Seg1;

	hcan1.Init.Prescaler = 5;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;		// 1 time quantum as default (to complete 10 time quanta)
	hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;			// 8 time quantum
	hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;			// 1 time quantum

	if(HAL_CAN_Init(&hcan1) != HAL_OK){
		Error_handler();
	}
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan){
	char msg[50];
	sprintf(msg, "Message Transmitted:M0\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan){
	char msg[50];
	sprintf(msg, "Message Transmitted:M1\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan){
	char msg[50];
	sprintf(msg, "Message Transmitted:M2\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	CAN_RxHeaderTypeDef RxHeader;

	uint8_t rcvd_msg[5];
	char msg[50];

	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, rcvd_msg) != HAL_OK){
		Error_handler();
	}

	sprintf(msg, "Message Received : %s\r\n", rcvd_msg);

	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan){
	char msg[50];
	sprintf(msg, "CAN Error Detected\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

