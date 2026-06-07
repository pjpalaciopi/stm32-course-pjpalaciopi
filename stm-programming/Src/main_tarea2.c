/*
 * main_tarea2.c
 *
 *  Created on: Jun 6, 2026
 *      Author: pedro
 */

#include <stdint.h>
#include "stm32f411xe.h"
#include "stm32f4xx.h"

typedef enum{
	display1,
	display2,
	display3,
	display4
}digit_t;

// variable
volatile uint8_t refresh_interrupt_flag = 0;
uint16_t counter = 0;
uint8_t digit_position = 0;
uint8_t digit_number = 0;
uint8_t position = 0;
uint8_t units = 0;
uint8_t tens = 0;
uint8_t hundreds = 0;
uint8_t thousands = 0;

uint8_t PC1State = 0;
uint8_t PC0State = 0;

// headers
void initGPIO(void);
void initTimer(void);
void initEXTI(void);

uint8_t changeDisplay(void);
void drawNumber(uint8_t number);
uint8_t extractNumber(uint8_t digit);

int main(void){
	initGPIO();
	initTimer();
	initEXTI();

	while(1){
		if (refresh_interrupt_flag){
			digit_position = changeDisplay();
			digit_number = extractNumber(digit_position);
			drawNumber(digit_number);

			refresh_interrupt_flag = 0;
		}
		PC1State = (GPIOC->IDR >> 1) & 1;
		PC0State = (GPIOC->IDR) & 1;
	}
}

