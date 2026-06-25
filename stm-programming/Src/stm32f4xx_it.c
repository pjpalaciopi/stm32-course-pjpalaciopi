/*
 * stm32f4xx_it.c
 *
 *  Created on: Jun 12, 2026
 *      Author: pedro
 */

/*
 * stm32f4xx_it.c
 * Interrupt service routines
 * Author: pedro
 */

#include "stm32f4xx_hal.h"

/* Declare the TIM3 handle — defined in main.c */
extern TIM_HandleTypeDef htim3;
/* Declare the ADC handle — defined in main.c */
extern ADC_HandleTypeDef hadc1;
/* Declare the USART2 handle — defined in main.c */
extern UART_HandleTypeDef huart2;


/* SysTick handler — required by HAL for HAL_Delay() and timeouts */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/* TIM3 update event handler */
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

/* ADC update event handler */
void ADC_IRQHandler(void)
{
	HAL_ADC_IRQHandler(&hadc1);
}

/* USART2 handler used for reception */
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}
