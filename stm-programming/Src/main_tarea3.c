/*
 * main_tarea3.c
 *
 *  Created on: Jun 25, 2026
 *      Author: pedro
 */

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

/* Enumeration for the FMS */
typedef enum
{
    STATE_IDLE,
    STATE_UART_EVENT,
    STATE_ENCODER_EVENT,
    STATE_ADC_EVENT
} FSM_State_t;

FSM_State_t state = STATE_IDLE;

/* TIM4 handle — must be global so stm32f4xx_it.c can access it
 * This timer manages the encoder */
TIM_HandleTypeDef htim4 = {0};
uint16_t old_position = 0;
uint16_t position = 0;
uint8_t dir = 0;
/* variable that triggers with a change in encoder position */
uint8_t encoder_event = 0;

/* TIM3 handle — must be global so stm32f4xx_it.c can access it */
TIM_HandleTypeDef htim3 = {0};
volatile uint8_t msg_flag = 0;

/* TIM2 handle — must be global so stm32f4xx_it.c can access it
 * This timer controls the ADC conversion
 *  */
TIM_HandleTypeDef htim2 = {0};

/* TIM1 handle — must be global so stm32f4xx_it.c can access it
 * This timer controls the PWM
 *  */
TIM_HandleTypeDef htim1 = {0};


/* USART2 handle — must be global so stm32f4xx_it.c can access it */
UART_HandleTypeDef huart2 = {0};
volatile uint8_t uart_event = 0;
volatile uint8_t rx_flag = 0;
uint8_t msg_buffer[150] = {0};
uint8_t Rx_char = {0};

/* ADC handle — must be global so stm32f4xx_it.c can access it */
ADC_HandleTypeDef hadc1 = {0};
volatile uint16_t raw_adc = 0;
volatile uint8_t adc_event = 0;
float adc_value_mv = 0.0f;

/* Private function prototypes */
static void SystemClock_Config(void);
static void gpio_Init(void);
static void tim4_Init(void);
static void tim3_Init(void);
static void tim2_Init(void);
static void tim1_init(void);
static void usart2_Init(void);
static void adc_Init(void);

/* FSM state functions */
void change_encoder_position(void);
void process_uart(void);
void process_encoder(void);
void process_adc(void);
void change_state();
void print_menu(void);
void print_state(void);

/* PWM value variables */
uint16_t red_pwm = 0;
uint16_t green_pwm = 0;
uint16_t blue_pwm = 0;


