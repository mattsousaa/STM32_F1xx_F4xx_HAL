#include "main.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart2;

void SystemClock_Config(uint8_t clock_freq);
void UART2_Init(void);

char *msg_user = "The application is running\r\n";

int main(void){

	char msg[100];

	HAL_Init();

	SystemClock_Config(SYS_CLOCK_FREQ_84_MHZ);

	UART2_Init();

	HAL_UART_Transmit(&huart2, (uint8_t*)msg_user, strlen(msg_user), HAL_MAX_DELAY);

	memset(msg,0,sizeof(msg));
	sprintf(msg, "SYSCLK: %ldHz\r\n", HAL_RCC_GetSysClockFreq());
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);


	memset(msg,0,sizeof(msg));
	sprintf(msg, "HCLK: %ldHz\r\n", HAL_RCC_GetHCLKFreq());
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);


	memset(msg,0,sizeof(msg));
	sprintf(msg, "PCLK1: %ldHz\r\n", HAL_RCC_GetPCLK1Freq());
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);


	memset(msg,0,sizeof(msg));
	sprintf(msg, "PCLK2: %ldHz\r\n", HAL_RCC_GetPCLK2Freq());
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

	while(1);

	return 0;
}


void SystemClock_Config(uint8_t clock_freq){

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	uint32_t FLatency = 0;

	/** Initializes the CPU, AHB and APB busses clocks*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	switch(clock_freq){

		case SYS_CLOCK_FREQ_25_MHZ:

			RCC_OscInitStruct.PLL.PLLM = 4;
			RCC_OscInitStruct.PLL.PLLN = 50;
			RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
			RCC_OscInitStruct.PLL.PLLQ = 2;

			RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
						                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
			RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
			RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

			FLatency = FLASH_ACR_LATENCY_0WS;

			break;

		case SYS_CLOCK_FREQ_50_MHZ:

			RCC_OscInitStruct.PLL.PLLM = 8;
			RCC_OscInitStruct.PLL.PLLN = 100;
			RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
			RCC_OscInitStruct.PLL.PLLQ = 2;

			RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
									      RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
			RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
			RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

			FLatency = FLASH_ACR_LATENCY_1WS;

			break;

		case SYS_CLOCK_FREQ_84_MHZ:

			/* Configure the main internal regulator output voltage */
			// Enable the clock for the power controller
			__HAL_RCC_PWR_CLK_ENABLE();

			// set regulator voltage scale as 2 to reach MAX frequency of 84MHz
			__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

			//turn on the over drive mode of the voltage regulator (for STM32F0446RE)
			//__HAL_PWR_OVERDRIVE_ENABLE();

			RCC_OscInitStruct.PLL.PLLM = 8;
			RCC_OscInitStruct.PLL.PLLN = 168;
			RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
			RCC_OscInitStruct.PLL.PLLQ = 2;

			RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
			                              RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
			RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
			RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

			FLatency = FLASH_ACR_LATENCY_2WS;

			break;

		default:

			return;

	}

	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
		Error_Handler();
	}

	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLatency) != HAL_OK){
		Error_Handler();
	}

	//Systick configuration
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}


void UART2_Init(void){

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX;

	if(HAL_UART_Init(&huart2) != HAL_OK){
		Error_Handler();
	}
}

void Error_Handler(void){

}

