#include <string.h>
#include <stdio.h>
#include "main.h"
#include "stm32f1xx_hal_tim.h"

void SystemClock_Config();
void MX_USART2_UART_Init(void);
void Frequency_Calculation(void);
void TIMER2_Init(void);
void GPIO_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer2;
UART_HandleTypeDef huart2;

uint32_t input_captures[2] = {0};
volatile uint8_t count = 1;
volatile uint8_t is_capture_done = FALSE;

char usr_msg[100];

int main(void){

	HAL_Init();
	SystemClock_Config();
	GPIO_Init();
	TIMER2_Init();

	MX_USART2_UART_Init();

	if(HAL_TIM_IC_Start_IT(&htimer2,TIM_CHANNEL_2) != HAL_OK){
		Error_Handler();
	}


	while(1);

  	return 0;
}

/* For Cortex-M3 (STM32F1xx)*/
void SystemClock_Config(){

      RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

	  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
	    Error_Handler();
	  }

	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
	                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK){
	    Error_Handler();
	  }

}

void GPIO_Init(void){

	GPIO_InitTypeDef ledgpio;

	/* GPIOA clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* High Level GPIO Initialization */
	ledgpio.Pin = GPIO_PIN_5;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	ledgpio.Speed = GPIO_SPEED_FREQ_LOW;

	/* Init GPIO */
	HAL_GPIO_Init(GPIOA, &ledgpio);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){

	//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

	if(! is_capture_done){
		if(count == 1){
			input_captures[0] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
			//sprintf(usr_msg, "Capture 1: %ld \r\n", input_captures[0]);
			//HAL_UART_Transmit(&huart2,(uint8_t*)usr_msg,strlen(usr_msg),HAL_MAX_DELAY);
			count++;
		} else if (count == 2){
			input_captures[1] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
			//sprintf(usr_msg, "Capture 2: %ld \r\n", input_captures[1]);
			//HAL_UART_Transmit(&huart2,(uint8_t*)usr_msg,strlen(usr_msg),HAL_MAX_DELAY);
			Frequency_Calculation();
			count = 1;
			is_capture_done = FALSE;
		}
	}
}

void TIMER2_Init(void){

	TIM_IC_InitTypeDef sConfigIC = {0};

	htimer2.Instance = TIM2;
	htimer2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer2.Init.Period = 0xFFFF;
	htimer2.Init.Prescaler = 1;

	if(HAL_TIM_IC_Init(&htimer2) != HAL_OK){
		Error_Handler();
	}

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;

	if(HAL_TIM_IC_ConfigChannel(&htimer2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK){
	    Error_Handler();
	}

}

void Frequency_Calculation(void){

	uint32_t capture_difference = 0;
	double timer2_cnt_freq = 0;
	double timer2_cnt_res = 0;
	double user_signal_time_period = 0;
	double user_signal_freq = 0;

	if(input_captures[1] > input_captures[0])
		capture_difference = input_captures[1] - input_captures[0];
	else
		capture_difference = (0XFFFF - input_captures[0]) + input_captures[1];

	timer2_cnt_freq = (HAL_RCC_GetPCLK1Freq() * 2) / (htimer2.Init.Prescaler + 1);
	timer2_cnt_res = 1/timer2_cnt_freq;
	user_signal_time_period = capture_difference * timer2_cnt_res;
	user_signal_freq = 1/user_signal_time_period;

	memset(usr_msg,0,sizeof(usr_msg));
	sprintf(usr_msg, "Frequency of the signal applied = %.2f Hz\r\n", user_signal_freq);
	HAL_UART_Transmit(&huart2, (uint8_t*)usr_msg, strlen(usr_msg), HAL_MAX_DELAY);

	input_captures[0] = 0;
	input_captures[1] = 0;

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
		Error_Handler();
  	}

}

void Error_Handler(void){
	while(1);
}
