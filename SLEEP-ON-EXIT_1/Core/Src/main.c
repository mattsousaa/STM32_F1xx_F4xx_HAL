/*
 * main.c
 *
 *  Created on: 18 de Junho de 2020
 *      Author: Mateus Sousa
 */

#include <string.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "main.h"

void GPIO_Init(void);
void Error_handler(void);
void TIMER6_Init(void);
void UART2_Init(void);
void SystemClock_Config_HSE(uint8_t clock_freq);
void GPIO_AnalogConfig(void);

TIM_HandleTypeDef htimer6;
UART_HandleTypeDef huart2;
extern uint8_t some_data[];

/*
 * Tips to reduce the Power Consumption
 * UART2, TIM6, GPIO
 * =======================================
 * ...........(HCLK: 50MHz PLL)
 * =======================================
 * With out SLEEPONEXIT/WFI/WFE
 * Current consumption: 11mA
 *
 * With SLEEPONEXIT/WFI/WFE
 * Current consumption: 7.5mA
 *
 * ========================================================
 * ...........(HCLK: 16MHz HSI) + UART2 Baudrate of 115200
 * ========================================================
 * With SLEEPONEXIT/WFI/WFE
 * Current consumption: 3mA
 *
 * =====================================================
 * ...........(HCLK: 16MHz HSI) + UART2 Baudrate 230400
 * =====================================================
 * With SLEEPONEXIT/WFI/WFE
 * Current consumption: 2.45mA
 *
 * =====================================================
 * ...........(HCLK: 16MHz HSI) + UART2 Baudrate 460800
 * =====================================================
 * With SLEEPONEXIT/WFI/WFE
 * Current consumption: 2.35mA
 *
 * ===========================================================================
 * ...........(HCLK: 16MHz HSI) +
 * ........... UART2 Baudrate 460800 +
 * ........... Disabling all peripherals of AHBx and APBx domain during sleep
 * ===========================================================================
 * With SLEEPONEXIT/WFI/WFE
 * Current consumption: 2.06mA
 *
 * ===========================================================================
 * ...........(HCLK: 16MHz HSI) +
 * ........... UART2 Baudrate 460800 +
 * ........... Disabling all peripherals of AHBx and APBx domain during sleep +
 * ........... I/O analog mode
 * ===========================================================================
 * With SLEEPONEXIT/WFI/WFE
 * Current consumption: 1.86mA
 *
 * How long your battery last?
 * Let's say you are using 9V battery for this application.
 * 9V battery typically comes with AH(Ampere-hour) rating of 500mAH.
 * i.e: The battery can give 500mA of current continuously for 1 Hr before going dead.
 *
 * Case 1: 50MHz PLL, No sleep mode
 * Avg. current consumption: 11mA
 * Application life = 500mA/11mA = 45Hrs (~2 days)
 *
 * Case 2: 16MHz, No PLL, with sleep mode and other settings
 * Avg. current consumption: 1.86mA
 * Application life = 500mA/1.86mA = 269Hrs (~11 days)
 *
 * */

