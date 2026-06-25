/*
 * main_08.c
 *
 *  Created on: Jun 19, 2026
 *      Author: pedro
 *
 * HAL Blinky — LED on PA5 toggled every 250 ms by TIM3
 * Author: pedro
 */

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

/* TIM3 handle — must be global so stm32f4xx_it.c can access it */
TIM_HandleTypeDef htim3 = {0};

/* USART2 handle — must be global so stm32f4xx_it.c can access it */
USART_HandleTypeDef husart2 = {0};
uint8_t msg_buffer[64] = {0};

/* ADC handle — must be global so stm32f4xx_it.c can access it */
ADC_HandleTypeDef hadc1 = {0};
volatile uint16_t raw_adc = 0;
volatile uint8_t adc_done = 0;
float adc_value_mv = 0.0f;


/* Private function prototypes */
static void SystemClock_Config(void);
static void gpio_Init(void);
static void tim3_Init(void);
static void usart2_Init(void);
static void adc_Init(void);

int main(void)
{
    HAL_Init();           /* initialize HAL: SysTick, cache, priority grouping */
    SystemClock_Config(); /* configure clock tree: HSI at 16 MHz               */
    gpio_Init();          /* configure PA5 as push-pull output                  */
    tim3_Init();          /* configure TIM3: update event every 250 ms          */
    usart2_Init();
    adc_Init();

    HAL_USART_Transmit(&husart2, (uint8_t *)"hola mundo! \n\r", strlen("hola mundo! \n\r"), 100);

    HAL_ADC_Start_IT(&hadc1);

	/* ADC configuration
	 * 1. GPIOx of the selected channel
	 * 2. General ADC configuration
	 * 3. Configure ADC channel
	 * 4. Interruptions (NVIC)
	 * */

    while (1)
    {
        /* application loop — LED toggling happens in the callback */

    	if (adc_done)
    	{
    		adc_value_mv = (float)((3300.0f/4095.0f) * raw_adc);
    		sprintf((char *)msg_buffer, "ADC value: %f\n\r", adc_value_mv);
    		HAL_USART_Transmit(&husart2, msg_buffer, strlen((char *)msg_buffer), 100);

    		adc_done = 0;
    	}

    }
}

/*
 * SystemClock_Config
 * Uses HSI internal oscillator at 16 MHz
 * No PLL — simplest possible clock configuration
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* HSI is already on at reset — confirm and use it */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Select HSI as SYSCLK — all bus dividers set to 1 */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_HCLK   |
                                       RCC_CLOCKTYPE_PCLK1  |
                                       RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;   /* HCLK  = 16 MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;     /* APB1  = 16 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     /* APB2  = 16 MHz */

    /* FLASH_LATENCY_0 = zero wait states, correct for 16 MHz */
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}

/*
 * gpio_Init
 * Configures PA5 as push-pull output — onboard LED on Nucleo board
 */
static void gpio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIOA clock on AHB1 bus
       Same as bare-metal: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PA5 */
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}


static void adc_Init(void)
{
    GPIO_InitTypeDef GPIO_ADC_ch4 = {0};

    /* Enable GPIOA clock on AHB1 bus
       Same as bare-metal: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN */
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* Configure PA5 */
    GPIO_ADC_ch4.Pin   = GPIO_PIN_4;
    GPIO_ADC_ch4.Mode  = GPIO_MODE_ANALOG;
    GPIO_ADC_ch4.Pull  = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_ADC_ch4);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DMAContinuousRequests = DISABLE;

    HAL_ADC_Init(&hadc1);

    ADC_ChannelConfTypeDef adc_ch4 = {4};
    adc_ch4.Channel = ADC_CHANNEL_4;
    adc_ch4.Rank = 1;
    adc_ch4.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    adc_ch4.Offset = 0;

    HAL_ADC_ConfigChannel(&hadc1, &adc_ch4);

    HAL_NVIC_EnableIRQ(ADC_IRQn);

}

/*
 * tim3_Init
 * Configures TIM3 to generate an update event every 250 ms
 *
 * Clock chain:
 *   HSI (16 MHz) → APB1 (16 MHz) → TIM3 clock (16 MHz)
 *
 * PSC = 15999  →  tick = 16,000,000 / (15999 + 1) = 1,000 Hz  (1 ms per tick)
 * ARR = 249    →  period = (249 + 1) x 1 ms = 250 ms
 */
static void tim3_Init(void)
{
    /* Enable TIM3 clock on APB1 bus */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* Configure TIM3 base */
    htim3.Instance               = TIM3;
    htim3.Init.Prescaler         = 15999;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 249;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    /* Load configuration */
    HAL_TIM_Base_Init(&htim3);

    /* Enable TIM3 interrupt line in the NVIC */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    /* Start TIM3 in interrupt mode — enables the update event interrupt */
    HAL_TIM_Base_Start_IT(&htim3);
}

/* USART2 init
 *
 * configure usart2, with pins A2
 *
 * */
static void usart2_Init(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_Init_Tx = {0};
	GPIO_Init_Tx.Pin = GPIO_PIN_2;
	GPIO_Init_Tx.Mode = GPIO_MODE_AF_PP;
	GPIO_Init_Tx.Pull = GPIO_NOPULL;
	GPIO_Init_Tx.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Init_Tx.Alternate = GPIO_AF7_USART2;

	HAL_GPIO_Init(GPIOA, &GPIO_Init_Tx);

    __HAL_RCC_USART2_CLK_ENABLE();

    husart2.Instance = USART2;
    husart2.Init.BaudRate = 19200;
    husart2.Init.Mode = USART_MODE_TX;
    husart2.Init.Parity = USART_PARITY_NONE;
    husart2.Init.StopBits = USART_STOPBITS_1;
    husart2.Init.WordLength = USART_WORDLENGTH_8B;

    HAL_USART_Init(&husart2);
}


/*
 * HAL_TIM_PeriodElapsedCallback
 * Called automatically by HAL_TIM_IRQHandler() every time a timer
 * update event fires. Shared by all timers — always check htim->Instance.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1)
	{
		raw_adc = hadc->Instance->DR;
		adc_done = 1;
	}
}
