/*
 * main_tarea3.c
 *
 *  Created on: Jun 25, 2026
 *      Author: pedro
 */

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

/* TIM3 handle — must be global so stm32f4xx_it.c can access it */
TIM_HandleTypeDef htim3 = {0};
volatile uint8_t msg_flag = 0;

/* TIM2 handle — must be global so stm32f4xx_it.c can access it
 * This timer controls the ADC conversion
 *  */
TIM_HandleTypeDef htim2 = {0};

/* USART2 handle — must be global so stm32f4xx_it.c can access it */
UART_HandleTypeDef huart2 = {0};
volatile uint8_t rx_flag = 0;
uint8_t msg_buffer[64] = {0};
uint8_t Rx_char = {0};

/* ADC handle — must be global so stm32f4xx_it.c can access it */
ADC_HandleTypeDef hadc1 = {0};
volatile uint16_t raw_adc = 0;
volatile uint8_t adc_done = 0;
float adc_value_mv = 0.0f;

/* Private function prototypes */
static void SystemClock_Config(void);
static void gpio_Init(void);
static void tim3_Init(void);
static void tim2_Init(void);
static void usart2_Init(void);
static void adc_Init(void);

int main(void)
{
    HAL_Init();           /* initialize HAL: SysTick, cache, priority grouping */
    SystemClock_Config(); /* configure clock tree: HSI at 16 MHz               */
    gpio_Init();          /* configure PA5 as push-pull output                  */
    tim3_Init();          /* configure TIM3: update event every 250 ms          */
    tim2_Init();          /* configure TIM2: update event every 20 ms          */
    usart2_Init();
    adc_Init();

    while (1)
    {
        /* application loop — LED toggling happens in the callback */

    	if (msg_flag)
    	{
    		adc_value_mv = (float)((3300.0f/4095.0f) * raw_adc);
    		sprintf((char *)msg_buffer, "ADC value: %f\n\r", adc_value_mv);
    		HAL_UART_Transmit(&huart2, msg_buffer, strlen((char *)msg_buffer), 100);
    		msg_flag = 0;
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
    GPIO_InitTypeDef GPIO_blink = {0};

    /* Enable GPIOA clock on AHB1 bus
       Same as bare-metal: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PA5 */
    GPIO_blink.Pin   = GPIO_PIN_5;
    GPIO_blink.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_blink.Pull  = GPIO_NOPULL;
    GPIO_blink.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_blink);
}

static void adc_Init(void)
{
    /* Enable GPIOA clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PA4 */
    GPIO_InitTypeDef GPIO_ADC_ch4 = {0};
    GPIO_ADC_ch4.Pin   = GPIO_PIN_4;
    GPIO_ADC_ch4.Mode  = GPIO_MODE_ANALOG;
    GPIO_ADC_ch4.Pull  = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_ADC_ch4);

    /* Enable ADC clock */
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* Configure ADC */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;				/* Tells the ADC which timer is generating the signal to generate the conversion */
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;		/* Each time the TIM2 overflows a conversion is triggered */
    hadc1.Init.DMAContinuousRequests = DISABLE;

    /* Load ADC configuration */
    HAL_ADC_Init(&hadc1);

    /* Configure specific channel */
    ADC_ChannelConfTypeDef adc_ch4 = {0};
    adc_ch4.Channel = ADC_CHANNEL_4;
    adc_ch4.Rank = 1;
    adc_ch4.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    adc_ch4.Offset = 0;

    /* Load channel configuration */
    HAL_ADC_ConfigChannel(&hadc1, &adc_ch4);

    /* Enable ADC interruption in the NVIC */
    HAL_NVIC_EnableIRQ(ADC_IRQn);

    /* Start the ADC in interruption mode. It triggers each 20 ms with the TIM2 signal */
    HAL_ADC_Start_IT(&hadc1);
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
    htim3.Init.Prescaler         = 16000 - 1;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 250 - 1;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    /* Load configuration */
    HAL_TIM_Base_Init(&htim3);

    /* Enable TIM3 interrupt line in the NVIC */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    /* Start TIM3 in interrupt mode — enables the update event interrupt */
    HAL_TIM_Base_Start_IT(&htim3);
}

static void tim2_Init(void)
{
    /* Enable TIM2 clock on APB1 bus */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* Configure TIM4 base */
    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = 16000 - 1;							/* The prescaler counts in 1ms increments */
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 20 - 1;								/* The period is set to sample each 20 ms */
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    /* Load base configuration */
    HAL_TIM_Base_Init(&htim2);

    /* After initializing the timer, configure its Master Mode so that every update event generates a TRGO signal. */
    TIM_MasterConfigTypeDef TIM2_adc_master = {0};
    TIM2_adc_master.MasterOutputTrigger = TIM_TRGO_UPDATE;
    TIM2_adc_master.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

    /* Load TIM2 master configuration */
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &TIM2_adc_master);

    /* Start TIM2 without interruptions */
    HAL_TIM_Base_Start(&htim2);
}

/* USART2 init
 *
 * configure usart2, with pins A2
 *
 * */
static void usart2_Init(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure PA2 as Tx */
	GPIO_InitTypeDef GPIO_Init_Tx = {0};
	GPIO_Init_Tx.Pin = GPIO_PIN_2;
	GPIO_Init_Tx.Mode = GPIO_MODE_AF_PP;
	GPIO_Init_Tx.Pull = GPIO_NOPULL;
	GPIO_Init_Tx.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Init_Tx.Alternate = GPIO_AF7_USART2;

	HAL_GPIO_Init(GPIOA, &GPIO_Init_Tx);

	/* Configure PA3 as Rx */
	GPIO_InitTypeDef GPIO_Init_Rx = {0};
	GPIO_Init_Rx.Pin = GPIO_PIN_3;
	GPIO_Init_Rx.Mode = GPIO_MODE_AF_PP;
	GPIO_Init_Rx.Pull = GPIO_NOPULL;
	GPIO_Init_Rx.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Init_Rx.Alternate = GPIO_AF7_USART2;

	HAL_GPIO_Init(GPIOA, &GPIO_Init_Rx);

	/* Turn on clock for USART2 */
    __HAL_RCC_USART2_CLK_ENABLE();

    /* Configure USART2 for transmission and reception */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 19200;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    /* Load USART2 configuration */
    HAL_UART_Init(&huart2);

    /* Enable USART2 interrupt line in the NVIC */
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    HAL_UART_Receive_IT(&huart2, &Rx_char, 1);
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
        msg_flag = 1;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1)
	{
		raw_adc = HAL_ADC_GetValue(hadc);
		adc_done = 1;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
    	HAL_UART_Transmit(&huart2, &Rx_char, 1, 100);

        HAL_UART_Receive_IT(&huart2, &Rx_char, 1);
    }
}
