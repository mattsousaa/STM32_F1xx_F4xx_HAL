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
void CAN1_Tx();
void CAN1_Rx(void);
void CAN_Filter_Config(void);
void TIMER6_Init(void);
void Send_response(uint32_t StdId);
void LED_Manage_Output(uint8_t led_no);

CAN_HandleTypeDef hcan1;
CAN_RxHeaderTypeDef RxHeader;
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htimer6;

uint8_t req_counter = 0;
uint8_t led_no = 0;

/*
 * ######## Screen of Node1 ########
 *
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Reply Received:0xABCD
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Message Transmitted:M0
 * Reply Received:0xABCD
 * ...
 *
 * ######## Screen of Node2 ########
 *
 * Message Received:#1
 * Message Received:#2
 * Message Received:#3
 * Message Received:#4
 * Message Transmitted:M0
 * Message Received:#1
 * Message Received:#2
 * Message Received:#3
 * Message Received:#4
 * Message Transmitted:M0
 * ...
 *
 * */

int main(void){

	HAL_Init();

	SystemClock_Config_HSE(SYS_CLOCK_FREQ_84_MHZ);

	GPIO_Init();

	UART2_Init();

	TIMER6_Init();

	CAN1_Init();

	/* In order to config the CAN filter configuration, you need to do the initialization of this function before CAN Start;
	 * That's why because CAN Start actually leaves the initialization mode.  */
	CAN_Filter_Config();

	/* Enabling CAN_IER control bits */
	if(HAL_CAN_ActivateNotification(&hcan1,CAN_IT_TX_MAILBOX_EMPTY|CAN_IT_RX_FIFO0_MSG_PENDING|CAN_IT_BUSOFF) != HAL_OK){
		Error_handler();
	}

	/* In order to do the normal operation of Tx and Rx, CAN has to be in normal mode;
	 * This is according to the controller state machine in the reference manual;
	 * This function moves the CAN controller from initialization mode to the normal mode. */
	if(HAL_CAN_Start(&hcan1) != HAL_OK){
		Error_handler();
	}

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
    uint8_t flash_latency = 0;

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

		  return;
	}

	if(HAL_RCC_OscConfig(&Osc_Init) != HAL_OK){
		Error_handler();
	}

	if(HAL_RCC_ClockConfig(&Clock_Init, flash_latency) != HAL_OK){
		Error_handler();
	}

	/* Configure the systick timer interrupt frequency (for every 1 ms) */
	uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
	HAL_SYSTICK_Config(hclk_freq/1000);

	/* Configure the Systick */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void CAN1_Tx(){

	CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;

	uint8_t message;

	TxHeader.DLC = 1;			// 1 byte
	TxHeader.StdId = 0x65D;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;

	message = led_no++;

	if(led_no == 4){
	    led_no = 0;
	}

	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);	// It toggles the user LED for every message sent

	/* Node1 sends a message (led number) using data frame for every 1sec to Node2 */
	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK){
		Error_handler();
	}
}

