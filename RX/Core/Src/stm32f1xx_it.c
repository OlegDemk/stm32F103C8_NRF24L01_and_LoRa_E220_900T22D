/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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

#include <menu/menu.h>
#include <am2302/am2302.h>

#define detected 0

extern uint8_t utton_enter_pressed_flag;
extern uint8_t button_status;						// Current button status
bool block_interrupt_form_up_and_down_buttons;		// Flag for lock interrupt from 'up' and 'down' buttons in some cases

int delay_time = 0;									//	Counter
uint8_t doesent_detected = 1;
uint8_t dalay_duration = 5;

int button_processed_status = 1;					// For interrupt work only one time

bool am2302_ready = false;
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
extern TIM_HandleTypeDef htim2;
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
	int stack_size_1 = check_stack_size();
//	 unsigned stack_size = 0;
//	stack_size = check_stack_size();
	/////////////////////////////////////////////////////////////////////////////////////////////
//	 struct
//	  {
//	    uint32_t r0;
//	    uint32_t r1;
//	    uint32_t r2;
//	    uint32_t r3;
//	    uint32_t r12;
//	    uint32_t lr;
//	    uint32_t pc;
//	    uint32_t psr;
//	  }*stack_ptr; 							//?????????????????? ???? ?????????????? ???????????????? ??????????(SP)
//
//	  asm(
//	      "TST lr, #4 \n" 					//?????????????????? 3???? ?????? ?????????????????? ??????????(?????????????????? ??)
//	      "ITE EQ \n"   					//???????????????? ?????????????????? ?????????? ?????????? ?????? 3?
//	      "MRSEQ %[ptr], MSP  \n"  			//????, ?????????????????? ???????????????? ?????????????????? ??????????
//	      "MRSNE %[ptr], PSP  \n"  			//??????, ?????????????????? ?????????????????? ?????????? ????????????????
//	      : [ptr] "=r" (stack_ptr)
//	      );
	/////////////////////////////////////////////////////////////////////////////////////////////
	tFailureData failureData;

	failureData.ACTLR = (* ((volatile uint32_t *) (0xe000e008)));
	failureData.CPUID = (* ((volatile uint32_t *) (0xe000ed00)));
	failureData.ICSR  = (* ((volatile uint32_t *) (0xe000ed04)));
	failureData.VTOR  = (* ((volatile uint32_t *) (0xe000ed08)));
	failureData.AIRCR = (* ((volatile uint32_t *) (0xe000ed0c)));
	failureData.SCR   = (* ((volatile uint32_t *) (0xe000ed10)));
	failureData.CCR   = (* ((volatile uint32_t *) (0xe000ed14)));
	failureData.SHPR1 = (* ((volatile uint32_t *) (0xe000ed18)));
	failureData.SHPR2 = (* ((volatile uint32_t *) (0xe000ed1c)));
	failureData.SHPR3 = (* ((volatile uint32_t *) (0xe000ed20)));
	failureData.SHCRS = (* ((volatile uint32_t *) (0xe000ed24)));
	failureData.CFSR.u32  = (* ((volatile uint32_t *) (0xe000ed28)));
	failureData.HFSR  = (* ((volatile uint32_t *) (0xe000ed2c)));
	failureData.MMAR  = (* ((volatile uint32_t *) (0xe000ed34)));
	failureData.BFAR  = (* ((volatile uint32_t *) (0xe000ed38)));
	failureData.AFSR  = (* ((volatile uint32_t *) (0xe000ed3c)));

	failureData.DFSR = (* ((volatile uint32_t *) (0xe000ed30)));

	/////////////////////////////////////////////////////////////////////////////////////////////

	  int hhh = 99;
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
	if(block_interrupt_form_up_and_down_buttons == false)				// Block wen function running
	{
		if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_8))						// If interrupt from GPIOA PIN_8
			{
				if(button_processed_status == doesent_detected)
				{
					HAL_TIM_Base_Start_IT(&htim1);							// Turn on Timer 1
					button_processed_status = detected;						// For interrupt work only one time
				}
			}
	}

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt.
  */
void TIM1_UP_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_IRQn 0 */
	/* This timmer start by external interrupts from buttons:
	 * EXTI9_5_IRQHandler and EXTI15_10_IRQHandler
	 */

//	static int delay_time = 0;										//	Counter

	if(button_processed_status == detected)							// If pressed button was detected by external interrupts
	{
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == 0)    			// If "UP" button was pressed
		{
			/*
			 * If every time when timer interrupt, delay_time will increment
			 * for avoid bounce button
			 */
			delay_time++;

			if(delay_time >= dalay_duration)						// if button pressed more than dalay_duration time it mean button was pressed
			{
				button_processed_status = 1;						// Flag for interrupts
				HAL_TIM_Base_Stop_IT(&htim1);						// Stop timer, because timer has done work above, and timer don't need

				button_status = BOTTON_UP;
				delay_time = 0;
			}
		}

		else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == 0)   			 	// If "DOWN" button was pressed
		{
			/*
			* If every time when timer interrupt, delay_time will increment
			* for avoid bounce button
			*/
			delay_time++;

			if(delay_time >= dalay_duration)						// if button pressed more than dalay_duration time it mean button was pressed
			{
				button_processed_status = 1;						// Flag for interrupts
				HAL_TIM_Base_Stop_IT(&htim1);						// Stop timer, because timer has done work above, and timer don't need

				button_status = BUTTON_DOWN;
				delay_time = 0;
			}
		}

		else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == 0)   			// If "ENTER" button was pressed
		{
			/*
			* If every time when timer interrupt, delay_time will increment
			* for avoid bounce button
			*/
			delay_time++;

			if(delay_time >= dalay_duration)						// if button pressed more than dalay_duration time it mean button was pressed
			{
				button_processed_status = 1;
				button_status = BUTTON_ENTER;
				delay_time = 0;
				HAL_TIM_Base_Stop_IT(&htim1);						// Stop timer, because timer has done work above, and timer don't need
			}
		}
		else
		{
			delay_time = 0;
		}

	}

  /* USER CODE END TIM1_UP_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_IRQn 1 */

  /* USER CODE END TIM1_UP_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
	static int i = 0;
	if(i == 200)								// Do it every 2 seconds
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		am2302_measure();
		i = 0;
		am2302_ready = !am2302_ready;
	}
	i++;

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
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
	if(block_interrupt_form_up_and_down_buttons == false)				// Block wen function running
	{
		// Detect "UP" button
		if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_14))	// If interrupt from GPIOB PIN_14
		{
			if(button_processed_status == doesent_detected)
			{
				HAL_TIM_Base_Start_IT(&htim1);		// Turn on Timer 1
				button_processed_status = detected;						// For interrupt work only one time
			}
		}
	}

	// Detect "ENTER" button
	if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_15))	// If interrupt from GPIOB PIN_15
	{
		if(button_processed_status == doesent_detected)
		{
			HAL_TIM_Base_Start_IT(&htim1);		// Turn on Timer 1
			button_processed_status = detected;						// For interrupt work only one time
		}
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
