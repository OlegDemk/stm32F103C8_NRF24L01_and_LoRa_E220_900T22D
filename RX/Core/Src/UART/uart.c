/*
 * uart.c
 *
 *  Created on: Jul 19, 2021
 *      Author: odemki
 */


#include "main.h"
#include <string.h>
#include <stdbool.h>

extern UART_HandleTypeDef huart1;

void simple_test_transmit_uart(void);
void simple_test_receive_uart(void);

//----------------------------------------------------------------------------------------
void test_uart()
{
	//simple_test_transmit_uart();
	simple_test_receive_uart();

}
//----------------------------------------------------------------------------------------
void simple_test_transmit_uart(void)
{
	int i = 0;
	uint8_t status = 99;
	char data[15] = "Test send: ";
	char number[7] = {0};
	char nl[5] = "\n\r";
	char buffer[30] = {0};
	while(i < 100000)
	{
		memset(buffer, 0, sizeof(buffer));

		// convert number on string
		itoa(i, number, 10);
		// Add data to buffer
		strcat(buffer, data);
		strcat(buffer, number);
		strcat(buffer,nl);

		// Method 1
		//while(HAL_UART_Transmit_IT(&huart1, buffer, sizeof(buffer)) == HAL_BUSY);

		// Method 2  Waiting for end of previous transmission
		while(HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX);
		HAL_UART_Transmit_IT(&huart1, buffer, sizeof(buffer));

		i++;
		//HAL_Delay(100);
	}
}
//----------------------------------------------------------------------------------------
void simple_test_receive_uart(void)
{
	///////////////////////////
	// Method 1 ( Echo )  Receive and transmit data
//	char str[3] = {};
//	while(1)
//	{
//		if(HAL_UART_Receive_IT(&huart1, str, 1) != HAL_BUSY )				// Receive data from comport (One char)
//		{
//			while( HAL_UART_Transmit_IT(&huart1, str, 1) == HAL_BUSY );     // Transmit data to comport (One char)
//		}
//	}
	///////////////////////////

	// Method 2
	// Read one char from comport
	// Receive and transmit data
//	char str[3] = {0};
//	while(1)
//	{
//		HAL_UART_Receive_IT(&huart1, str, 1);
//		if(str[0] == '1')								// If received '1'
//		{
//			int h = 0;
//			HAL_UART_Transmit_IT(&huart1, str, 1);		// Transmit data back to comport
//			memset(str, 0, sizeof(str));
//		}
//
//		if(str[0] == '2')								// If received '2'
//		{
//			int e = 0;
//			HAL_UART_Transmit_IT(&huart1, str, 1);		// Transmit data back to comport
//			memset(str, 0, sizeof(str));
//		}
//	}
	///////////////////////////

	// Method 3


}
//----------------------------------------------------------------------------------------
