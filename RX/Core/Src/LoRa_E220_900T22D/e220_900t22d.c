/*
 * e220_900t22d.c
 *
 *  Created on: Jul 17, 2021
 *      Author: odemki
 */

#include "main.h"
#include <string.h>
#include <stdbool.h>


extern UART_HandleTypeDef huart1;

void init_lora(void);
void test_uart(void);


//----------------------------------------------------------------------------------------
void lora_test_module(void)
{
	//init_lora();



//	while(1)
//	{
//
//	}

}
//----------------------------------------------------------------------------------------
void init_lora(void)
{
	// Set "Normal mode"
	HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);

	// Команди надсилаються в 10-ві формі
	uint8_t data[10] = {0};
	data[0] = 0;
	data[1] = 3;
	data[2] = 12;
	data[3] = 34;
	data[4] = 62;

	//HAL_UART_Transmit(&huart3, data, 5, 1000);

//	  HAL_OK       = 0x00U,
//	  HAL_ERROR    = 0x01U,
//	  HAL_BUSY     = 0x02U,
//	  HAL_TIMEOUT  = 0x03U
	uint8_t status = 99;


	uint8_t RX_data[6] = {0};


	//HAL_UART_Receive_IT(&huart3, RX_data, 5);


	int g = 0;

}



