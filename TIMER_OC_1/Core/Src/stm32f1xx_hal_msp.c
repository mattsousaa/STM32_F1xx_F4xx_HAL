/*
 * stm32f4xx_hal_msp.c
 *
 *  Created on: 03 de Junho de 2020
 *      Author: Mateus Sousa
 */

#include "main.h"
#include "stm32f1xx_hal_tim.h"

void HAL_MspInit(void){

	__HAL_RCC_PWR_CLK_ENABLE();

	/* DISABLE: JTAG-DP Disabled and SW-DP Disabled */
	//__HAL_AFIO_REMAP_SWJ_DISABLE();

	//1. Set up the priority grouping of the arm cortex mx processor
	// This line is not required because priority grouping will be 4 by default
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	//2. Enable the required system exceptions of the arm cortex mx processor
	SCB->SHCSR |= 0x7 << 16; //usage fault, memory fault and bus fault system exceptions

	//3. configure the priority for the system exceptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);

}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim){

	GPIO_InitTypeDef GPIO_InitStruct;

	//1. enable the peripheral clock for the timer2 peripheral
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//2. Configure gpios to behave as timer2 channel 1,2,3 and 4
	/**TIM2 GPIO Configuration
	   PA0    ------> TIM2_CH1
	   PA1    ------> TIM2_CH2
	   PA2    ------> TIM2_CH3
	   PA3    ------> TIM2_CH4
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//3. nvic settings
	HAL_NVIC_SetPriority(TIM2_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

}
