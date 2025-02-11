/*
 * it.c
 *
 *  Created on: 18 de Junho de 2020
 *      Author: Mateus Sousa
 */

#include "main.h"

extern TIM_HandleTypeDef htimer6;
extern UART_HandleTypeDef huart2;

void SysTick_Handler (void){
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM6_DAC_IRQHandler(void){
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_TIM_IRQHandler(&htimer6);
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
}

void USART2_IRQHandler(void){
	HAL_UART_IRQHandler(&huart2);
}
