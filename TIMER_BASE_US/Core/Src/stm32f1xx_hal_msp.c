#include "main.h"
#include "stm32f1xx_hal_tim.h"

//Uncomment the #define HAL_TIM_MODULE_ENABLED in stm32f1xx_hal_conf.h
#include "stm32f1xx_hal_conf.h"

void HAL_MspInit(void){

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

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htimer){

	//1. enable the clock for the TIM6 peripheral
	__HAL_RCC_TIM3_CLK_ENABLE();

	//2. Enable the IRQ of TIM6
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	//3. setup the priority for TIM6_DAC_IRQn
	HAL_NVIC_SetPriority(TIM3_IRQn, 15, 0);

}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart){

	GPIO_InitTypeDef gpio_uart;
	//here we are going to do the low level inits of the USART2 peripheral

	//1. enable the clock for the USART2 peripheral as well as for GPIOA and AFIO peripheral
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

	//2. Do the pin muxing configurations
	gpio_uart.Pin = GPIO_PIN_2; //UART2_TX
	gpio_uart.Mode = GPIO_MODE_AF_PP;
	gpio_uart.Pull = GPIO_PULLUP;
	gpio_uart.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOA, &gpio_uart);

	gpio_uart.Pin = GPIO_PIN_3;
	//gpio_uart.Mode = GPIO_MODE_INPUT;
	HAL_GPIO_Init(GPIOA, &gpio_uart);

	//3. Enable the IRQ and set up the priority (NVIC settings)
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);

}
