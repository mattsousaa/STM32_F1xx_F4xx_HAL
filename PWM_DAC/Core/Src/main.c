#include <string.h>
#include <stdio.h>
#include <math.h>
#include "main.h"
#include "stm32f1xx_hal_tim.h"

#define PI		3.14159
#define ASR 	1.8 	//360/200 = 1.8

void SystemClock_Config(uint8_t clock_freq);
void TIMER2_Init(void);
void GPIO_Init(void);
void Error_Handler(void);

TIM_HandleTypeDef htimer2;
UART_HandleTypeDef huart2;

int main(void){

	uint16_t IV[200];	// vector containing the Pulse values used to generate the sine wave (which corresponds to the output voltage levels)
	float angle;

	HAL_Init();
	SystemClock_Config(SYS_CLOCK_FREQ_48_MHZ);
	GPIO_Init();
	TIMER2_Init();

	if(HAL_TIM_PWM_Start(&htimer2, TIM_CHANNEL_2) != HAL_OK){
		Error_Handler();
	}

	/*
	 * By varying the output voltage (which implies that we vary the duty cycle) we can generate an
	 * arbitrary output waveform, whose frequency is a fraction of the PWM period. The basic idea here
	 * is to divide the waveform we want, for example a sine wave, into ‘x’ number of divisions. For each
	 * division we have a single PWM cycle. The T_ON time (that is, the duty cycle) directly corresponds to
	 * the amplitude of the waveform in that division, which is calculated using sin() function;
	 *
	 * For example, a sine wave can be divided in 10 steps within various cycles of a PWM signal. In this way,
	 * we will require 10 different PWM pulses increasing/decreasing in sinusoidal manner. A PWM pulse with
	 * 0% duty cycle will represent the min amplitude (0V), the one with 100% duty cycle will represent max
	 * amplitude(3.3V). Since out PWM pulse has voltage swing between 0V to 3.3V, our sine wave will swing
	 * between 0V to 3.3V too;
	 *
	 * It takes 360 degrees for a sine wave to complete one cycle. Hence for 10 divisions we will need to
	 * increase the angle in steps of 36 degrees. This is called the Angle Step Rate or Angle Resolution. We
	 * can increase the number of divisions to get more accurate waveform. But as divisions increase we
	 * also need to increase the resolution, which implies that we have to increase the frequency of the
	 * timer used to generate the PWM signal (the faster runs the timer the smaller is the period);
	 *
	 * Usually 200 divisions are a good approximation for an output wave. This means that if we want to
	 * generate a 50Hz sine wave, we need to run the timer at a 50Hz*200 = 10kHz (Time base required = 0.0001).
	 * The pulse period will be equal to 200 (the number of steps - this means that we vary the output voltage by
	 * 3.3V/200=0.016V), and so the Prescaler value will be (assuming an STM32F103xx MCU running at 48MHz):
	 *
	 * Prescaler = 48MHz/(50Hz * 200_divisions * 200_pulses) = 24
	 *
	 * The following example shows how to generate a 50Hz pure sine wave in an STM32F103xx MCU running at 48MHz.
	 *
	 * */

	while(1){
		for(uint8_t i = 0; i < 200; i++){
			angle = ASR*(float)i;
			/*
			 * The C sinf() returns the sine of the given angle expressed in radians. So we need to convert the angular
			 * expresses in degrees to radians using the formula:
			 * Radians = (pi/180) * Degrees
			 * However, in our case we have divided the sine wave cycle in 200 steps (that is, we have divided the
			 * circumference in 200 steps), so we need to compute the value in radians of each step. But since sine
			 * gives negative values for angle between 180° and 360° we need to scale it, since PWM output values
			 * cannot be negative.
			 * */
			IV[i] = (uint16_t) rint(100 + 99 * sinf(angle * (PI / 180)));

			__HAL_TIM_SET_COMPARE(&htimer2, TIM_CHANNEL_2, IV[i]);

			/*
			 * Here, I have used a 100ohm resistor and a 10μF capacitor, which give a cut-off frequency
			 * of ~159Hz and a Vpp equal to 0.08V.
			 * */
		}
	}

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

void TIMER2_Init(void){

	TIM_OC_InitTypeDef tim2PWM_Config;
	/* First of all you have to decide a time period of the signal adjusting the period and prescaler;
	 * Enter with time clock frequency of microcontroller and time base required;
	 * Adjust the prescaler if you need;
	 * In this example, I decided to make a time period of 1ms as you can se in the configurations below;
	 * I'm using a time clock frequency of 64MHz;
	 * If you want to increase the resolution (time gap) of the counter clock, just decrease the prescaler value */
	htimer2.Instance = TIM2;
	htimer2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htimer2.Init.Prescaler = 23;
	htimer2.Init.Period = 200-1;
	htimer2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;

	if(HAL_TIM_PWM_Init(&htimer2) != HAL_OK){
		Error_Handler();
	}

	memset(&tim2PWM_Config, 0, sizeof(tim2PWM_Config)); // Just in case

	/* The value of duty cycle can be adjusted by storing the % value of the period in pulse value;
	 * When the CCR1 register matches with the value stored in ARR, then it just toggled;
	 * The rest of the period will be low or high;
	 * What makes the time on or off is the polarity;
	 * In STM32F1xx this is in reverse and I don't know why;
	 * The normal like in STM32F4xx would be with polarity high, we had the active duty cycle within a period of clock;
	 * Here is inverse, but just change the polarity and everthing will looks good;
	 * */
	tim2PWM_Config.OCMode = TIM_OCMODE_PWM1;
	tim2PWM_Config.OCPolarity = TIM_OCPOLARITY_LOW;
	tim2PWM_Config.Pulse = 0;							// Start with 0% of duty cycle

	if(HAL_TIM_PWM_ConfigChannel(&htimer2, &tim2PWM_Config, TIM_CHANNEL_2) != HAL_OK){
		Error_Handler();
	}
}

void Error_Handler(void){
	while(1);
}
