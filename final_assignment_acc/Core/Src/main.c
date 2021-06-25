/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "acc_fir.h"
#include "acc_utils.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4_discovery_lis3dsh.h"
#include <stdio.h>
#include <math.h>
#include "stm32f4xx.h"

#include "arm_math.h"
#include "math_helper.h"
#include "noarm_cmsis.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define __FPU_PRESENT             1U       /*!< FPU present                                   */
/* ----------------------------------------------------------------------
 * Defines for each of the tests performed
 * ------------------------------------------------------------------- */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

void Acc_Config(void);

uint8_t chRX = 0;
uint8_t dataReceived = 0;
uint8_t stream_active = 0;
uint8_t stream_mode = 0;
uint8_t dataReady = 0;

float32_t roll;
float32_t pitch;

uint16_t idx = 0;
int16_t accData[SIZE][3];

// fir output data
float32_t fir_out_x[SIZE];
float32_t fir_out_y[SIZE];
float32_t fir_out_z[SIZE];

// To check the inclusion on DSP
// Test as in: https://community.st.com/s/article/configuring-dsp-libraries-on-stm32cubeide
#define FFT_Length 1024
float32_t FFT_Input_Q15_f[50];
float32_t aFFT_Input_Q15[50];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void led_off();
void led_orientation(float32_t roll, float32_t pitch);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	// To check the inclusion of DSP
	// Test as in: https://community.st.com/s/article/configuring-dsp-libraries-on-stm32cubeide
	arm_float_to_q15((float32_t*) &FFT_Input_Q15_f[0], (q15_t*) &aFFT_Input_Q15[0], FFT_Length * 2);

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM3_Init();
	MX_USART2_UART_Init();
	MX_SPI1_Init();
	/* USER CODE BEGIN 2 */

	if (HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq)) > 0U) {
		return HAL_ERROR;
	}

	LL_USART_EnableIT_RXNE(USART2);
	LL_USART_Enable(USART2);

	LL_SPI_Enable(LIS3DSH_SPI);
	Acc_Config();

	// initialize 3 axis FIR
	init_fir_3_axes();
	led_off();

	printf("Type s to start/stop streaming and d to toggle streaming mode...\r\n");
	/* USER CODE END 2 */
	stream_active = 1;
	stream_mode = 1;
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (dataReceived) {
			if (chRX == 's') {
				stream_active = 1 - stream_active;
				if (stream_active == 1) {
					printf("Stream ON\r\n");
				} else {
					printf("Stream OFF\r\n");
				}

			} else if (chRX == 'd') {
				// toggle streaming mode between accelerometer data streaming and roll, pitch
				stream_mode = 1 - stream_mode;
				if (stream_mode == 1) {
					printf("Now streaming (roll, pitch) \r\n");
				} else {
					printf("Now streaming (x, y, z)\r\n");
				}
			} else {
				printf("Wrong command\r\n");
			}

			dataReceived = 0;
		}

		if (stream_active) {
			if (dataReady) {
				LIS3DSH_ReadACC(accData[idx++]);
				// set data ready after reading
				// wait another 10ms before doing it again
				dataReady = 0;
				if (idx == SIZE) {
					// collected SIZE samples (stream freq / sample freq)
					// run FIR on collected data, stream most current value
					fir_3_axes(accData, &fir_out_x[0], &fir_out_y[0], &fir_out_z[0]);

					roll = cal_roll(fir_out_y[SIZE - 1], fir_out_z[SIZE - 1]);
					pitch = cal_pitch(fir_out_x[SIZE - 1], fir_out_y[SIZE - 1], fir_out_z[SIZE - 1]);
					led_orientation(roll, pitch);

					if (stream_mode == 0) {
						// stream_mode = 0 -> stream acc data
						printf("X: %d\tY: %d\tZ: %d\r\n", accData[SIZE - 1][0], accData[SIZE - 1][1], accData[SIZE - 1][2]);
						printf("X: %9f\tY: %9f\tZ: %9f\r\n", fir_out_x[SIZE - 1], fir_out_y[SIZE - 1], fir_out_z[SIZE - 1]);
					} else {
						// stream_mode = 1 -> stream (roll, pitch) data
						printf("ROLL: %10f\t\tPITCH: %10f\r\n", roll, pitch);
					}

					printf("------------------------------------------------------------\r\n");
					idx = 0;
				}
			}
		}
	}

	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

