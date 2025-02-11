#include "main.h"
#include "stm32f1xx_hal_tim.h"

void SystemClockConfig(void);
void TIMER3_Init(void);
void GPIO_Init(void);
void Error_Handler(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/* TIMER3 can be used for basic timer
 * See STM32 cross-series timer overview document for more details */
TIM_HandleTypeDef htimer3;

int main(void){

	HAL_Init();
	SystemClockConfig();
	GPIO_Init();
	TIMER3_Init();

	//Lets start timer in IT mode
	HAL_TIM_Base_Start_IT(&htimer3);

	while(1);

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);

}

void SystemClockConfig(void){


}

void GPIO_Init(void){

	GPIO_InitTypeDef ledgpio;

	/* GPIOA clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* High Level GPIO Initialization */
	ledgpio.Pin = GPIO_PIN_2;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	ledgpio.Speed = GPIO_SPEED_FREQ_LOW;

	/* Init GPIO */
	HAL_GPIO_Init(GPIOA, &ledgpio);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}

void TIMER3_Init(void){

	/* Verify the spreadsheet "timer_period_calculation" to adjust Prescaler and Period values;
	 * Prescaler value is stored in TIMx_PSC register (16 bits);
	 * Period value is stored in TIMx_ARR register (16 bits - MAX 65535);
	 * Choose a clock value for your microcontroller in the spreadsheet (TIMx_CLK);
	 * Choose a time base required in seconds in the spreadsheet;
	 * Check if the period exceeded the maximum value of TIMx_ARR register (16 bits - MAX 65535);
	 * If yes, increase or decrease the prescaler in the spreedsheet until 0 < TIMx_ARR <= 65535;
	 * This process will find how many ticks there exists for every period of clock;
	 * For instance, if TIMx_CLK = 16MHz and prescaler = 0, so for every 0.0625us a tick happens;
	 * In this case, what is the period value must be configured to get the time base of 100ms?;
	 * This math results in 1600000 and should be placed in TIMx_ARR register, but this value is greater than 65535;
	 * That's why you should increase or decrease the prescaler value.
	 * */

	/* Create a time base for 1s with SYSCLK = 8MHz */
	htimer3.Instance = TIM3;
	htimer3.Init.Prescaler = 300;
	htimer3.Init.Period = 26578-1;	// The update event happens after one time gap or one time period

	if(HAL_TIM_Base_Init(&htimer3) != HAL_OK){
		Error_Handler();
	}

}


void Error_Handler(void){
	while(1);
}
