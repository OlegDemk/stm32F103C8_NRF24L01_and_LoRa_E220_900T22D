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

extern bool flag_command_received;

void init_lora(void);
void test_uart(void);




//----------------------------------------------------------------------------------------
void lora_test_module(void)
{



}
//----------------------------------------------------------------------------------------
void init_lora(void)
{
	HAL_Delay(1000);

	// Set "Normal mode"
	HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);

	HAL_Delay(100);

	uint8_t data[10] = {0};

	HAL_Delay(2000);

	data[0] = 0xC0;
	data[1] = 0x00;		// Starting address
	data[2] = 0x03;		// Length
	data[3] = 0x12;		// 00H ADD H
	data[4] = 0x34;		// 01H ADD L
	data[5] = 0x62;		// 02H register ()

	HAL_UART_Transmit_IT(&huart1, data, 6);
	HAL_Delay(1000);
	memset(data, 0, sizeof(data));


	data[0] = 0xC0;
	data[1] = 0x05;		// Starting address
	data[2] = 0x01;		// Length
	data[3] = 0x00;		//

	HAL_UART_Transmit_IT(&huart1, data, 4);
	HAL_Delay(1000);


	//	data[0] = 0xC1;
	//	data[1] = 0x04;
	//	data[2] = 0x01;
	//
	//	HAL_UART_Transmit_IT(&huart1, data, 3);
	//
	//	HAL_Delay(2000);
	memset(data, 0, sizeof(data));


	read_settings_from_module();
	HAL_Delay(1000);




	// Set Receive mode
	HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);

	HAL_Delay(100);







//перевірити які дані зчитуються
//зробити окремі функціі на зчитування даних
//і виводити дані на екран не сирі дані а те що вони означають
//
//		Розібратись з:
//		1. Некоректним пересиланням даних
//		2. чому так довго AUX в 0
		// Read module address, serial port, and airspeed
//		memset(data, 0, sizeof(data));
//		data[0] = 0xC1;
//		data[1] = 0x00;
//		data[2] = 0x06;
//
//		HAL_UART_Transmit_IT(&huart1, data, 3);



}
//-------------------------------------------------------------------------------------------------
void read_settings_from_module(void)
{
	// Turn on configuration mode
	HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	uint8_t data[10] = {0};
	// Read module address, serial port, and airspeed COMMAND
	data[0] = 0xC1;
	data[1] = 0x00;
	data[2] = 0x08;

//	while(1)
//	{
		HAL_UART_Transmit_IT(&huart1, data, 3);
		HAL_Delay(1000);
//	}


	// Return:
	// 0xC1 0x00 0x03 0x12 0x34 0x62
	// 0x12 0x34 - Adders 1234
	// 0x62 - 9600, 8n1 and 2,4 k air data rate
}
//-------------------------------------------------------------------------------------------------

















//-------------------------------------------------------------------------------------------------