/** Configure the main internal regulator output voltage
 */
__HAL_RCC_PWR_CLK_ENABLE();
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
/** Initializes the RCC Oscillators according to the specified parameters
 * in the RCC_OscInitTypeDef structure.
 */
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_ON;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
RCC_OscInitStruct.PLL.PLLM = 8;
RCC_OscInitStruct.PLL.PLLN = 336;
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
RCC_OscInitStruct.PLL.PLLQ = 4;
if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
	Error_Handler();
}
/** Initializes the CPU, AHB and APB buses clocks
 */
RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
	Error_Handler();
}
}

/* USER CODE BEGIN 4 */

/*
 * @brief 			turns off all leds
 * @return 			none
 */
void led_off() {
	// reset all leds
	LL_GPIO_ResetOutputPin(GPIOD, GPIO_PIN_12);
	LL_GPIO_ResetOutputPin(GPIOD, GPIO_PIN_13);
	LL_GPIO_ResetOutputPin(GPIOD, GPIO_PIN_14);
	LL_GPIO_ResetOutputPin(GPIOD, GPIO_PIN_15);
}

/*
 * @brief 			turn on lowest led based on roll and pitch
 * @param[roll]		pitch angle
 * @param[pitch] 	roll angle
 * @return 			none
 */
void led_orientation(float32_t roll, float32_t pitch) {
	// reset all leds
	led_off();
	unsigned int to_turn_on = -1;
	// introduce tolerance - dead zone
	if(roll > ACC_TOL) {
		// front
		to_turn_on = GPIO_PIN_13; //orange
	} else if (roll < -ACC_TOL){
		//back
		to_turn_on = GPIO_PIN_15; // blue
		roll = -roll;
	}
	if(pitch > ACC_TOL) {
		// may be sx
		if(pitch > roll) {
			to_turn_on = GPIO_PIN_12; //green
		}
	} else if (pitch < -ACC_TOL) {
		// may be dx
		if(-pitch > roll) {
			to_turn_on = GPIO_PIN_14; //red
		}
	}

	// if pitch == 0 and roll == 0 no led turns on
	if (to_turn_on != -1) {
		// then set only the chosen one
		LL_GPIO_SetOutputPin(GPIOD, to_turn_on);
	}
}

/*
 * This function finds the maximum value in an array and returns its index in the array.
 * @param 	a[]		array to find maximum
 * @param 	n 		size of the array
 * @return 	index 	array index of maximum value in array
 */

int __io_putchar(int ch) {
/* Place your implementation of fputc here */
/* e.g. write a character to the USART */
LL_USART_TransmitData8(USART2, (uint8_t) ch);

/* Loop until the end of transmission */
while (LL_USART_IsActiveFlag_TC(USART2) == 0) {
}

return ch;
}

void Acc_Config(void) {
LIS3DSH_InitTypeDef AccInitStruct;

AccInitStruct.Output_DataRate = LIS3DSH_DATARATE_25;
AccInitStruct.Axes_Enable = LIS3DSH_XYZ_ENABLE;
AccInitStruct.SPI_Wire = LIS3DSH_SERIALINTERFACE_4WIRE;
AccInitStruct.Self_Test = LIS3DSH_SELFTEST_NORMAL;
AccInitStruct.Full_Scale = LIS3DSH_FULLSCALE_2;
AccInitStruct.Filter_BW = LIS3DSH_FILTER_BW_800;

LIS3DSH_Init(&AccInitStruct);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
/* USER CODE BEGIN Error_Handler_Debug */
/* User can add his own implementation to report the HAL error return state */
__disable_irq();
while (1) {
}
/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