void CAN_Filter_Config(void){
	CAN_FilterTypeDef can1_filter_init;

	can1_filter_init.FilterActivation = ENABLE;
	can1_filter_init.FilterBank  = 0;
	can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
	can1_filter_init.FilterIdHigh = 0x0000;
	can1_filter_init.FilterIdLow = 0x0000;
	can1_filter_init.FilterMaskIdHigh = 0x0000;
	can1_filter_init.FilterMaskIdLow = 0x0000;
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;

	/* 0x65D = 110 0101 1101 (discard in node2)
	 * 0x651 = 110 0101 0001 (accept in node2)
	 *
	 * FB0_R1(32 bits) and FB0_R2(32 bits)
	 *
	 * Fields mapping for FB0_R1 and FB0_R2
	 *
	 * |(A)x x x x x x x x|(B)x x x|(C)x x x x x|(D)x x x x x x x x|(E)x x x x x|(F)x|(G)x|(H)x| 32 bits(x)
	 *
	 * A - STID[10:3] - MSB
	 * B - STID[2:0] - LSB
	 * C - EXID[17:13]
	 * D - EXID[12:5]
	 * E - EXID[4:0]
	 * F - IDE
	 * G - RTR
	 * H - 0
	 *
	 * A + B + C = HIGH PART REGISTER
	 * D + E + F + G + H = LOW PART REGISTER
	 *
	 * Mask register = |x x x x x x x 1|1 1 x|x x x x x|x x x x x x x x|x x x x x|x|x|x| = 0x01C0 (grouping by 4 bits)
	 * Id. register =  |x x x x x x x 0|0 0 x|x x x x x|x x x x x x x x|x x x x x|x|x|x|
	 *
	 * Node 2 will not receive the data frame from the Node1 (0x65D). It will only transmit to Node1.
	 * Node 2 will receive only remote frame (0x651)
	 * It will not print any reception messages, only transmission messages
	 *
	 * can1_filter_init.FilterActivation = ENABLE;
	 * can1_filter_init.FilterBank = 0;
	 * can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
	 * can1_filter_init.FilterIdHigh = 0x0000;
	 * can1_filter_init.FilterIdLow = 0x0000;
	 * can1_filter_init.FilterMaskIdHigh = 0X01C0;
	 * can1_filter_init.FilterMaskIdLow = 0x0000;
	 * can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	 * can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;
	 *
	 * */

	if(HAL_CAN_ConfigFilter(&hcan1,&can1_filter_init) != HAL_OK){
		Error_handler();
	}
}

void GPIO_Init(void){

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef ledgpio;

	/* LED for Node1 */
	ledgpio.Pin = GPIO_PIN_5;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &ledgpio);

	/* LEDs for Node2 */
	ledgpio.Pin = GPIO_PIN_9 | GPIO_PIN_8 | GPIO_PIN_6;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &ledgpio);

	/* LEDs for Node2 */
	ledgpio.Pin = GPIO_PIN_8;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &ledgpio);

	/* User button */
	ledgpio.Pin = GPIO_PIN_13;
	ledgpio.Mode = GPIO_MODE_IT_FALLING;
	ledgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &ledgpio);

	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

void TIMER6_Init(void){
	htimer6.Instance = TIM6;
	htimer6.Init.Prescaler = 4999;
	htimer6.Init.Period = 16800-1;

	if(HAL_TIM_Base_Init(&htimer6) != HAL_OK){
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

	if(HAL_UART_Init(&huart2) != HAL_OK){
		// There is a problem
		Error_handler();
	}
}

void CAN1_Init(void){

	hcan1.Instance = CAN1;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	/* The Bus-Off state is reached when TEC (transmit error counter) is greater than 255, this state is indicated by BOFF bit in CAN_ESR register.
	 * In Bus-Off state, the bxCAN is no longer able to transmit and receive messages.
	 * If ABOM is set, the bxCAN will start the recovering sequence automatically after it has entered Bus-Off state. */
	hcan1.Init.AutoBusOff = ENABLE;
	hcan1.Init.AutoRetransmission = ENABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;

	// Settings related to CAN bit timings
	// CAN1 peripheral at 42MHz
	// Visit http://www.bittiming.can-wiki.info/
	// Bit Rate (Bits/s) = 1Mbps (1us) and number of time quanta = 14;
	// One bit = Sync + (Prop_Seg + Phase_Seg1) + Seg2 = 14;
	// (Prop_Seg + Phase_Seg1) = Seg1;

	/* Increase the distance between nodes to 1 meter using twisted pair cable
	 * and see still you are able to achieve the frame transmission with 1Mbps
	 * without ACK error. If there is any error reduce the bitrate and repeat the
	 * experiment. */

	hcan1.Init.Prescaler = 3;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;		// 1 time quantum as default (to complete 10 time quanta)
	hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;			// 11 time quantum
	hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;			// 2 time quantum

	if(HAL_CAN_Init(&hcan1) != HAL_OK){
		Error_handler();
	}
}

// We don't know where mailbox is available (schedule will choose)
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan){
	char msg[50];
	sprintf(msg, "Message Transmitted:M0\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

// We don't know where mailbox is available (schedule will choose)
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan){
	char msg[50];
	sprintf(msg, "Message Transmitted:M1\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

// We don't know where mailbox is available (schedule will choose)
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan){
	char msg[50];
	sprintf(msg, "Message Transmitted:M2\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	uint8_t rcvd_msg[8];

	char msg[50];

	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rcvd_msg) != HAL_OK){
		Error_handler();
	}

	if(RxHeader.StdId == 0x65D && RxHeader.RTR == 0){
		// This is data frame sent by n1 to n2
		// After the reception of the message (led number), Node2 has to glow the corresponding LED
		LED_Manage_Output(rcvd_msg[0]);
		sprintf(msg, "Message Received : #%x\r\n", rcvd_msg[0]);

	} else if(RxHeader.StdId == 0x651 && RxHeader.RTR == 1){
		// This is a remote frame sent by n1 to n2
		Send_response(RxHeader.StdId);
		return;

	} else if(RxHeader.StdId == 0x651 && RxHeader.RTR == 0){
		// It's a reply (data frame) by n2 to n1
		sprintf(msg, "Reply Received : %#X\r\n", rcvd_msg[0] << 8 | rcvd_msg[1]); // 0xABCD
	}

	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	CAN_TxHeaderTypeDef TxHeader;
	uint32_t TxMailbox;
	uint8_t message; 						// No meaning for data frame

	if(req_counter == 4){					// Node1 sends a remote frame for every 4 seconds to request 2 bytes of data
		// N1 sending Remote frame to N2
		TxHeader.DLC = 2; 					// N1 demanding 2 bytes of reply
		TxHeader.StdId = 0x651;				// This is just an arbitrary identifier
		TxHeader.IDE = CAN_ID_STD;			// 11 bits + RTR
		TxHeader.RTR = CAN_RTR_REMOTE;		// Request or remote frame

		// Message here there is no meaning because frame messages will not be sent
		if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK){
			Error_handler();
		}

		req_counter = 0;

	} else{
		CAN1_Tx();		// Send messages for every 1 second
		req_counter++;	// Increment request counter
	}
}

