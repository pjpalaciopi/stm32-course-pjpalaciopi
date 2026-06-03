/*
 * main_01.c
 *
 *  Created on: May 22, 2026
 *      Author: pedro
 */
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f411xe.h"

// variables
uint8_t pin_value = 0;

// headers
uint8_t readUserButton(void);
void toggleLedBSRR(void);
void toggleLedODR(void);

int main(void){

	// turn on clock register for GPIOA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	// turn on clock register for GPIOB
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	// turn off clock register for GPIOB
	RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
	// enable clock for GPIOC
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	// set GPIOA P5 as output
	GPIOA->MODER |= (0b01 << GPIO_MODER_MODE5_Pos);
	// set GPIOA P5 as push-pull
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT5_Pos);
	// set GPIOA P5 as fast speed
	GPIOA->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED5_Pos);
	// set GPIOA P5 output data register as 1 (turn on led)
	GPIOA->ODR |= (0b1 << GPIO_ODR_OD5_Pos);

	// set mode register as input for P13
	GPIOC->MODER &= ~(0b11 << GPIO_MODER_MODE13_Pos);		// input 0b00
	// set input with pull up resistor
	GPIOC->PUPDR |= (0b01 << GPIO_PUPDR_PUPD13_Pos);			// pull up 0b01


	while(1){
		pin_value = readUserButton();
		/* toggle pin using the BSR register, from position 0:15 it writes 1 to set the corresponding pin as 1 (high).
		 * From position 16:31 it writes 1 to set the corresponding pin (position - 16) as 0 (low).
		 * If both the low position and the high position are set, the high position takes priority
		 * */
		// toggleLedBSRR();
		/* toggle pin using the ODR
		 * */
		toggleLedODR();

	}
	return 0;
}

uint8_t readUserButton(void){
	uint8_t aux = 0;
	// load PC13 info into auxiliary variable
	aux = GPIOC->IDR >> 13;
	// clean any other bit
	aux &= 1;
	// return the state of the user button
	return aux;
}

void toggleLedBSRR(void){
	uint8_t state = readUserButton();
	if (state){
		GPIOA->BSRR |= (1 << GPIO_BSRR_BS5_Pos);
	}
	else{
		GPIOA->BSRR |= (1 << GPIO_BSRR_BR5_Pos);
	}
}

void toggleLedODR(void){
	uint8_t state = readUserButton();
	if (state){
		GPIOA->ODR |= (1 << GPIO_ODR_OD5_Pos);
	}
	else{
		GPIOA->ODR &= ~(1 << GPIO_ODR_OD5_Pos);
	}

}
