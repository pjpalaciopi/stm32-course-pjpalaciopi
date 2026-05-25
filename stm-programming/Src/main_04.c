/*
 * main_04.c
 *
 *  Created on: May 25, 2026
 *      Author: pedro
 */
#include <stdint.h>
#include "stm32f411xe.h"
#include "stm32f4xx.h"

int main(void){

	/* Enable GPIOA clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	/* Configure PA5 MODER as output*/
	GPIOA->MODER |= (0b01 << GPIO_MODER_MODE5_Pos);
	GPIOA->OTYPER &= ~(0b1 << GPIO_OTYPER_OT5_Pos);
	GPIOA->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED5_Pos);
	GPIOA->ODR |= (0b1 << GPIO_ODR_OD5_Pos);

	while(1){
		for (uint32_t i = 0; i < 1000000; i++){

		}
		/* The ^= (xor) flips the bit value of the ODR register at the position of the PA5 */
		GPIOA->ODR ^= (0b1 << GPIO_ODR_OD5_Pos);
	}
}