int main(void)
{
    HAL_Init();				/* initialize HAL: SysTick, cache, priority grouping */
    SystemClock_Config();	/* configure clock tree: HSI at 16 MHz */
    gpio_Init();          	/* configure PA5 as push-pull output */
    tim4_Init();			/* configure TIM4 in Encoder mode */
    tim3_Init();	        /* configure TIM3: update event every 250 ms */
    tim2_Init();			/* configure TIM2: update event every 20 ms */
    tim1_init();
    usart2_Init();
    adc_Init();

    print_menu();
    print_state();

    while (1)
    {
    	change_encoder_position();
    	change_state();
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
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Configure PA5 */
    GPIO_blink.Pin   = GPIO_PIN_1;
    GPIO_blink.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_blink.Pull  = GPIO_NOPULL;
    GPIO_blink.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOH, &GPIO_blink);
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

/* TIM4 configuration in encoder mode */
static void tim4_Init(void)
{
	/* Configure GPIO pins B6 (ch1) and B7 (ch2) as alternate function2 to work with TIM4*/
	/* Turn on GPIOB clock */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_Encoder = {0};

	GPIO_Encoder.Pin = GPIO_PIN_6 | GPIO_PIN_7;		/* Set the two bits at once */
	GPIO_Encoder.Mode = GPIO_MODE_AF_PP;			/* Alternate function push-pull */
	GPIO_Encoder.Pull = GPIO_NOPULL;				/* No pull-up pull-down */
	GPIO_Encoder.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Encoder.Alternate = GPIO_AF2_TIM4;			/* Set the alternate function 2 according to the datasheet */

	/* Load channel configuration */
	HAL_GPIO_Init(GPIOB, &GPIO_Encoder);

    /* Enable TIM4 clock on APB1 bus */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* Configure TIM4 base */
    htim4.Instance               = TIM4;
    htim4.Init.Prescaler         = 0;								/* In Encoder mode the counter is not driven by the timer so the prescaler is not necessary */
    htim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim4.Init.Period            = 0xffff;
    htim4.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    /* Load configuration */
    HAL_TIM_Base_Init(&htim4);

    /* Define handler for the Encoder mode configuration */
    TIM_Encoder_InitTypeDef EncoderConfig = {0};

    /* Uses transitions from both channels resulting in 4x decoding as opposed to the other options that
     * only uses one channel edge to count resulting in 2x decoding*/
    EncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;

    EncoderConfig.IC1Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
    EncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;			/* Tells the timer to direct the ch1 input to input capture 1 */
    EncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    EncoderConfig.IC1Filter = 10;

    EncoderConfig.IC2Polarity = TIM_INPUTCHANNELPOLARITY_RISING;
    EncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;			/* Tells the timer to direct the ch2 input to input capture 2 */
    EncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    EncoderConfig.IC2Filter = 10;

    /* Load Encoder configuration */
    HAL_TIM_Encoder_Init(&htim4, &EncoderConfig);

    /* Start the timer in encoder mode */
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
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

/* Initial configuration for the TIM2 that controls the trigger output signal of the ADC conversion.
 * It has a 16 kHz prescaler with gives a count each 1 ms. The period is set to 20 ms effectively
 * sampling at 50 Hz.
 * Additionally the timer must be configured as master so that it can serve as an input to trigger
 * the ADC conversion.
 *  */
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

/* Configure TIM1 as PWM with 4 channels PA8 - PA10 */
static void tim1_init(void)
{
	/* Turn on Timer 1 clock */
	__HAL_RCC_TIM1_CLK_ENABLE();

	/* Configure GPIOA pins as alternate functions */
	/* Turn on GPIOA clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_PWM_channels = {0};

	GPIO_PWM_channels.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_PWM_channels.Mode = GPIO_MODE_AF_PP;
	GPIO_PWM_channels.Pull = GPIO_NOPULL;
	GPIO_PWM_channels.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_PWM_channels.Alternate = GPIO_AF1_TIM1;

	HAL_GPIO_Init(GPIOA, &GPIO_PWM_channels);

	/* TIM1 base configuration */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 15;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 999;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	HAL_TIM_PWM_Init(&htim1);

	/* Configure PWM */
	TIM_OC_InitTypeDef PWM_Config = {0};

	PWM_Config.OCMode = TIM_OCMODE_PWM1;
	PWM_Config.OCPolarity = TIM_OCPOLARITY_HIGH;
	PWM_Config.OCFastMode = TIM_OCFAST_DISABLE;

	/* Configures the Capture/Compare register, the Duty = CC/(ARR + 1) */
	PWM_Config.Pulse = 0;
	HAL_TIM_PWM_ConfigChannel(&htim1, &PWM_Config, TIM_CHANNEL_1);

	PWM_Config.Pulse = 0;
	HAL_TIM_PWM_ConfigChannel(&htim1, &PWM_Config, TIM_CHANNEL_2);

	PWM_Config.Pulse = 0;
	HAL_TIM_PWM_ConfigChannel(&htim1, &PWM_Config, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

/* USART2 init
 *
 * configure usart2, with pins A2 Tx and A3 Rx
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
        HAL_GPIO_TogglePin(GPIOH, GPIO_PIN_1);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == ADC1)
	{
		adc_event = 1;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
    	uart_event = 1;

        HAL_UART_Receive_IT(&huart2, &Rx_char, 1);
    }
}

void change_encoder_position(void)
{
	int16_t encoder_count;

	encoder_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);

	position = ((encoder_count / 4) % 101 + 101) % 101;

	if(position != old_position)
	{
	    encoder_event = 1;
	    old_position = position;
	}

}

void process_uart(void)
{
	/* The Rx interruption can receive the following characters:
	 * '+': increments the PWM duty in 10 units, effectively yielding 100 levels
	 * '-': decreases the PWM duty in 10 units, effectively yielding 100 levels
	 * */
	if(Rx_char == '+')
	{
	    if(green_pwm >= 1000)
	        green_pwm = 0;
	    else
	        green_pwm += 10;
	}
	else if(Rx_char == '-')
	{
	    if(green_pwm == 0)
	        green_pwm = 1000;
	    else
	        green_pwm -= 10;
	}

	else if(Rx_char == 'm')
	{
		print_menu();
	}

	else if(Rx_char == 's')
	{
		print_state();
	}


    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, green_pwm);
}

void process_encoder(void)
{
	/* The position range is (0 - 100) times 10 yields the corresponding duty between 0 - 1000 */
    blue_pwm = position * 10;

    /* set the new duty changing the CCR register */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, blue_pwm);
}

void process_adc(void)
{
	raw_adc = HAL_ADC_GetValue(&hadc1);
	/* get a value between (0 - 1000) as the PWM duty, normalizing the ADC conversion range (0 - 4095) */
    red_pwm = raw_adc * 1000 / 4095;

    /* set the new duty changing the CCR register */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, red_pwm);
}

void change_state()
{
	switch(state)
	{
		case STATE_IDLE:

			if(uart_event)
				state = STATE_UART_EVENT;

			else if(encoder_event)
				state = STATE_ENCODER_EVENT;

			else if(adc_event)
				state = STATE_ADC_EVENT;

			break;

		case STATE_UART_EVENT:

			process_uart();
			uart_event = 0;
			state = STATE_IDLE;

			break;

		case STATE_ENCODER_EVENT:

			process_encoder();
			encoder_event = 0;
			state = STATE_IDLE;

			break;

		case STATE_ADC_EVENT:

			process_adc();
			adc_event = 0;
			state = STATE_IDLE;

			break;

		default:
			break;
	}
}

void print_menu(void)
{
	    	sprintf((char *)msg_buffer, "Menu:\r\n 'm': print menu, 's': print current state\r\n"
	    			" Potentiometer: Red LED, Encoder: Blue LED,"
	    			" '+': Increase Green LED, '-': Decrease Green LED\r\n");

	    	HAL_UART_Transmit(&huart2, msg_buffer, strlen((char *)msg_buffer), 100);
}

void print_state(void)
{
	sprintf((char *)msg_buffer, "R=%u G=%u B=%u\r\n", red_pwm, green_pwm, blue_pwm);

	HAL_UART_Transmit(&huart2, msg_buffer, strlen((char *)msg_buffer), 100);
}

































