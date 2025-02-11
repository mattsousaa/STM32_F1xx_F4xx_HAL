/*
 * main.c
 *
 *  Created on: 21 de Junho, 2020
 *      Author: Mateus Sousa
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "stm32f4xx_hal.h"

void SystemClockConfig(void);
void ADC1_Init(void);
void UART2_Init(void);
void Error_Handler(void);

UART_HandleTypeDef huart2;
ADC_HandleTypeDef hadc1;

int main(void){

	HAL_Init();
	SystemClockConfig();

	UART2_Init();

	ADC1_Init();

	HAL_ADC_Start_IT(&hadc1);

	while(1);

	return 0;

}

void SystemClockConfig(void){

}

/* ADC1 init function */
void ADC1_Init(void){

	ADC_ChannelConfTypeDef sConfig = {0};

	/* Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;	/* The clock of the analog part of the ADC is the half of the PCLK frequency*/
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;	/* MAX Resolution of ADC Channel */
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;	/* We can repeatedly poll for a conversion without stopping and then restarting the ADC */
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;	/* Specifies the data align of the converted result. ADC data register is implemented as half-word register. */
	hadc1.Init.NbrOfConversion = 1;	/* Meaningless. Single conversion mode automatically assumes that the number of sampled channels is equal to 1. */
	hadc1.Init.DMAContinuousRequests = DISABLE;	/* No DMA for now */
	hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;	/* End Of Conversion (EOC) for continuous conversion */

	if(HAL_ADC_Init(&hadc1) != HAL_OK){
		Error_Handler();
	}

	/* Configure for the selected ADC regular channel its corresponding rank in the sequence and its sample time */
	sConfig.Channel = ADC_CHANNEL_1;	/* PA1 pin */
	sConfig.Rank = 1;
	/* FCLK = 16MHz
	 * ADCCLK = 8MHz
	 * T_ADCCLK = 12.5us
	 * ADC_SAMPLETIME_3CYCLES
	 * We have that an A/D conversion is performed every 37.5μs*/
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

	/* Put reference to ADC Channel with VREF+ or VREF- */
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	char msg[20];
	uint16_t rawValue;
	double temp;

	rawValue = HAL_ADC_GetValue(&hadc1);	// Get ADC raw value after conversion
	temp = ((float) rawValue) / 4095 * 3300;	// Just simulating a temperature sensor with a formula
	temp = ((temp - 760.0) / 2.5) + 25;

	sprintf(msg, "rawValue: %hu\r\n", rawValue);	// Print raw value in UART
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	// Put the command "-u _printf_float" in MCU GCC Linker (Miscellaneous)
	sprintf(msg, "Temperature: %.2f\r\n", temp);// Print temperature value in UART
	HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
}

/* High level initialization */
void UART2_Init(void){

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX;

	if(HAL_UART_Init(&huart2) != HAL_OK){
		//There is a problem
		Error_Handler();
	}
}

void Error_Handler(void){
	while(1);
}
