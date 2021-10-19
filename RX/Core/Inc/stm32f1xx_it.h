/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
 typedef struct {
//     uint32_t stacked_r0;
//     uint32_t stacked_r1;
//     uint32_t stacked_r2;
//     uint32_t stacked_r3;
//     uint32_t stacked_r12;
//     uint32_t stacked_lr;
//     uint32_t stacked_pc;
//     uint32_t stacked_psr;

     /*
      * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0552a/Cihdjcfc.html
      * Cortex-M3 Devices Generic User Guide
      * 4.3. System control block
      */
     uint32_t ACTLR;
     uint32_t CPUID;
     uint32_t ICSR;
     uint32_t VTOR;
     uint32_t AIRCR;
     uint32_t SCR;
     uint32_t CCR;
     uint32_t SHPR1;
     uint32_t SHPR2;
     uint32_t SHPR3;
     uint32_t SHCRS;

     union  {
         uint32_t u32;
         struct {
             uint8_t MMSR;
             uint8_t BFSR;
             uint16_t UFSR;
         } subregisters;
     } CFSR;

     uint32_t HFSR;
     uint32_t MMAR;
     uint32_t BFAR;
     uint32_t AFSR;

     /*
      * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337h/BABJHEIG.html
      * Cortex-M3 Technical Reference Manual
      * 7.1.3. Debug register summary
      */
     uint32_t DFSR;

     uint32_t control;
 } tFailureData;
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
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void EXTI2_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void TIM1_UP_IRQHandler(void);
void TIM2_IRQHandler(void);
void USART1_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
