/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <state_machine/state_machine.h>
extern uint8_t button_up_or_down_was_pressed_flag;
extern uint8_t utton_enter_pressed_flag;

extern uint8_t pppp;
extern int pressed_batton_counter;

int i = 0;
int new_press = 1;


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_IRQn 0 */

  /* USER CODE END EXTI2_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_IRQn 1 */

  /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
	// Detect "DOWN" button
	if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_8))
	{
		if(new_press == 1)
		{
			HAL_TIM_Base_Start_IT(&htim1);		// Вмикає таймер
			new_press = 0;
		}
	}


//		if(utton_enter_pressed_flag == 0)		// If menu doesen't enter
//		{
//
////			if(pppp == 1)
////			{
//				pppp = 0;
//				pressed_batton_counter++;				// For debug
//				button_up_or_down_was_pressed_flag = 1;
//				switch(state_get())
//				{
//					case ST_1:
//						state_set(ST_2);
//						break;
//					case ST_2:
//						state_set(ST_3);
//						break;
//					case ST_3:
//						state_set(ST_4);
//						break;
//					case ST_4:
//						state_set(ST_1);
//						break;
//
//				}
//			}

//		}
//	}
  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt.
  */
void TIM1_UP_IRQHandler(void)			// Period = 1 msec
{
  /* USER CODE BEGIN TIM1_UP_IRQn 0 */

	if(new_press == 0)		// Якщо зафіксований перши наростаючий фронт
	{
		// UP BUTTON
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == 0)    // якщо кнопка натиснута
		{
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == 0)    // якщо кнопка натиснута
			{
				i++;
			}
			if(i >= 10)		// Кнопка вваажється натиснутою, якщо вона утримана більше 100 мс
			{
				// button UP = 1 (Pressed)
				i = 0;
				new_press = 1;
				HAL_TIM_Base_Stop_IT(&htim1);

				pppp++;

				if(utton_enter_pressed_flag == 0)		// If menu doesen't enter
				{
						button_up_or_down_was_pressed_flag = 1;
						switch(state_get())
						{
							case ST_1:
								state_set(ST_4);
								break;
							case ST_2:
								state_set(ST_1);
								break;
							case ST_3:
								state_set(ST_2);
								break;
							case ST_4:
								state_set(ST_3);
							break;
						}
				}
			}
		}

		// DOWN BUTTON
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 0)    // якщо кнопка натиснута
		{
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 0)    // якщо кнопка натиснута
			{
				i++;
			}
			if(i >= 10)		// Кнопка вваажється натиснутою, якщо вона утримана більше 100 мс
			{
							// button UP = 1 (Pressed)
							i = 0;
							new_press = 1;
							HAL_TIM_Base_Stop_IT(&htim1);

							if (pppp >0)
							{
								pppp--;
							}


							if(utton_enter_pressed_flag == 0)		// If menu doesen't enter
							{
									button_up_or_down_was_pressed_flag = 1;
									switch(state_get())
									{
										case ST_1:
											state_set(ST_2);
											break;
										case ST_2:
											state_set(ST_3);
											break;
										case ST_3:
											state_set(ST_4);
											break;
										case ST_4:
											state_set(ST_1);
											break;
									}
							}
			}
		}

		// DOWN ENTER
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == 0)    // якщо кнопка натиснута
		{

			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == 0)    // якщо кнопка натиснута
			{
				i++;
			}
			if(i >= 10)		// Кнопка вваажється натиснутою, якщо вона утримана більше 100 мс
			{
				// button UP = 1 (Pressed)
				i = 0;
				new_press = 1;

				button_up_or_down_was_pressed_flag = 1;
				utton_enter_pressed_flag = !utton_enter_pressed_flag;
				HAL_TIM_Base_Stop_IT(&htim1);
			}


		}

	}

  /* USER CODE END TIM1_UP_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_IRQn 1 */

  /* USER CODE END TIM1_UP_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

	if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_14))
	{
		if(new_press == 1)
		{
			HAL_TIM_Base_Start_IT(&htim1);		// Вмикає таймер
			new_press = 0;
		}
	}


//	uint8_t count = 0;
//	// Detect "UP" button
//	if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_14))
//	{
//		//i = 0;
//		//new_press = 1;						// Зафіксовано перший зростаючий фронт
//		//HAL_TIM_Base_Start_IT(&htim1);		// Вмикає таймер
//		if(utton_enter_pressed_flag == 0)		// If menu doesen't enter
//		{
//			button_up_or_down_was_pressed_flag = 1;
//			switch(state_get())
//			{
//				case ST_1:
//					state_set(ST_4);
//					break;
//				case ST_2:
//					state_set(ST_1);
//					break;
//				case ST_3:
//					state_set(ST_2);
//					break;
//				case ST_4:
//					state_set(ST_3);
//					break;
//			}
//		}
//
//	}

	// Detect "ENTER" button
	if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_15))
	{
		if(new_press == 1)
		{
			HAL_TIM_Base_Start_IT(&htim1);		// Вмикає таймер
			new_press = 0;
		}
//		button_up_or_down_was_pressed_flag = 1;
//		utton_enter_pressed_flag = !utton_enter_pressed_flag;
	}

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