// functions
/* Initial configuration of the GPIO pins */
void initGPIO(void){
	/* Turn on clock for the GPIOA */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	/* Turn on clock for the GPIOB */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	/* Turn on clock for the GPIOC */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	/* Turn on clock for the GPIOD */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	/* Turn on clock for the GPIOH */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

	/* Configure Blinking LED */
	/* Set GPIOH Pin 1 as output */
	GPIOH->MODER |= GPIO_MODER_MODE1_0;			// GPIO_MODER_MODE1_0 = 0b01 << 2
	/* Set output type as push-pull */
	GPIOH->OTYPER &= ~GPIO_OTYPER_OT1;			// GPIO_OTYPER_OT1 = 0b1 << 1
	/* Set output speed as fast */
	GPIOH->OSPEEDR |= GPIO_OSPEEDR_OSPEED1_1;	// GPIO_OSPEEDR_OSPEED1_1 = 0b10 << 2
	/* Set initial state in the output data register as 1 (high) */
	GPIOH->ODR |= GPIO_ODR_OD1;					// GPIO_ODR_OD1 = 0b1 << 1

	/* Configure PC13 as digit 1 of the 7 segment display */
	/* Set GPIOC Pin 13 as output */
	GPIOC->MODER |= GPIO_MODER_MODE13_0;
	/* Set output type as push-pull */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT13;
	/* Set output speed as fast */
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED13_1;
	/* Set initial state in the output data register as 1 (Turn off for a common anode display) */
	GPIOC->ODR |= GPIO_ODR_OD13;

	/* Configure PC6 as digit 2 of the 7 segment display */
	/* Set GPIOC Pin 6 as output */
	GPIOC->MODER |= GPIO_MODER_MODE6_0;
	/* Set output type as push-pull */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT6;
	/* Set output speed as fast */
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_1;
	/* Set initial state in the output data register as 1 (Turn off for a common anode display) */
	GPIOC->ODR |= GPIO_ODR_OD6;

	/* Configure PC12 as digit 3 of the 7 segment display */
	/* Set GPIOC Pin 12 as output */
	GPIOC->MODER |= GPIO_MODER_MODE12_0;
	/* Set output type as push-pull */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT12;
	/* Set output speed as fast */
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED12_1;
	/* Set initial state in the output data register as 1 (Turn off for a common anode display) */
	GPIOC->ODR |= GPIO_ODR_OD12;

	/* Configure PC8 as digit 4 of the 7 segment display */
	/* Set GPIOC Pin 8 as output */
	GPIOC->MODER |= GPIO_MODER_MODE8_0;
	/* Set output type as push-pull */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT8;
	/* Set output speed as fast */
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED8_1;
	/* Set initial state in the output data register as 1 (Turn off for a common anode display) */
	GPIOC->ODR |= GPIO_ODR_OD8;

	/* Configure PC11 as segment A of the display */
	/* Set GPIOC Pin 11 as output */
	GPIOC->MODER |= GPIO_MODER_MODE11_0;
	/* Set output type as push-pull */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT11;
	/* Set output speed as fast */
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED11_1;
	/* Set initial state in the output data register as 0 (Turn on for a common anode display) */
	GPIOC->ODR &= ~GPIO_ODR_OD11;

	/* Configure PD2 as segment B of the display */
	/* Set GPIOD Pin 2 as output */
	GPIOD->MODER |= GPIO_MODER_MODE2_0;
	/* Set output type as push-pull */
	GPIOD->OTYPER &= ~GPIO_OTYPER_OT2;
	/* Set output speed as fast */
	GPIOD->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_1;
	/* Set initial state in the output data register as 0 (Turn on for a common anode display) */
	GPIOD->ODR &= ~GPIO_ODR_OD2;

	/* Configure PC5 as segment C of the display */
	/* Set GPIOC Pin 5 as output */
	GPIOC->MODER |= GPIO_MODER_MODE5_0;
	/* Set output type as push-pull */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT5;
	/* Set output speed as fast */
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED5_1;
	/* Set initial state in the output data register as 0 (Turn on for a common anode display) */
	GPIOC->ODR &= ~GPIO_ODR_OD5;

	/* Configure PA11 as segment D of the display */
	/* Set GPIOB Pin 7 as output */
	GPIOA->MODER |= GPIO_MODER_MODE11_0;
	/* Set output type as push-pull */
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT11;
	/* Set output speed as fast */
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED11_1;
	/* Set initial state in the output data register as 0 (Turn on for a common anode display) */
	GPIOA->ODR &= ~GPIO_ODR_OD11;

	/* Configure PA12 as segment E of the display */
	/* Set GPIOA Pin 12 as output */
	GPIOA->MODER |= GPIO_MODER_MODE12_0;
	/* Set output type as push-pull */
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT12;
	/* Set output speed as fast */
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED12_1;
	/* Set initial state in the output data register as 0 (Turn on for a common anode display) */
	GPIOA->ODR &= ~GPIO_ODR_OD12;

	/* Configure PC10 as segment F of the display */
	/* Set GPIOC Pin 10 as output */
	GPIOC->MODER |= GPIO_MODER_MODE10_0;
	/* Set output type as push-pull */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT10;
	/* Set output speed as fast */
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED10_1;
	/* Set initial state in the output data register as 0 (Turn on for a common anode display) */
	GPIOC->ODR &= ~GPIO_ODR_OD10;

	/* Configure PB12 as segment G of the display */
	/* Set GPIOB Pin 12 as output */
	GPIOB->MODER |= GPIO_MODER_MODE12_0;
	/* Set output type as push-pull */
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT12;
	/* Set output speed as fast */
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED12_1;
	/* Set initial state in the output data register as 0 (Turn on for a common anode display) */
	GPIOB->ODR &= ~GPIO_ODR_OD12;

	/* configure PC1 as input for the external interrupt */
	GPIOC->MODER &= ~(GPIO_MODER_MODE1);			// input mode
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD1);			// no pullup pulldown

	/* configure PC0 as input for the external interrupt */
	GPIOC->MODER &= ~(GPIO_MODER_MODE0);			// input mode
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD0);			// no pullup pulldown
}

