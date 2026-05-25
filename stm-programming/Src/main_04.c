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

int main(void){

	/* Enable GPIOA clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	/* Configure PA5 MODER as output*/
	GPIOA->MODER |= (0b01 << GPIO_MODER_MODE5_Pos);
	GPIOA->OTYPER &= ~(0b1 << GPIO_OTYPER_OT5_Pos);				// push-pull
	GPIOA->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED5_Pos);		// fast
	GPIOA->ODR |= (0b1 << GPIO_ODR_OD5_Pos);					// initial value 1
	/* Configure PA11 as output*/
	GPIOA->MODER |= (0b01 << GPIO_MODER_MODE11_Pos);
	GPIOA->OTYPER &= ~(0b1 << GPIO_OTYPER_OT11_Pos);				// push-pull
	GPIOA->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED11_Pos);		// fast
	GPIOA->ODR |= (0b1 << GPIO_ODR_OD11_Pos);					// initial value 1

	/* Enable GPIOB clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	/* Configure PB11 MODER as output*/
	GPIOB->MODER |= (0b01 << GPIO_MODER_MODE12_Pos);
	GPIOB->OTYPER &= ~(0b1 << GPIO_OTYPER_OT12_Pos);				// push-pull
	GPIOB->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED12_Pos);		// fast
	GPIOB->ODR |= (0b1 << GPIO_ODR_OD12_Pos);					// initial value 1

	while(1){
		for (uint32_t i = 0; i < FREQUENCY; i++){
			/* when the for has run for half its length toggle the PB3 */
			if (i == FREQUENCY/2){
				GPIOB->ODR ^= (0b1 << GPIO_ODR_OD12_Pos);
			}
		}
		GPIOB->ODR ^= (0b1 << GPIO_ODR_OD12_Pos);
		/* The ^= (xor) flips the bit value of the ODR register at the position of the PA5 */
		GPIOA->ODR ^= (0b1 << GPIO_ODR_OD5_Pos);
	}
}
