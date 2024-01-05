/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

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
#define Y1_Pin GPIO_PIN_2
#define Y1_GPIO_Port GPIOE
#define X2_Pin GPIO_PIN_13
#define X2_GPIO_Port GPIOC
#define X0_Pin GPIO_PIN_0
#define X0_GPIO_Port GPIOA
#define X5_Pin GPIO_PIN_2
#define X5_GPIO_Port GPIOH
#define X6_Pin GPIO_PIN_3
#define X6_GPIO_Port GPIOH
#define Y4_Pin GPIO_PIN_5
#define Y4_GPIO_Port GPIOA
#define X8_Pin GPIO_PIN_12
#define X8_GPIO_Port GPIOF
#define X9_Pin GPIO_PIN_13
#define X9_GPIO_Port GPIOF
#define X10_Pin GPIO_PIN_14
#define X10_GPIO_Port GPIOF
#define X11_Pin GPIO_PIN_15
#define X11_GPIO_Port GPIOF
#define X12_Pin GPIO_PIN_0
#define X12_GPIO_Port GPIOG
#define X13_Pin GPIO_PIN_1
#define X13_GPIO_Port GPIOG
#define ETH_NRST_Pin GPIO_PIN_6
#define ETH_NRST_GPIO_Port GPIOH
#define Y6_Pin GPIO_PIN_7
#define Y6_GPIO_Port GPIOH
#define X7_Pin GPIO_PIN_8
#define X7_GPIO_Port GPIOH
#define UART4_RE_Pin GPIO_PIN_9
#define UART4_RE_GPIO_Port GPIOH
#define X14_Pin GPIO_PIN_11
#define X14_GPIO_Port GPIOD
#define X1_Pin GPIO_PIN_2
#define X1_GPIO_Port GPIOG
#define X3_Pin GPIO_PIN_3
#define X3_GPIO_Port GPIOG
#define X4_Pin GPIO_PIN_4
#define X4_GPIO_Port GPIOG
#define X15_Pin GPIO_PIN_5
#define X15_GPIO_Port GPIOG
#define SPI3_CS_Pin GPIO_PIN_6
#define SPI3_CS_GPIO_Port GPIOG
#define Y0_Pin GPIO_PIN_15
#define Y0_GPIO_Port GPIOA
#define Y5_Pin GPIO_PIN_12
#define Y5_GPIO_Port GPIOC
#define Y7_Pin GPIO_PIN_3
#define Y7_GPIO_Port GPIOD
#define X16_Pin GPIO_PIN_7
#define X16_GPIO_Port GPIOD
#define Y2_Pin GPIO_PIN_15
#define Y2_GPIO_Port GPIOG
#define Y3_Pin GPIO_PIN_8
#define Y3_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