void LED_Manage_Output(uint8_t led_no){

	switch(led_no){

 	case 1:
 		HAL_GPIO_WritePin(LED1_PORT, LED1_PIN_NO, GPIO_PIN_SET);
 		HAL_GPIO_WritePin(LED2_PORT, LED2_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED3_PORT, LED3_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED4_PORT, LED4_PIN_NO, GPIO_PIN_RESET);
 		break;

 	case 2:
 		HAL_GPIO_WritePin(LED1_PORT, LED1_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED2_PORT, LED2_PIN_NO, GPIO_PIN_SET);
 		HAL_GPIO_WritePin(LED3_PORT, LED3_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED4_PORT, LED4_PIN_NO, GPIO_PIN_RESET);
 		break;

 	case 3:
 		HAL_GPIO_WritePin(LED1_PORT, LED1_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED2_PORT, LED2_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED3_PORT, LED3_PIN_NO, GPIO_PIN_SET);
 		HAL_GPIO_WritePin(LED4_PORT, LED4_PIN_NO, GPIO_PIN_RESET);
 		break;

 	case 4:
 		HAL_GPIO_WritePin(LED1_PORT,LED1_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED2_PORT,LED2_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED3_PORT,LED3_PIN_NO, GPIO_PIN_RESET);
 		HAL_GPIO_WritePin(LED4_PORT,LED4_PIN_NO, GPIO_PIN_SET);
 		break;
	}
}


void Send_response(uint32_t StdId){

 	CAN_TxHeaderTypeDef TxHeader;
 	uint32_t TxMailbox;
 	uint8_t response[2] = {0xAB, 0xCD};		/* It can be anything like the temperature value, some engine state, some other sensor value,
 	 	 	 	 	 	 	 	 	 	 	 * speed information, whatever. */
 	TxHeader.DLC = 2;				// 0xAB and 0xCD (2 bytes)
 	TxHeader.StdId = StdId;			// Identifier ID of Node2
 	TxHeader.IDE = CAN_ID_STD;		// 11 bits + RTR
 	TxHeader.RTR = CAN_RTR_DATA;	// Request frame

 	/* Node2 upon receiveing the Remote frame should send back 2 bytes of data using data frame */
 	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, response, &TxMailbox) != HAL_OK){
 		Error_handler();
 	}
}

void Error_handler(void){
	while(1);
}
