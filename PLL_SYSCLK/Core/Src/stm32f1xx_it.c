#include "main.h"
#include "stm32f1xx_it.h"

extern UART_HandleTypeDef huart2;

void SysTick_Handler(void){

	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();

}