/* Initial configuration for the timers*/
void initTimer(void){
	// turn on clock for TIM3
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	/* configure TIM3 to control the blinking LED period*/
	/* set prescaler to 0.1 ms */
	TIM3->PSC = (1600 -1);
	/* set autorreload to 2500 * 0.1 ms = 250 ms */
	TIM3->ARR = (2500 - 1);
	/* reset counter */
	TIM3->CNT = 0;
	/* clean interruption flag */
	TIM3->SR &= ~(TIM_SR_UIF);
	/* Clear and set the interrupt enable register bit 0 (update interrupt enable)*/
	TIM3->DIER &= ~(TIM_DIER_UIE);			// clear interruption enable
	TIM3->DIER |= TIM_DIER_UIE;				// set interruption enable
	/* The NVIC must know that an interrupt from the TIM3 is enabled */
	__NVIC_EnableIRQ(TIM3_IRQn);		// found in the /Includes/.../Core/Include/core_cm4.h file
	/* Set the timer as upcounter */
	TIM3->CR1 &= ~(TIM_CR1_DIR);
	/* Clear and set the auto-reload preload bit */
	TIM3->CR1 &= ~(TIM_CR1_ARPE);		// clear arr preload
	TIM3->CR1 |= (TIM_CR1_ARPE);		// set arr preload
	/* Clear and set the enable bit in the timer control register*/
	TIM3->CR1 &= ~(TIM_CR1_CEN);		// clear timer enable
	TIM3->CR1 |= (TIM_CR1_CEN);			// set timer enable

	// turn on clock for TIM4
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	/* configure TIM4 to control the display refresh rate*/
	/* set prescaler to 0.1 ms */
	TIM4->PSC = (1600 -1);
	/* set autorreload to 70 * 0.1 ms = 8 ms */
	TIM4->ARR = (70 - 1);
	/* reset counter */
	TIM4->CNT = 0;
	/* clean interruption flag */
	TIM4->SR &= ~(TIM_SR_UIF);
	/* Clear and set the interrupt enable register bit 0 (update interrupt enable)*/
	TIM4->DIER &= ~(TIM_DIER_UIE);			// clear interruption enable
	TIM4->DIER |= TIM_DIER_UIE;				// set interruption enable
	/* The NVIC must know that an interrupt from the TIM4 is enabled */
	__NVIC_EnableIRQ(TIM4_IRQn);		// found in the /Includes/.../Core/Include/core_cm4.h file
	/* Set the timer as upcounter */
	TIM4->CR1 &= ~(TIM_CR1_DIR);
	/* Clear and set the auto-reload preload bit */
	TIM4->CR1 &= ~(TIM_CR1_ARPE);		// clear arr preload
	TIM4->CR1 |= (TIM_CR1_ARPE);		// set arr preload
	/* Clear and set the enable bit in the timer control register*/
	TIM4->CR1 &= ~(TIM_CR1_CEN);		// clear timer enable
	TIM4->CR1 |= (TIM_CR1_CEN);			// set timer enable
}

/* Initial configuration for the EXTI */
void initEXTI(void){
	/* Turn on clock for SYSCONFG */
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	/* Configure SYSCONFG EXTI MUX to select EXTI1*/
	SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI1);	// Clear register
	SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI1_PC);	// Write EXTI1 to work with port GPIOC (PC1)
	EXTI->FTSR |= EXTI_FTSR_TR1;					// Enable falling edge detection
	NVIC_EnableIRQ(EXTI1_IRQn);						// Tell NVIC I'm using EXTI1
	EXTI->PR |= EXTI_PR_PR1;						// Clear flag
	EXTI->IMR |= EXTI_IMR_IM1;						// Enable interrupt

	/* Configure SYSCONFG EXTI MUX to select EXTI0*/
	SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0);	// Clear register
	SYSCFG->EXTICR[0] |= (SYSCFG_EXTICR1_EXTI0_PC);	// Write EXTI1 to work with port GPIOC (PC0)
	EXTI->RTSR |= EXTI_RTSR_TR0;					// Enable rising edge detection
	NVIC_EnableIRQ(EXTI0_IRQn);						// Tell NVIC I'm using EXTI0
	EXTI->PR |= EXTI_PR_PR0;						// Clear flag
	EXTI->IMR |= EXTI_IMR_IM0;						// Enable interrupt
}


/* ISR for the blink period */
void TIM3_IRQHandler(void){
	/* verify which flag is up for the interruption */
	if (TIM3->SR && TIM_SR_UIF){
		GPIOH->ODR ^= GPIO_ODR_OD1;		// change the state of PH1
		TIM3->SR &= ~(TIM_SR_UIF);		// clear flag
	}
}

