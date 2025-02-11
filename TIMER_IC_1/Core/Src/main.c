#include <string.h>
#include <stdio.h>
#include "main.h"
#include "stm32f1xx_hal_tim.h"

void SystemClock_Config(uint8_t clock_freq);
void MX_USART2_UART_Init(void);
void Frequency_Calculation(void);
void TIMER2_Init(void);
void TIMER3_Init(void);
void GPIO_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer2;
TIM_HandleTypeDef htimer3;

UART_HandleTypeDef huart2;

uint32_t input_captures[2] = {0};
volatile uint8_t count = 1;
volatile uint8_t is_capture_done = FALSE;

char usr_msg[100];

int main(void){

	HAL_Init();
	SystemClock_Config(SYS_CLOCK_FREQ_64_MHZ);
	GPIO_Init();
	TIMER2_Init();
	TIMER3_Init();

	MX_USART2_UART_Init();

	//HAL_TIM_Base_Start_IT(&htimer3); // You can use this signal in PA1

	if(HAL_TIM_IC_Start_IT(&htimer2, TIM_CHANNEL_2) != HAL_OK){
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

	//You can generate a clock with MCO pins from different sources
	//HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);

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

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){

	if(! is_capture_done){
		if(count == 1){
			// Get the TIM Capture Compare Register value on runtime (rising edge)
			input_captures[0] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
			count++;
		} else if (count == 2){
			// Get the TIM Capture Compare Register value on runtime (rising edge)
			input_captures[1] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
			Frequency_Calculation();	// Compute the input frequency signal
			count = 1;
			is_capture_done = FALSE;
		}
	}
}

void TIMER2_Init(void){

	TIM_IC_InitTypeDef sConfigIC;

	htimer2.Instance = TIM2;
	htimer2.Init.CounterMode = TIM_COUNTERMODE_UP;	// Timer up count
	htimer2.Init.Period = 0xFFFF;					// TIM2 16 bits
	htimer2.Init.Prescaler = 1;						// CNT_CLK = TIMx_CLK/(prescaler + 1)

	if(HAL_TIM_IC_Init(&htimer2) != HAL_OK){
		Error_Handler();
	}

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;		// Capture in rising edge of the clock
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;			// Timer in direct mode
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;						// Prescaler 1
	sConfigIC.ICFilter = 0;

	// Enter an input signal on PA1 or use other timer as clock source (Digital clock generator on Proteus)
	if(HAL_TIM_IC_ConfigChannel(&htimer2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK){
	    Error_Handler();
	}

}

void TIMER3_Init(void){

	/* Create a time base for 10us with SYSCLK = 64MHz */
	htimer3.Instance = TIM3;
	htimer3.Init.Prescaler = 15;
	htimer3.Init.Period = 40-1;	// The update event happens after one time gap or one time period

	if(HAL_TIM_Base_Init(&htimer3) != HAL_OK){
		Error_Handler();
	}

}

void Frequency_Calculation(void){

	uint32_t capture_difference = 0;
	double timer2_cnt_freq = 0;
	double timer2_cnt_res = 0;
	double user_signal_time_period = 0;
	double user_signal_freq = 0;

	if(input_captures[1] > input_captures[0])
		capture_difference = input_captures[1] - input_captures[0];
	else
		capture_difference = (0xFFFF - input_captures[0]) + input_captures[1];

	// CLK_TIM2 = 2*PCLK1 because RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2 (see clock tree)
	timer2_cnt_freq = (HAL_RCC_GetPCLK1Freq() * 2) / (htimer2.Init.Prescaler + 1);
	timer2_cnt_res = 1/timer2_cnt_freq; // Period of CLK_TIM2
	user_signal_time_period = capture_difference * timer2_cnt_res; // Period of input signal on PA1
	user_signal_freq = 1/user_signal_time_period; // Desired frequency

	memset(usr_msg,0,sizeof(usr_msg));
	// Put the command "-u _printf_float" in MCU GCC Linker (Miscellaneous)
	sprintf(usr_msg, "Frequency of the signal applied = %.2f Hz\r\n", user_signal_freq);
	HAL_UART_Transmit(&huart2, (uint8_t*)usr_msg, strlen(usr_msg), HAL_MAX_DELAY);

	input_captures[0] = 0;
	input_captures[1] = 0;

}

void MX_USART2_UART_Init(void){

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX;

	if(HAL_UART_Init(&huart2) != HAL_OK){
		Error_Handler();
  	}

}

void Error_Handler(void){
	while(1);
}
