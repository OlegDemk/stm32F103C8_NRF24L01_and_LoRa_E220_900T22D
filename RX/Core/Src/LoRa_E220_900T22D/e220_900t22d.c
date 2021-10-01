/*
 * e220_900t22d.c
 *
 *  Created on: Jul 17, 2021
 *      Author: odemki
 */

#include "main.h"
#include <string.h>
#include <stdbool.h>

#include <OLED/fonts.h>
#include <OLED/oled_ssd1306.h>
#include <OLED/ssd1306.h>
#include <OLED/oled_main.h>

#include <keyboard/keyboard.h>
#include <am2302/am2302.h>

extern UART_HandleTypeDef huart1;

extern bool flag_command_received;			// Flag show status receive data (completed/not completed)
extern char uart_rx_data[50];				// Main rx buffer data
extern char str[1];							// Buffer for one char (It uses for HAL_UART_Receive_IT)
int transmit_count = 1;						// Variable for transmit
int tx_lora_data = 0;						// Test TX data

bool init_lora_RX(void);
bool init_lora_TX(void);
void lora_transmit_data(int transmit_count);
void read_all_settings_from_module(void);
//void read_settings_from_module(void);
void set_config_deep_sleep_mode (void);
void set_WOR_RX_mode (void);
void set_WOR_TX_mode (void);

void LoRa_TX_send_test_number(bool flag);
void LoRa_TX_send_T_and_H(bool flag);

