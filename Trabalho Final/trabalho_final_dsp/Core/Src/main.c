/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>

#include "display.h"
#include "fft.h"
#include "fft_tables.h"
#include "ECUAL/LCD16X2/LCD16X2.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define NUM_SAMPLES FFT_TABLE_N
#define SAMPLE_FREQUENCY FFT_TABLE_FS
#define FREQ_SCALE FFT_FREQ_SCALE
#define MAG_THRESHOLD 100

#define ADC_HANDLE hadc1
#define DMA_HANDLE hdma2
#define TIM_HANDLE htim2

#define LCD_HANDLE LCD16X2_1

#define DISPLAY_BUFFER_SIZE 17

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t samples[2][NUM_SAMPLES] = {0};
uint8_t sampling_buffer_index = 0;
bool pending_process = false;

int16_t real_buffer[NUM_SAMPLES] = {0};
int16_t imaginary_buffer[NUM_SAMPLES] = {0};
uint16_t mag_buffer[NUM_SAMPLES/2 + 1] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  LCD16X2_Init(LCD_HANDLE);
  LCD16X2_Clear(LCD_HANDLE);
  HAL_Delay(1000);
  LCD16X2_Set_Cursor(LCD_HANDLE, 1, 1);
  LCD16X2_Write_String(LCD_HANDLE, "  Teste");
  HAL_Delay(1000);
  LCD16X2_Set_Cursor(LCD_HANDLE, 2, 1);
  LCD16X2_Write_String(LCD_HANDLE, "do display LCD");
  HAL_Delay(1000);

  HAL_ADC_Start_DMA(&ADC_HANDLE, (uint32_t*)&samples[0][0], NUM_SAMPLES);
	HAL_TIM_Base_Start(&TIM_HANDLE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

  	if(pending_process){
//			uint8_t buffer[25];
//			snprintf(buffer,sizeof(buffer),"Iniciando: %d\n\r", HAL_GetTick());
//			HAL_UART_Transmit(&huart2,buffer , sizeof(buffer), 1000);

  		pending_process = false;
  		const uint8_t processing_buffer_index = sampling_buffer_index ^ 0b1;
			uint32_t peak_mag_index;
			if(!fft(&samples[processing_buffer_index][0],
					mag_buffer,
					NUM_SAMPLES,
					real_buffer,
					imaginary_buffer,
					&peak_mag_index
			)){
				uint8_t buffer[25] = "Erro ao realizar FFT\0";
				HAL_UART_Transmit(&huart2,buffer , sizeof(buffer), 1000);
				break;
			}

			char lines[2][DISPLAY_BUFFER_SIZE] = {'\0'};
			strncpy(&lines[0][0],"Sem sinal\0",DISPLAY_BUFFER_SIZE);

			if(mag_buffer[peak_mag_index] > MAG_THRESHOLD){
				const uint32_t frequency = frequency_bins[peak_mag_index];
				parse_display(frequency, &lines[0][0], &lines[1][0]);
			}

		  LCD16X2_Set_Cursor(LCD_HANDLE, 1, 1);
		  LCD16X2_Write_String(LCD_HANDLE, &lines[0][0]);
		  LCD16X2_Set_Cursor(LCD_HANDLE, 2, 1);
		  LCD16X2_Write_String(LCD_HANDLE, &lines[1][0]);

//			snprintf(buffer,sizeof(buffer),"Terminando: %d\n\r", HAL_GetTick());
//			HAL_UART_Transmit(&huart2,buffer , sizeof(buffer), 1000);
  	}

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	if(hadc == &ADC_HANDLE){
    sampling_buffer_index ^= 0b1;
    pending_process = true;
    HAL_ADC_Start_DMA(&ADC_HANDLE,(uint32_t*)&samples[sampling_buffer_index][0],NUM_SAMPLES);
	}
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