/* ISR for the refresh rate */
void TIM4_IRQHandler(void){
	/* Check which flag is up */
	if (TIM4->SR && TIM_SR_UIF){
		TIM4->SR &= ~TIM_SR_UIF;		// clear flag
		/* I write to a custom flag to avoid doing computations during the interruption */
		refresh_interrupt_flag = 1;
	}
}

/* ISR for the EXTI1 counter up */
void EXTI1_IRQHandler(void){
	if (EXTI->PR && EXTI_PR_PR1){
		/* The EXTI pending register is cleared by writing 1 */
		EXTI->PR |= EXTI_PR_PR1;		// clear flag
		counter--;						// every variable to be changed between an interruption must be volatile
	}
}

/* ISR for the EXTI0 counter down */
void EXTI0_IRQHandler(void){
	if (EXTI->PR && EXTI_PR_PR0){
		/* The EXTI pending register is cleared by writing 1 */
		EXTI->PR |= EXTI_PR_PR0;		// clear flag
		counter++;						// every variable to be changed between an interruption must be volatile
	}
}

/* Change the selected display */
uint8_t changeDisplay(void){
	switch (position) {
		case display1:
			/* Turn on digit 1 and turn off the rest */
			GPIOC->ODR &= ~GPIO_ODR_OD13;
			GPIOC->ODR |= GPIO_ODR_OD6;
			GPIOC->ODR |= GPIO_ODR_OD12;
			GPIOC->ODR |= GPIO_ODR_OD8;
			position++;		// update the display number for the next call to enter case display2
			return display1;
			break;

		case display2:
			/* Turn on digit 2 and turn off the rest */
			GPIOC->ODR |= GPIO_ODR_OD13;
			GPIOC->ODR &= ~GPIO_ODR_OD6;
			GPIOC->ODR |= GPIO_ODR_OD12;
			GPIOC->ODR |= GPIO_ODR_OD8;
			position++;		// update the display number for the next call to enter case display3
			return display2;
			break;

		case display3:
			/* Turn on digit 3 and turn off the rest */
			GPIOC->ODR |= GPIO_ODR_OD13;
			GPIOC->ODR |= GPIO_ODR_OD6;
			GPIOC->ODR &= ~GPIO_ODR_OD12;
			GPIOC->ODR |= GPIO_ODR_OD8;
			position++;		// update the display number for the next call to enter case display4
			return display3;
			break;

		case display4:
			/* Turn on digit 4 and turn off the rest */
			GPIOC->ODR |= GPIO_ODR_OD13;
			GPIOC->ODR |= GPIO_ODR_OD6;
			GPIOC->ODR |= GPIO_ODR_OD12;
			GPIOC->ODR &= ~GPIO_ODR_OD8;
			position = 0;		// update the display number for the next call to go back to case display1
			return display4;
			break;

		default:
			break;
	}
	return 0;
}

