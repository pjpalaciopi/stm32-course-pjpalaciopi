/*
 * main_06.c
 *
 *  Created on: Jun 5, 2026
 *      Author: pedro
 */

#include <stdint.h>
#include "stm32f411xe.h"
#include "stm32f4xx.h"

volatile uint8_t increment_counter = 0;
uint8_t exti_counter = 0;

void init_EXTI(void);
void init_GPIO(void);
void init_TIMER(void);

int main(void){
	init_GPIO();
	init_TIMER();
	/*
	 * EXTI configuration
	 * 1. Turn on clock sysconfig APB2 bit 14.
	 * 2. Configure sysconfig register to eneble the EXTIx at the desired GPIO port. It may be SYSCFG_EXTICR1, SYSCFG_EXTICR2,
	 * SYSCFG_EXTICR3, or SYSCFG_EXTICR4.
	 * 3. Configure GPIOx Piny
	 * 4.a Select rising edge, falling edge or both
	 * 4.b Tell the NVIC that the interrupt is going to be used
	 * 4.c Clean the flag from the pending register
	 * 4.d Activate the interruption
	 * 4.e Write the ISR
	 * */
	init_EXTI();

	while(1){
		if (increment_counter == 1){
			exti_counter++;
			increment_counter = 0;
		}

	}
}

void init_EXTI(void){
	/* Turn on clock for SYSCONFG */
	RCC->APB1ENR |= RCC_APB2ENR_SYSCFGEN;
	/* Configure SYSCONFG EXTI MUX */
	SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI1);	// clean register
	SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PC);	// write EXTI1 to work with port GPIOC (PC1)
	EXTI->FTSR |= EXTI_FTSR_TR1;					// Enable falling edge detection
	NVIC_EnableIRQ(EXTI1_IRQn);						// Tell NVIC
	EXTI->PR |= EXTI_PR_PR1;						// clean flag
	EXTI->IMR |= EXTI_IMR_IM1;						// Enable interrupt
}

void init_GPIO(void){
	// turn on clock for GPIOC
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	// configure PC1
	GPIOC->MODER &= ~(GPIO_MODER_MODE1);			// input mode
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD1);			// no pullup pulldown
}

void init_TIMER(void){
	// turn on clock for TIM3
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	// configure TIM3
	TIM3->ARR = (30000 - 1);				// set autorreload to 3250 * 0.1 ms = 325 ms
	TIM3->PSC = (1600 -1);					// set prescaler to 0.1 ms
	TIM3->CNT = 0;							// reset counter
	TIM3->SR &= ~(TIM_SR_UIF);				// clean interruption flag
	TIM3->DIER &= ~(TIM_DIER_UIE);			// clean interruption enable
	TIM3->DIER |= TIM_DIER_UIE;				// set interruption enable
	/* The NVIC must know that an interrupt from the TIM3 is enabled */
	__NVIC_EnableIRQ(TIM3_IRQn);		// found in the /Includes/.../Core/Include/core_cm4.h file
	TIM3->CR1 &= ~(TIM_CR1_DIR);		// set as upcounter
	TIM3->CR1 &= ~(TIM_CR1_ARPE);		// clean arr preload
	TIM3->CR1 |= (TIM_CR1_ARPE);		// set arr preload
	TIM3->CR1 &= ~(TIM_CR1_CEN);		// clean timer enable
	TIM3->CR1 |= (TIM_CR1_CEN);			// set timer enable
}

void EXTI1_IRQHandler(void){
	if (EXTI->PR && EXTI_PR_PR1){
		EXTI->PR |= EXTI_PR_PR1;		// clean flag
		increment_counter = 1;			// every variable to be changed between an interruption must be volatile
	}
}































