/*
 * main_05.c
 *
 *  Created on: May 29, 2026
 *      Author: pedro
 */
#include <stdint.h>
#include "stm32f411xe.h"
#include "stm32f4xx.h"

typedef enum{
	BLUE,
	RED,
	GREEN
}states_t;

// variables
states_t state = GREEN;

// headers
void configuration(void);
void changeState(void);
void TIM3_IRQHandler(void);

int main(void){
	configuration();

	while(1){

	}
}

void configuration(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	RCC->APB1ENR &= ~(RCC_APB1ENR_TIM3EN);
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	// configure PC8
	GPIOC->MODER |= (0b01 << GPIO_MODER_MODE8_Pos);
	GPIOC->OTYPER &= ~(0b1 << GPIO_OTYPER_OT8_Pos);				// push-pull
	GPIOC->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED8_Pos);		// fast
	GPIOC->ODR |= (0b1 << GPIO_ODR_OD8_Pos);					// initial value 1

	// configure PC6
	GPIOC->MODER |= (0b01 << GPIO_MODER_MODE6_Pos);
	GPIOC->OTYPER &= ~(0b1 << GPIO_OTYPER_OT6_Pos);				// push-pull
	GPIOC->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED6_Pos);		// fast
	GPIOC->ODR |= (0b1 << GPIO_ODR_OD6_Pos);					// initial value 1

	// configure PC5
	GPIOC->MODER |= (0b01 << GPIO_MODER_MODE5_Pos);
	GPIOC->OTYPER &= ~(0b1 << GPIO_OTYPER_OT5_Pos);				// push-pull
	GPIOC->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED5_Pos);		// fast
	GPIOC->ODR |= (0b1 << GPIO_ODR_OD5_Pos);					// initial value 1

	// configure TIM3
	TIM3->ARR = (30000 - 1);				// set autorreload to 3250 * 0.1 ms = 325 ms
	TIM3->PSC = (1600 -1);					// set prescaler to 0.1 ms
	TIM3->CNT = 0;							// reset counter
	TIM3->SR &= ~(TIM_SR_UIF);				// clear interruption flag
	TIM3->DIER &= ~(TIM_DIER_UIE);			// clear interruption enable
	TIM3->DIER |= TIM_DIER_UIE;				// set interruption enable
	/* The NVIC must know that an interrupt from the TIM3 is enabled */
	__NVIC_EnableIRQ(TIM3_IRQn);		// found in the /Includes/.../Core/Include/core_cm4.h file

	TIM3->CR1 &= ~(TIM_CR1_DIR);		// set as upcounter
	TIM3->CR1 &= ~(TIM_CR1_ARPE);		// clean arr preload
	TIM3->CR1 |= (TIM_CR1_ARPE);		// set arr preload
	TIM3->CR1 &= ~(TIM_CR1_CEN);		// clear timer enable
	TIM3->CR1 |= (TIM_CR1_CEN);			// set timer enable
}
/* The IRQ are the startup file */
void TIM3_IRQHandler(void){
	// verify which flag is up for the interruption
	if (TIM3->SR && TIM_SR_UIF){
		changeState();					// change the states of the stop light
		TIM3->SR &= ~(TIM_SR_UIF);		// clear flag
	}
}

void changeState(void){
	switch (state) {
		case GREEN:
			GPIOC->ODR |= GPIO_ODR_OD8;
			GPIOC->ODR &= ~(GPIO_ODR_OD6);
			GPIOC->ODR &= ~(GPIO_ODR_OD5);
			state = BLUE;
			break;
		case BLUE:
			GPIOC->ODR &= ~GPIO_ODR_OD8;
			GPIOC->ODR |= (GPIO_ODR_OD6);
			GPIOC->ODR &= ~(GPIO_ODR_OD5);
			state = RED;
			break;
		case RED:
			GPIOC->ODR &= ~GPIO_ODR_OD8;
			GPIOC->ODR &= ~(GPIO_ODR_OD6);
			GPIOC->ODR |= (GPIO_ODR_OD5);
			state = GREEN;
			break;
		default:
			break;
	}
}

























