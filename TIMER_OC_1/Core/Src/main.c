/*
 * main.c
 *
 *  Created on: 03 de Junho de 2020
 *      Author: Mateus Sousa
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "stm32f1xx_hal_tim.h"

void SystemClock_Config(uint8_t clock_freq);
void TIMER3_Init(void);
void GPIO_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer2;
UART_HandleTypeDef huart2;


/* TIM_CNT_CLK = 64MHz
 * Use the spreadsheet to find these values;
 * Put the desired frequency and the counter clock (pay attention in prescaler value);
 * Put the value of Pulse in tim3OC_init.Pulse; This value will be store in CCR1 register;
 * When the CCR1 value matches with counter timer, so the channel toggles;
 * For this, you must add the pulse value in CCR1 to make a square wave. */
volatile uint32_t pulse1_value = 640000; // 500Hz
volatile uint32_t pulse2_value = 32000;  // 1kHz
volatile uint32_t pulse3_value = 16000;  // 2kHz
volatile uint32_t pulse4_value = 8000;   // 4kHz

volatile uint32_t ccr_content;

int main(void){

	HAL_Init();
	SystemClock_Config(SYS_CLOCK_FREQ_64_MHZ);
	GPIO_Init();
	TIMER3_Init();

	if(HAL_TIM_OC_Start_IT(&htimer2, TIM_CHANNEL_1) != HAL_OK){
		Error_Handler();
	}

	if(HAL_TIM_OC_Start_IT(&htimer2, TIM_CHANNEL_2) != HAL_OK){
		Error_Handler();
	}

	if(HAL_TIM_OC_Start_IT(&htimer2, TIM_CHANNEL_3) != HAL_OK){
		Error_Handler();
	}

	if(HAL_TIM_OC_Start_IT(&htimer2, TIM_CHANNEL_4) != HAL_OK){
		Error_Handler();
	}

	while(1);

  	return 0;
}

void SystemClock_Config(uint8_t clock_freq){

	RCC_OscInitTypeDef osc_init;
	RCC_ClkInitTypeDef clk_init;

	uint32_t FLatency = 0;

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	osc_init.HSIState = RCC_HSI_ON;
	osc_init.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;

	switch(clock_freq){

		case SYS_CLOCK_FREQ_20_MHZ:

			osc_init.PLL.PLLMUL = RCC_CFGR_PLLMULL5;

			clk_init.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
				                     RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

			// flash memory latency
			FLatency = FLASH_LATENCY_0;

			break;

		case SYS_CLOCK_FREQ_48_MHZ:

			osc_init.PLL.PLLMUL = RCC_CFGR_PLLMULL12;

			clk_init.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
						         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

			// flash memory latency
			FLatency = FLASH_LATENCY_1;

			break;

		case SYS_CLOCK_FREQ_64_MHZ:

			osc_init.PLL.PLLMUL = RCC_CFGR_PLLMULL16;

			clk_init.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
								 RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

			// flash memory latency
			FLatency = FLASH_LATENCY_2;

			break;

		default:

			return;

	}

	if(HAL_RCC_OscConfig(&osc_init) != HAL_OK){
		Error_Handler();
	}

	if(HAL_RCC_ClockConfig(&clk_init, FLatency) != HAL_OK){
		Error_Handler();
	}

	//Systick configuration
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

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

void TIMER3_Init(void){

	TIM_OC_InitTypeDef tim3OC_init;

	htimer2.Instance = TIM2;
	htimer2.Init.Period = 0xFFFF;
	htimer2.Init.Prescaler = 0;

	if(HAL_TIM_OC_Init(&htimer2) != HAL_OK){
		Error_Handler();
	}

	memset(&tim3OC_init, 0, sizeof(tim3OC_init));	// Just in case

	tim3OC_init.OCMode = TIM_OCMODE_TOGGLE;
	tim3OC_init.OCNPolarity = TIM_OCPOLARITY_HIGH;
	tim3OC_init.Pulse = pulse1_value;

	if(HAL_TIM_OC_ConfigChannel(&htimer2, &tim3OC_init, TIM_CHANNEL_1) != HAL_OK){
		Error_Handler();
	}

	tim3OC_init.Pulse  = pulse2_value;
	if(HAL_TIM_OC_ConfigChannel(&htimer2, &tim3OC_init, TIM_CHANNEL_2) != HAL_OK){
		Error_Handler();
	}

	tim3OC_init.Pulse  = pulse3_value;
	if(HAL_TIM_OC_ConfigChannel(&htimer2, &tim3OC_init, TIM_CHANNEL_3) != HAL_OK){
		Error_Handler();
	}

	tim3OC_init.Pulse  = pulse4_value;
	if(HAL_TIM_OC_ConfigChannel(&htimer2, &tim3OC_init, TIM_CHANNEL_4) != HAL_OK){
		Error_Handler();
	}

}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){

	/* TIM3_CH1 toggling with frequency = 500 Hz */
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, ccr_content+pulse1_value);	//Adding value to next toggling in CCR1
	}

	/* TIM3_CH2 toggling with frequency = 1000 Hz */
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, ccr_content+pulse2_value);	//Adding value to next toggling in CCR1
	}

	/* TIM3_CH3 toggling with frequency = 2000 Hz */
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, ccr_content+pulse3_value);	//Adding value to next toggling in CCR1
	}

	/* TIM3_CH4 toggling with frequency = 4000 Hz */
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_4, ccr_content+pulse4_value);	//Adding value to next toggling in CCR1
	}
}

void Error_Handler(void){
	while(1);
}
