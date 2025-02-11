#include "main.h"
#include "stm32f1xx_it.h"

extern TIM_HandleTypeDef htimer2;
extern TIM_HandleTypeDef htimer3;

void SysTick_Handler(void){

	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();

}

void TIM2_IRQHandler(void){

	HAL_TIM_IRQHandler(&htimer2);
}

void TIM3_IRQHandler(void){

	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);	// 50KHz square wave
	HAL_TIM_IRQHandler(&htimer3);

}
