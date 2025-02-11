#include "main.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart2;

void MX_USART2_UART_Init(void);
void Error_Handler(void);

char *msg_user = "The application is running\r\n";

int main(void){

	char msg[100];
	RCC_OscInitTypeDef osc_init;
	RCC_ClkInitTypeDef clk_init;

	HAL_Init();
	MX_USART2_UART_Init();

	memset(&osc_init, 0, sizeof(osc_init));

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_init.HSEState = RCC_HSE_ON;

	if(HAL_RCC_OscConfig(&osc_init) != HAL_OK){
		Error_Handler();
	}

	clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
  		    		     RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	clk_init.AHBCLKDivider = RCC_SYSCLK_DIV2;
	clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
	clk_init.APB2CLKDivider = RCC_HCLK_DIV4;

	if(HAL_RCC_ClockConfig(&clk_init, FLASH_ACR_LATENCY_0) != HAL_OK){
		Error_Handler();
	}

	/*---------------------------- AFTER THIS LINE SYSCLK is SOURCED BY HSE------------------*/

  	__HAL_RCC_HSI_DISABLE(); 	//Saves some current

  	/* LETS REDO THE SYSTICK CONFIGURATION */
  	/* F_HCLK = 4MHz; T_HCLK = 0.25us*/
  	/* If 0.25us is equivalent to 1 tick, so how many ticks we have in 1ms? */
  	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  	MX_USART2_UART_Init();

  	HAL_UART_Transmit(&huart2, (uint8_t*)msg_user, strlen(msg_user), HAL_MAX_DELAY);

  	memset(msg, 0, sizeof(msg));
  	sprintf(msg,"SYSCLK: %ldHz\r\n", HAL_RCC_GetSysClockFreq());
  	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  	memset(msg, 0, sizeof(msg));
  	sprintf(msg, "HCLK: %ldHz\r\n", HAL_RCC_GetHCLKFreq());
  	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  	memset(msg, 0, sizeof(msg));
  	sprintf(msg,"PCLK1: %ldHz\r\n", HAL_RCC_GetPCLK1Freq());
  	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  	memset(msg,0,sizeof(msg));
  	sprintf(msg, "PCLK2: %ldHz\r\n", HAL_RCC_GetPCLK2Freq());
  	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  	while(1);

  	return 0;

}

void MX_USART2_UART_Init(void){

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX;

	if(HAL_UART_Init(&huart2) != HAL_OK){
		//There is a problem
		Error_Handler();
	}
}


void Error_Handler(void){
	while(1);
}