//----------------------------------------------------------------------------------------
// for receiving data from LoRa module using one function
// "flag" needed for start or stop this function
void LoRa_RX(bool flag)
{
	static bool flag_first_time = true;								// Trigger variable
	char str_1[25] = {0};

	if((flag_first_time == true) && (flag == true))					// Do it only first time (init LoRa RX )
	{
		HAL_Delay(100);
		init_lora_RX();
		HAL_Delay(500);

		ssd1306_SetCursor(0, 16);
		strcpy(str_1, "Receiving data:");
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		HAL_UART_Receive_IT(&huart1, str, 1);						// Refresh interrupt
		memset(str_1, 0, sizeof(str_1));
		flag_first_time = false;
	}
	if((flag_first_time == false) && (flag == true))				// Do it when data was received
	{
		if(flag_command_received == true)							// If data is ready
		{
			// Clean data place on OLED
			strcpy(str_1, "                       ");
			ssd1306_SetCursor(0, 28);
			ssd1306_WriteString(str_1,  Font_7x10, White);
			ssd1306_UpdateScreen();

			// Print received data
			// Delete first element of array (because sometimes first element of array can be '\0' it will break down ssd1306_WriteString function)
			ssd1306_SetCursor(0, 28);
			int i = 0;
			for(i = 0; i <= sizeof(str_1); i++)
			{
				if(i == 0)								// Delay first element of array
				{
					i++;
				}
				str_1[i-1] = uart_rx_data[i];
			}
			ssd1306_WriteString(str_1,  Font_7x10, White);
			ssd1306_UpdateScreen();

			HAL_Delay(100);
			flag_command_received = false;							// Set flag. Set show? that data was printed
			memset(uart_rx_data, 0, sizeof(uart_rx_data));			// Cleaning buffer where was received data (From HAL_UART_RxCpltCallback)
			HAL_UART_Receive_IT(&huart1, str, 1);					// Start interrupt again
		}
	}
	if(flag == false)												// Stop function
	{
		flag_first_time = true;
	}
}
// -------------------------------------------------------------------------------
// Sent test one test number and increment it every time. tx_lora_data
void LoRa_TX_send_test_number(bool flag)
{
	static bool flag_first_time = true;								// Trigger variable
	char str_1[20] = {0};

	if((flag_first_time == true) && (flag == true))					// Do it only first time (init)
	{
		memset(uart_rx_data, 0, sizeof(uart_rx_data)); 				// Clean buf

		HAL_Delay(100);
		init_lora_TX();
		HAL_Delay(500);

		ssd1306_SetCursor(0, 16);
		strcpy(str_1, "TX count:");
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		memset(str_1, 0, sizeof(str_1));
		ssd1306_SetCursor(0, 28);
		strcpy(str_1, "Data:");
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		HAL_UART_Receive_IT(&huart1, str, 1);
		flag_first_time = false;
	}
	if((flag_first_time == false) && (flag == true))				// Repeat it part for transmit data
	{
		lora_transmit_data(tx_lora_data);


		// Print transmitter counter
		memset(str_1, 0, sizeof(str_1));
		ssd1306_SetCursor(70, 16);
		sprintf(str_1, "%d", transmit_count);
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		// Print transmitter data
		memset(str_1, 0, sizeof(str_1));
		ssd1306_SetCursor(35, 28);
		sprintf(str_1, "%d", tx_lora_data);
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		tx_lora_data++;
		transmit_count++;											// Increment test data
		HAL_Delay(2000);											// Must be more than 1.5 sec
	}
	if(flag == false)
	{
		flag_first_time = true;
		transmit_count = 1;
		tx_lora_data = 0;											// Every time count will be start from 1
	}
}
//----------------------------------------------------------------------------------------
void lora_transmit_data(int transmit_count)
{
	static uint8_t data[10] = {0};

	data[5] = '0' + transmit_count%10;
	data[4] = '0' + (transmit_count/10) % 10;
	data[3] = '0' + (transmit_count/100) % 10;
	data[2] = '0' + (transmit_count/1000) % 10;
	data[1] = '0' + (transmit_count/10000) % 10;
	data[0] = '0' + (transmit_count/100000) % 10;
	data[6] = '\n';

	HAL_UART_Transmit_IT(&huart1, data, 7);
}
// -------------------------------------------------------------------------------
void LoRa_TX_send_T_and_H(bool flag)   // Зробити пересилання стрінги !!!!
{
	static bool flag_first_time = true;								// Trigger variable
	//static int transmit_count = 0;									// Variable for transmit
	char str_1[20] = {0};

	if((flag_first_time == true) && (flag == true))					// Do it only first time (init)
	{
		memset(uart_rx_data, 0, sizeof(uart_rx_data));//uart_rx_data[50]

		HAL_Delay(100);
		init_lora_TX();
		HAL_Delay(500);

		ssd1306_SetCursor(0, 16);
		strcpy(str_1, "TX count:");
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		memset(str_1, 0, sizeof(str_1));
		ssd1306_SetCursor(0, 28);
		strcpy(str_1, "Transmitting data:");
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		HAL_UART_Receive_IT(&huart1, str, 1);
		flag_first_time = false;
	}
	if((flag_first_time == false) && (flag == true))				// Repeat it part for transmit data
	{
		// Message look like this:
		// counter| T = 25C H = 55%'\n'
		char str_main_buf[35] = {0};
		char str_buf[10] = {0};
		// Add counter to string
		itoa(transmit_count, str_buf, 10);
		strcat(str_main_buf, str_buf);
		memset(str_buf, 0, sizeof(str_buf));
		// Add temperature to string
		strcat(str_main_buf, "| T=");
		itoa(am3202_sensor.temterature, str_buf, 10);
		strcat(str_main_buf, str_buf);
		memset(str_buf, 0, sizeof(str_buf));
		strcat(str_main_buf, "C");
		// Add humidity to string
		strcat(str_main_buf, " H=");
		itoa(am3202_sensor.humidity, str_buf, 10);
		strcat(str_main_buf, str_buf);
		memset(str_buf, 0, sizeof(str_buf));
		strcat(str_main_buf, "%");

		// Print transmitter data
		ssd1306_SetCursor(0, 40);
		ssd1306_WriteString(str_main_buf,  Font_7x10, White);
		ssd1306_UpdateScreen();

		// Add end of line to string
		strcat(str_main_buf, "\n");				// Add stop

		HAL_UART_Transmit_IT(&huart1, str_main_buf, sizeof(str_main_buf));				// Transmitting over LoRa module
		HAL_Delay(2000);

		// Print transmitter counter
		memset(str_1, 0, sizeof(str_1));
		ssd1306_SetCursor(70, 16);
		sprintf(str_1, "%d", transmit_count);
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		transmit_count++;											// Increment test data
		HAL_Delay(2000);											// Must be more than 1.5 sec
	}
	if(flag == false)
	{
		flag_first_time = true;
		transmit_count = 1;
	}
}
//-------------------------------------------------------------------------------------------------
void read_all_settings_from_module(void)
{
	// Turn on configuration mode
	set_config_deep_sleep_mode();
	HAL_Delay(10);

	static uint8_t data[10] = {0};
	// Read module address, serial port, and airspeed COMMAND
	data[0] = 0xC1;			// 0xC1 - Read register command
	data[1] = 0x00;			// Number of register for read
	data[2] = 0x08;			// How many registers must be read

	HAL_UART_Transmit_IT(&huart1, data, 3);

	//HAL_UART_Receive_IT(&huart1, str, 1);    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//	while чикати на зчитування регістрів
//	порівняти їх з записаними

	HAL_Delay(100);


	// Return:
	// 0xC1 0x00 0x03 0x12 0x34 0x62
	// 0x12 0x34 - Adders 1234
	// 0x62 - 9600, 8n1 and 2,4 k air data rate
}
//----------------------------------------------------------------------------------------
bool init_lora_TX(void)
{
	// Зробити перевірку, що зчиталися записані конфігураційні регістри
	// Зробити глобальну змінну для конфігураційного масиву і порівняти його
	// з зчитаними даними конфіг регістрів. Якщо співпадає повністю, тоді записати в
	// глобальну змінну що модуль ініціалізований нормально

	static uint8_t data[10] = {0};

	set_config_deep_sleep_mode();
	HAL_Delay(100);

	// Init module
	// Descripe settings structure
	data[0] = 0xC0;		// 0xC0 - Set register command
	data[1] = 0x00;		// Starting address
	data[2] = 0x03;		// Length
	data[3] = 0x12;		// 00H ADD H
	data[4] = 0x34;		// 01H ADD L
	data[5] = 0x62;		// 02H register (see in Datasheet)

	HAL_UART_Transmit_IT(&huart1, data, 6);
	HAL_Delay(10);

	memset(data, 0, sizeof(data));
	// Set WOR Cycle
	data[0] = 0xC0;		// 0xC0 - Set register command
	data[1] = 0x05;		// Starting address
	data[2] = 0x01;		// Length
	data[3] = 0x00;		// set WOR Cycle 500ms
	HAL_UART_Transmit_IT(&huart1, data, 4);
	HAL_Delay(10);
	///////////////

	read_all_settings_from_module();
	set_WOR_TX_mode();
	HAL_Delay(100);

	return true;
}
//----------------------------------------------------------------------------------------
bool init_lora_RX(void)
{
	static uint8_t data[10] = {0};
	set_config_deep_sleep_mode();
	HAL_Delay(100);

	data[0] = 0xC0;
	data[1] = 0x00;		// Starting address
	data[2] = 0x03;		// Length
	data[3] = 0x12;		// 00H ADD H
	data[4] = 0x34;		// 01H ADD L
	data[5] = 0x62;		// 02H register ()

	HAL_UART_Transmit_IT(&huart1, data, 6);
	HAL_Delay(100);

	memset(data, 0, sizeof(data));
	// Set WOR Cycle
	data[0] = 0xC0;		// 0xC0 - Set register command
	data[1] = 0x05;		// Starting address
	data[2] = 0x01;		// Length
	data[3] = 0x00;		// set WOR Cycle 500ms
	HAL_UART_Transmit_IT(&huart1, data, 4);
	HAL_Delay(100);

	read_all_settings_from_module();
	set_WOR_RX_mode();
	HAL_Delay(100);

	return true;
}
//-------------------------------------------------------------------------------------------------
void set_config_deep_sleep_mode (void)
{
	// Function use for go to deep sleep and configuration mode
	// Set M0 and M1 PINs
	HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);
}
//-------------------------------------------------------------------------------------------------
void set_WOR_RX_mode (void)
{
	// Set M0 and M1 PINs in WOR Receiving mode
	HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_SET);
}
//-------------------------------------------------------------------------------------------------
void set_WOR_TX_mode (void)
{
	// Set M0 and M1 PINs in WOR Transmitting mode
	HAL_GPIO_WritePin(GPIOB, M0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, M1_Pin, GPIO_PIN_RESET);
}
//-------------------------------------------------------------------------------------------------














