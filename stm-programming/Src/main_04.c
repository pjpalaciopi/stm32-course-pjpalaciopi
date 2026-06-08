/*
 * main_04.c
 *
 *  Created on: May 25, 2026
 *      Author: pedro
 */
#include <stdint.h>
#include "stm32f411xe.h"
#include "stm32f4xx.h"
#define FREQUENCY 2000000

// Headers
void TIM3_IRQHandler(void);

int main(void){

	/* Enable GPIOA clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	/* Enable GPIOB clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	/* Enable TIM3 clock*/
	RCC->APB1ENR &= ~(RCC_APB1ENR_TIM3EN);
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	/* Configure PA5 MODER as output*/
	GPIOA->MODER |= (0b01 << GPIO_MODER_MODE5_Pos);
	GPIOA->OTYPER &= ~(0b1 << GPIO_OTYPER_OT5_Pos);				// push-pull
	GPIOA->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED5_Pos);		// fast
	GPIOA->ODR |= (0b1 << GPIO_ODR_OD5_Pos);					// initial value 1

	/* Configure PA11 as output*/
	GPIOA->MODER |= (0b01 << GPIO_MODER_MODE11_Pos);
	GPIOA->OTYPER &= ~(0b1 << GPIO_OTYPER_OT11_Pos);			// push-pull
	GPIOA->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED11_Pos);		// fast
	GPIOA->ODR |= (0b1 << GPIO_ODR_OD11_Pos);					// initial value 1

	/* Configure PB11 MODER as output*/
	GPIOB->MODER |= (0b01 << GPIO_MODER_MODE12_Pos);
	GPIOB->OTYPER &= ~(0b1 << GPIO_OTYPER_OT12_Pos);			// push-pull
	GPIOB->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED12_Pos);		// fast
	GPIOB->ODR |= (0b1 << GPIO_ODR_OD12_Pos);					// initial value 1

	/*
	 * Relevant register for the timer to simply count time
	 * (TIMx_CR1)		Enables timer						bits 0, 4, 7
	 * (TIMx_DIER)		Enables interruptions				bits 0
	 * (TIMx_SR)		Controls the interruption flags		bits 0
	 * (TIMx_CNT)		Sets counter						16 or 32 bit number
	 * (TIMx_PSC)		Sets prescaler						16 bit number
	 * (TIMx_ARR)		sets autorreload					16 or 32 bit number
	 * */
	TIM3->ARR = (3250 - 1);				// set autorreload to 3250 * 0.1 ms = 325 ms
	TIM3->PSC = (1600 -1);				// set prescaler to 0.1 ms
	TIM3->CNT = 0;						// reset counter
	TIM3->SR &= ~(TIM_SR_UIF);			// clean interruption flag
	TIM3->DIER &= ~(TIM_DIER_UIE);		// clean interruption enable
	TIM3->DIER |= TIM_DIER_UIE;			// set interruption enable
	/* The NVIC must know that an interrupt from the TIM3 is enabled */
	__NVIC_EnableIRQ(TIM3_IRQn);		// found in the /Includes/.../Core/Include/core_cm4.h file

	TIM3->CR1 &= ~(TIM_CR1_DIR);		// set as upcounter
	TIM3->CR1 &= ~(TIM_CR1_ARPE);		// clean arr preload
	TIM3->CR1 |= (TIM_CR1_ARPE);		// set arr preload
	TIM3->CR1 &= ~(TIM_CR1_CEN);		// clean timer enable
	TIM3->CR1 |= (TIM_CR1_CEN);			// set timer enable

	while(1){
//		for (uint32_t i = 0; i < FREQUENCY; i++){
//			/* when the for has run for half its length toggle the PB3 */
//			if (i == FREQUENCY/2){
//				GPIOB->ODR ^= (0b1 << GPIO_ODR_OD12_Pos);
//			}
//		}
//		GPIOB->ODR ^= (0b1 << GPIO_ODR_OD12_Pos);
//		/* The ^= (xor) flips the bit value of the ODR register at the position of the PA5 */
//		GPIOA->ODR ^= (0b1 << GPIO_ODR_OD5_Pos);
	}
}

/*
 * The ISR for the interruption found in the startup folder
 * Usually a function that returns void and with void arguments
 * */
void TIM3_IRQHandler(void){
	// verify which flag is up for the interruption
	if (TIM3->SR && TIM_SR_UIF){
		GPIOA->ODR ^= GPIO_ODR_OD5;		// toggle led
		TIM3->SR &= ~(TIM_SR_UIF);		// clean flag
	}
}
