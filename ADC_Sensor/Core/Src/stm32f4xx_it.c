/*
 * it.c
 *
 *  Created on: 21 de Junho, 2020
 *      Author: Mateus Sousa
 */

#include "main.h"

extern UART_HandleTypeDef huart2;

void SysTick_Handler(void){

	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();

}
