/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OLED_DC_Pin GPIO_PIN_0
#define OLED_DC_GPIO_Port GPIOA
#define OLED_RES_Pin GPIO_PIN_1
#define OLED_RES_GPIO_Port GPIOA
#define OLED_SDIN_Pin GPIO_PIN_2
#define OLED_SDIN_GPIO_Port GPIOA
#define OLED_SCLK_Pin GPIO_PIN_3
#define OLED_SCLK_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_4
#define LED_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_0
#define SPI1_CS_GPIO_Port GPIOB
#define KEY3_Pin GPIO_PIN_1
#define KEY3_GPIO_Port GPIOB
#define KEY3_EXTI_IRQn EXTI0_1_IRQn
#define IRQ_Pin GPIO_PIN_8
#define IRQ_GPIO_Port GPIOA
#define IRQ_EXTI_IRQn EXTI4_15_IRQn
#define NRST_Pin GPIO_PIN_9
#define NRST_GPIO_Port GPIOA
#define WAKE_UP_Pin GPIO_PIN_10
#define WAKE_UP_GPIO_Port GPIOA
#define WAKE_UP_EXTI_IRQn EXTI4_15_IRQn
#define KEY2_Pin GPIO_PIN_15
#define KEY2_GPIO_Port GPIOA
#define KEY2_EXTI_IRQn EXTI4_15_IRQn
#define KEY1_Pin GPIO_PIN_3
#define KEY1_GPIO_Port GPIOB
#define KEY1_EXTI_IRQn EXTI2_3_IRQn

/* USER CODE BEGIN Private defines */
#define DI1_Pin GPIO_PIN_6
#define DI1_GPIO_Port GPIOB
#define DI2_Pin GPIO_PIN_7
#define DI2_GPIO_Port GPIOB


//#define DEBUG 1

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