void drawNumber(uint8_t digit_number){
	switch (digit_number) {
		case 0:
			/* Draw the digit 0 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR &= ~GPIO_ODR_OD11;		// segment D on
			GPIOA->ODR &= ~GPIO_ODR_OD12;		// segment E on
			GPIOC->ODR &= ~GPIO_ODR_OD10;		// segment F on
			GPIOB->ODR |= GPIO_ODR_OD12;			// segment G off
			break;

		case 1:
			/* Draw the digit 1 */
			GPIOC->ODR |= GPIO_ODR_OD11;		// segment A off
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR |= GPIO_ODR_OD11;		// segment D off
			GPIOA->ODR |= GPIO_ODR_OD12;		// segment E off
			GPIOC->ODR |= GPIO_ODR_OD10;		// segment F off
			GPIOB->ODR |= GPIO_ODR_OD12;			// segment G off
			break;

		case 2:
			/* Draw the digit 2 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR |= GPIO_ODR_OD5;			// segment C off
			GPIOA->ODR &= ~GPIO_ODR_OD11;		// segment D on
			GPIOA->ODR &= ~GPIO_ODR_OD12;		// segment E on
			GPIOC->ODR |= GPIO_ODR_OD10;		// segment F off
			GPIOB->ODR &= ~GPIO_ODR_OD12;		// segment G on
			break;

		case 3:
			/* Draw the digit 3 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR &= ~GPIO_ODR_OD11;		// segment D on
			GPIOA->ODR |= GPIO_ODR_OD12;		// segment E off
			GPIOC->ODR |= GPIO_ODR_OD10;		// segment F off
			GPIOB->ODR &= ~GPIO_ODR_OD12;		// segment G on
			break;

		case 4:
			/* Draw the digit 4 */
			GPIOC->ODR |= GPIO_ODR_OD11;		// segment A off
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR |= GPIO_ODR_OD11;		// segment D off
			GPIOA->ODR |= GPIO_ODR_OD12;		// segment E off
			GPIOC->ODR &= ~GPIO_ODR_OD10;		// segment F on
			GPIOB->ODR &= ~GPIO_ODR_OD12;		// segment G on
			break;

		case 5:
			/* Draw the digit 5 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR |= GPIO_ODR_OD2;			// segment B off
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR &= ~GPIO_ODR_OD11;		// segment D on
			GPIOA->ODR |= GPIO_ODR_OD12;			// segment E off
			GPIOC->ODR &= ~GPIO_ODR_OD10;		// segment F on
			GPIOB->ODR &= ~GPIO_ODR_OD12;		// segment G on
			break;

		case 6:
			/* Draw the digit 6 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR |= GPIO_ODR_OD2;			// segment B off
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR &= ~GPIO_ODR_OD11;		// segment D on
			GPIOA->ODR &= ~GPIO_ODR_OD12;		// segment E on
			GPIOC->ODR &= ~GPIO_ODR_OD10;		// segment F on
			GPIOB->ODR &= ~GPIO_ODR_OD12;		// segment G on
			break;

		case 7:
			/* Draw the digit 7 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR |= GPIO_ODR_OD11;		// segment D off
			GPIOA->ODR |= GPIO_ODR_OD12;		// segment E off
			GPIOC->ODR |= GPIO_ODR_OD10;		// segment F off
			GPIOB->ODR |= GPIO_ODR_OD12;		// segment G off
			break;

		case 8:
			/* Draw the digit 8 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR &= ~GPIO_ODR_OD11;		// segment D on
			GPIOA->ODR &= ~GPIO_ODR_OD12;		// segment E on
			GPIOC->ODR &= ~GPIO_ODR_OD10;		// segment F on
			GPIOB->ODR &= ~GPIO_ODR_OD12;		// segment G on
			break;

		case 9:
			/* Draw the digit 9 */
			GPIOC->ODR &= ~GPIO_ODR_OD11;		// segment A on
			GPIOD->ODR &= ~GPIO_ODR_OD2;		// segment B on
			GPIOC->ODR &= ~GPIO_ODR_OD5;		// segment C on
			GPIOA->ODR &= ~GPIO_ODR_OD11;		// segment D on
			GPIOA->ODR |= GPIO_ODR_OD12;		// segment E off
			GPIOC->ODR &= ~GPIO_ODR_OD10;		// segment F on
			GPIOB->ODR &= ~GPIO_ODR_OD12;		// segment G on
			break;
		default:
			break;
	}

}

/* Extract the individual value for each digit position */
uint8_t extractNumber(uint8_t digit_position){
	uint16_t aux = counter;				// Declare the an auxiliary variable to operate over
	units = aux % 10;					// the units is the remainder after division by 1
	tens = (aux / 10) % 10;				// the tens is the remainder after division by 10
	hundreds = (aux / 100) % 10;		// the hundreds is the remainder after division by 100
	thousands = (aux / 1000) % 10;		// the thousands is the remainder after division by 1000

	switch (digit_position) {
		case 0:
			return thousands;
			break;
		case 1:
			return hundreds;
			break;
		case 2:
			return tens;
			break;
		case 3:
			return units;
			break;
		default:
			break;
	}
	return 0;
}