int main(void){

	HAL_Init();

	//SystemClock_Config_HSE(SYS_CLOCK_FREQ_50_MHZ);

	GPIO_Init();

	//HAL_SuspendTick();

	UART2_Init();

	TIMER6_Init();

	GPIO_AnalogConfig();

	//SCB->SCR |= (1 << 1); // See on Generic user guide
	HAL_PWR_EnableSleepOnExit(); // Enter sleep mode, on return from an ISR.

	/* lets start with fresh Status register of Timer to avoid any spurious interrupts */
    TIM6->SR = 0;

	//Lets start the timer in interrupt mode
	HAL_TIM_Base_Start_IT(&htimer6);

	while(1);

	return 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config_HSE(uint8_t clock_freq){

	RCC_OscInitTypeDef Osc_Init;
	RCC_ClkInitTypeDef Clock_Init;
    uint8_t flash_latency=0;

	Osc_Init.OscillatorType = RCC_OSCILLATORTYPE_HSE ;
	Osc_Init.HSEState = RCC_HSE_ON;
	Osc_Init.PLL.PLLState = RCC_PLL_ON;
	Osc_Init.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	switch(clock_freq){

	case SYS_CLOCK_FREQ_50_MHZ:

		Osc_Init.PLL.PLLM = 4;
		Osc_Init.PLL.PLLN = 50;
		Osc_Init.PLL.PLLP = RCC_PLLP_DIV2;
		Osc_Init.PLL.PLLQ = 2;
		Osc_Init.PLL.PLLR = 2;
		Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		Clock_Init.APB1CLKDivider = RCC_HCLK_DIV2;
		Clock_Init.APB2CLKDivider = RCC_HCLK_DIV1;
		flash_latency = 1;

	    break;

	case SYS_CLOCK_FREQ_84_MHZ:

		Osc_Init.PLL.PLLM = 4;
		Osc_Init.PLL.PLLN = 84;
		Osc_Init.PLL.PLLP = RCC_PLLP_DIV2;
		Osc_Init.PLL.PLLQ = 2;
		Osc_Init.PLL.PLLR = 2;
		Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                           |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		Clock_Init.APB1CLKDivider = RCC_HCLK_DIV2;
		Clock_Init.APB2CLKDivider = RCC_HCLK_DIV1;
        flash_latency = 2;

	    break;

	case SYS_CLOCK_FREQ_120_MHZ:
		Osc_Init.PLL.PLLM = 4;
		Osc_Init.PLL.PLLN = 120;
		Osc_Init.PLL.PLLP = RCC_PLLP_DIV2;
		Osc_Init.PLL.PLLQ = 2;
		Osc_Init.PLL.PLLR = 2;
		Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                           |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
		Clock_Init.APB1CLKDivider = RCC_HCLK_DIV4;
		Clock_Init.APB2CLKDivider = RCC_HCLK_DIV2;
        flash_latency = 3;

	    break;

	default:

		return ;
	}

	if(HAL_RCC_OscConfig(&Osc_Init) != HAL_OK){
		Error_handler();
	}

	if(HAL_RCC_ClockConfig(&Clock_Init, flash_latency) != HAL_OK){
		Error_handler();
	}

	/*Configure the systick timer interrupt frequency (for every 1 ms) */
	uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
	HAL_SYSTICK_Config(hclk_freq/1000);

	/**Configure the Systick*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

}

void GPIO_AnalogConfig(void){

	GPIO_InitTypeDef GpioA;

	uint32_t gpio_pins = GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_4 | 		\
						 GPIO_PIN_5 | GPIO_PIN_6 |GPIO_PIN_7 |		\
						 GPIO_PIN_8 | GPIO_PIN_9 |GPIO_PIN_10 |		\
						 GPIO_PIN_11 | GPIO_PIN_12 |GPIO_PIN_13 | 	\
						 GPIO_PIN_14 | GPIO_PIN_15;

	GpioA.Pin = gpio_pins;
	GpioA.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &GpioA);
}

void GPIO_Init(void){

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef ledgpio ;
	ledgpio.Pin = GPIO_PIN_5;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &ledgpio);

	ledgpio.Pin = GPIO_PIN_12;
	ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
	ledgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &ledgpio);

}

void UART2_Init(void){

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 921600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX;

	if(HAL_UART_Init(&huart2) != HAL_OK){
		//There is a problem
		Error_handler();
	}
}

void TIMER6_Init(void){

	htimer6.Instance = TIM6;
	htimer6.Init.Prescaler = 4999;
	htimer6.Init.Period = 100-1;

	if(HAL_TIM_Base_Init(&htimer6) != HAL_OK){
		Error_handler();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if(HAL_UART_Transmit(&huart2,(uint8_t*)some_data,(uint16_t)strlen((char*)some_data),HAL_MAX_DELAY) != HAL_OK){
		Error_handler();
	 }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12, GPIO_PIN_RESET);
}

void Error_handler(void){
	while(1);
}
