/*
 * e220_900t22d.c
 *
 *  Created on: Aug 6, 2021
 *      Author: odemki
 */


#include "main.h"
#include <string.h>
#include <stdbool.h>


extern UART_HandleTypeDef huart1;

extern bool flag_command_received;

void init_lora(void);
void test_uart(void);
void TX_LoRa(void);


//----------------------------------------------------------------------------------------
int lora_test_module(void)
{
	static int transmeet_count = 0;
	uint8_t data[10] = {0};

	transmeet_count++;


	data[5] = '0' + transmeet_count%10;
	data[4] = '0' + (transmeet_count/10) % 10;
	data[3] = '0' + (transmeet_count/100) % 10;
	data[2] = '0' + (transmeet_count/1000) % 10;
	data[1] = '0' + (transmeet_count/10000) % 10;
	data[0] = '0' + (transmeet_count/100000) % 10;
	data[6] = '\0';


	HAL_UART_Transmit_IT(&huart1, data, 7);
	HAL_Delay(100);

	return transmeet_count;
}
//----------------------------------------------------------------------------------------
void init_lora(void)
{
	HAL_Delay(1000);

		// Set "Normal mode"
		HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);

		HAL_Delay(10);

		uint8_t data[10] = {0};

		//HAL_Delay(2000);

		// Init module
		// Descripe settings structure
		data[0] = 0xC0;
		data[1] = 0x00;
		data[2] = 0x03;
		data[3] = 0x12;
		data[4] = 0x34;
		data[5] = 0x62;

		HAL_UART_Transmit_IT(&huart1, data, 6);

		HAL_Delay(1000);
		//memset(data, 0, sizeof(data));


		data[0] = 0xC0;
		data[1] = 0x05;		// Starting address
		data[2] = 0x01;		// Length
		data[3] = 0x00;		//

		HAL_UART_Transmit_IT(&huart1, data, 4);
		HAL_Delay(1000);



		read_settings_from_module();
		HAL_Delay(1000);


		// Set Transmitting mode
		HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_RESET);

		HAL_Delay(10);

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
	data[2] = 0x03;

	HAL_UART_Transmit_IT(&huart1, data, 3);

	// Return:
	// 0xC1 0x00 0x03 0x12 0x34 0x62
	// 0x12 0x34 - Adders 1234
	// 0x62 - 9600, 8n1 and 2,4 k air data rate
}
//-------------------------------------------------------------------------------------------------
void test(void)
{
	static int transmeet_count = 0;
	uint8_t data[10] = {0};

	data[0] = '0' + transmeet_count%10;
	data[1] = '0' + (transmeet_count/10) % 10;
	data[2] = '0' + (transmeet_count/100) % 10;

	//data[6] = '\0';


	transmeet_count++;

	HAL_Delay(100);
}
//-------------------------------------------------------------------------------------------------

