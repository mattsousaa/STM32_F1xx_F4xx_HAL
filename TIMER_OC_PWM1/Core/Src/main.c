#include <string.h>
#include <stdio.h>
#include "main.h"
#include "stm32f1xx_hal_tim.h"

void SystemClock_Config(uint8_t clock_freq);
void TIMER2_Init(void);
void GPIO_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer2;
UART_HandleTypeDef huart2;

int main(void){

	HAL_Init();
	SystemClock_Config(SYS_CLOCK_FREQ_64_MHZ);
	GPIO_Init();
	TIMER2_Init();

	if(HAL_TIM_PWM_Start(&htimer2, TIM_CHANNEL_1) != HAL_OK){
		Error_Handler();
	}

	if(HAL_TIM_PWM_Start(&htimer2, TIM_CHANNEL_2) != HAL_OK){
		Error_Handler();
	}

	if(HAL_TIM_PWM_Start(&htimer2, TIM_CHANNEL_3) != HAL_OK){
		Error_Handler();
	}

	if(HAL_TIM_PWM_Start(&htimer2, TIM_CHANNEL_4) != HAL_OK){
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

void TIMER2_Init(void){

	TIM_OC_InitTypeDef tim2PWM_Config;
	/* First of all you have to decide a time period of the signal adjusting the period and prescaler;
	 * Enter with time clock frequency of microcontroller and time base required;
	 * Adjust the prescaler if you need;
	 * In this example, I decided to make a time period of 10ms as you can se in the configurations below;
	 * I'm using a time clock frequency of 64MHz;
	 * If you want to increase the resolution (time gap) of the counter clock, just decrease the prescaler value */
	htimer2.Instance = TIM2;
	htimer2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer2.Init.Prescaler = 4999;
	htimer2.Init.Period = 128-1;

	if(HAL_TIM_PWM_Init(&htimer2) != HAL_OK){
		Error_Handler();
	}

	memset(&tim2PWM_Config, 0, sizeof(tim2PWM_Config)); // Just in case

	/* The value of duty cycle can be adjusted by storing the % value of the period in pulse value;
	 * When the CCR1 register matches with the value stored in ARR, then it just toggled;
	 * The rest of the period will be low or high;
	 * What makes the time on or off is the polarity;
	 * In STM32F1xx this is in reverse and I don't know why;
	 * The normal like in STM32F4xx would be with polarity high, we had the active duty cycle within a period of clock;
	 * Here is inverse, but just change the polarity and everthing will looks good;
	 * */
	tim2PWM_Config.OCMode = TIM_OCMODE_PWM1;
	tim2PWM_Config.OCPolarity = TIM_OCPOLARITY_LOW;
	tim2PWM_Config.Pulse = (htimer2.Init.Period * 25)/100;		// Generate 25% of duty cycle

	if(HAL_TIM_PWM_ConfigChannel(&htimer2, &tim2PWM_Config, TIM_CHANNEL_1) != HAL_OK){
		Error_Handler();
	}

	tim2PWM_Config.Pulse = (htimer2.Init.Period * 45)/100;		// Generate 45% of duty cycle
	if(HAL_TIM_PWM_ConfigChannel(&htimer2, &tim2PWM_Config, TIM_CHANNEL_2) != HAL_OK){
		Error_Handler();
	}

	tim2PWM_Config.Pulse = (htimer2.Init.Period * 75)/100;		// Generate 75% of duty cycle
	if(HAL_TIM_PWM_ConfigChannel(&htimer2, &tim2PWM_Config, TIM_CHANNEL_3) != HAL_OK){
		Error_Handler();
	}

	tim2PWM_Config.Pulse = (htimer2.Init.Period * 95)/100;		// Generate 95% of duty cycle
	if(HAL_TIM_PWM_ConfigChannel(&htimer2, &tim2PWM_Config, TIM_CHANNEL_4) != HAL_OK){
		Error_Handler();
	}

}

void Error_Handler(void){
	while(1);
}
