/*
 * it.c
 *
 *  Created on: 30 de Junho de 2020
 *      Author: Mateus Sousa
 */

#include "main.h"

extern UART_HandleTypeDef huart2;

void SysTick_Handler (void){
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
