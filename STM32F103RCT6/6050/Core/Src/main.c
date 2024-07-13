/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "MPU6050.h"
#include "quater.h"
#include "mpu6050_reg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static int time;
	if(htim -> Instance == TIM3)
	{
			time += 2;
			if(time % 8 ==0 )
			{
				time = 0;
				imu_task();
			}
	}
}

//重写fputc
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,50);
	return ch;
}
/* USER CODE END 0 */

int flag = 1;
uint32_t sample_count = 0;
int16_t tAX, tAY, tAZ, tGX, tGY, tGZ;

// void zat_getData(void){
//   printf("\r\n%d\r\n", sample_count);
//   MPU6050_GetData(&tAX, &tAY, &tAZ, &tGX, &tGY, &tGZ);
//   if((abs(tAX - AX) > 10) || (abs(tAY - AY) > 10) || (abs(tAZ - AZ) > 10)){
//     HAL_Delay(10);
//     sample_count = 0;
//     flag = 1;
//     MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
//   }else{
//     sample_count++;
//     if(sample_count > 30){
//       MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x20);
//       MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x07);
//       flag = 2;
//     }
//   }
// }

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
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	MPU6050_Init();
	HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t i = 0, j = 20;
  // MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x20);
  // MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x47);
  // int close_flag = 0;
  // int normal_flag = 1;
  // int timer_flag = 2;
  while (1)
  {
		uint8_t ch;
		//发送数据
    

    if(flag == 0){
      HAL_Delay(100);
    }else if(flag == 1){
      MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
      printf("AX:%d AY:%d AZ:%d GX:%d GY:%d GZ:%d\r\n",AX, AY, AZ, GX, GY, GZ);
      printf("pitch:%.1f roll:%.1f \r\n",eulerAngle.pitch, eulerAngle.roll);
      HAL_Delay(100);
    }else if(flag == 2){
      if((i++ % j) == 0){
        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
        printf("AX:%d AY:%d AZ:%d GX:%d GY:%d GZ:%d\r\n",AX, AY, AZ, GX, GY, GZ);
        printf("pitch:%.1f roll:%.1f \r\n",eulerAngle.pitch, eulerAngle.roll);
        HAL_Delay(100);
      }else{
        HAL_Delay(100);
      }
    }

		HAL_UART_Receive(&huart1, &ch, 1, 0);

		if(ch =='o'){
      flag = 1;
			MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
			MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
		}
		else if(ch == 'c'){
      flag = 0;
			MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x40);
		}else if(ch == 't'){
      flag = 2;
    }else if(ch == 'l'){
      MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x20);
      MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x07);
    }
    /* USER CODE END WHILE */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
