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

bool init_lora_RX(void);
bool init_lora_TX(void);

int lora_transmit_data(int transmit_count);
void read_all_settings_from_module(void);
void read_settings_from_module(void);
void set_config_deep_sleep_mode (void);
void set_WOR_RX_mode (void);
void set_WOR_TX_mode (void);

lora_transmit_string_data(char* transmit_str);

extern char str[1];							// Buffer for one char
//char test_main[20] = {0};
extern bool flag_command_received;			// Flag show status receive data (completed/not completed)
extern char uart_rx_data[50];				// Main rx buffer data

int transmit_count = 1;									// Variable for transmit
int tx_lora_data = 99999;								// Test TX data
//----------------------------------------------------------------------------------------
void LoRa_RX(bool flag)
{
	static bool flag_first_time = true;								// Trigger variable
	char str_1[20] = {0};

	if((flag_first_time == true) && (flag == true))					// Do it only first time (init)
	{
		//memset(uart_rx_data, 0, sizeof(uart_rx_data));//uart_rx_data[50]

		// state_machine
		HAL_Delay(100);
		init_lora_RX();
		HAL_Delay(500);

		ssd1306_SetCursor(0, 16);
		strcpy(str_1, "Receiving data:");
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		HAL_UART_Receive_IT(&huart1, str, 1);
		memset(str_1, 0, sizeof(str_1));
		flag_first_time = false;
	}
	if((flag_first_time == false) && (flag == true))				// Do it when data was received
	{
		if(flag_command_received == true)							// If data is ready
		{
			// Data receive
			// Clean data part on OLED
			char clearn_array[25] = "                       ";
			ssd1306_SetCursor(0, 28);
			ssd1306_WriteString(clearn_array,  Font_7x10, White);
			ssd1306_UpdateScreen();

			//memset(uart_rx_data, 0, sizeof(uart_rx_data));//uart_rx_data[50]

			//Відсіяти 0 елемент в массиві
			// Print received data
			ssd1306_SetCursor(0, 28);
			//strcpy(str_1, uart_rx_data);       /// PROBLEM WHERE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			int i = 0;
			for(i = 0; i <= sizeof(str_1); i++)
			{
				if(i == 0)
				{
					//str_1[i] = uart_rx_data[]
					i++;
				}
				str_1[i-1] = uart_rx_data[i];
			}
			ssd1306_WriteString(str_1,  Font_7x10, White);
			ssd1306_UpdateScreen();

			HAL_Delay(100);
			//memset(uart_rx_data, 0, sizeof(uart_rx_data));
			flag_command_received = false;

			HAL_UART_Receive_IT(&huart1, str, 1);					// Start interrupt again
		}
	}
	if(flag == false)
	{
		flag_first_time = true;
	}
}
// -------------------------------------------------------------------------------
void LoRa_TX_send_test_number(bool flag)
{
	static bool flag_first_time = true;								// Trigger variable
	//static int transmit_count = 0;									// Variable for transmit
	char str_1[20] = {0};

	if((flag_first_time == true) && (flag == true))					// Do it only first time (init)
	{
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
		tx_lora_data++;

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

		transmit_count++;											// Increment test data
		HAL_Delay(2000);											// Must be more than 1.5 sec
	}
	if(flag == false)
	{
		flag_first_time = true;
		transmit_count = 0;
		tx_lora_data = 99999;
	}
}
//----------------------------------------------------------------------------------------
int lora_transmit_data(int transmit_count)
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

	return transmit_count;
}
// -------------------------------------------------------------------------------
void LoRa_TX_send_T_and_H(bool flag)   // Зробити пересилання стрінги !!!!
{
	static bool flag_first_time = true;								// Trigger variable
	//static int transmit_count = 0;									// Variable for transmit
	char str_1[20] = {0};

	if((flag_first_time == true) && (flag == true))					// Do it only first time (init)
	{
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

		char test_strung[35] = {0};
		char str_buf[10] = {0};
		// Add counter
		itoa(transmit_count, str_buf, 10);
		strcat(test_strung, str_buf);
		memset(str_buf, 0, sizeof(str_buf));
		// Add temperature

		strcat(test_strung, "| T=");
		itoa(am3202_sensor.temterature, str_buf, 10);
		strcat(test_strung, str_buf);
		memset(str_buf, 0, sizeof(str_buf));
		strcat(test_strung, "C");
		// Add humidity
		strcat(test_strung, " H=");
		itoa(am3202_sensor.humidity, str_buf, 10);
		strcat(test_strung, str_buf);
		memset(str_buf, 0, sizeof(str_buf));
		strcat(test_strung, "%");

		strcat(test_strung, "\n");

		HAL_UART_Transmit_IT(&huart1, test_strung, sizeof(test_strung));				// Transmitting over LoRa module
		HAL_Delay(2000);

		// Print transmitter counter
		memset(str_1, 0, sizeof(str_1));
		ssd1306_SetCursor(70, 16);
		sprintf(str_1, "%d", transmit_count);
		ssd1306_WriteString(str_1,  Font_7x10, White);
		ssd1306_UpdateScreen();

		// Print transmitter data

		ssd1306_SetCursor(0, 40);
		ssd1306_WriteString(test_strung,  Font_7x10, White);
		ssd1306_UpdateScreen();

		transmit_count++;											// Increment test data
		HAL_Delay(2000);											// Must be more than 1.5 sec
	}
	if(flag == false)
	{
		flag_first_time = true;
		transmit_count = 1;
		tx_lora_data = 99999;
	}
}
//----------------------------------------------------------------------------------------
int lora_transmit_string_data(char* transmit_str)
{
	//char end_of_message = '\n';
	//char data[10] = "777";
	//strcat(data, "\n");

//	data[5] = '0' + transmit_count%10;
//	data[4] = '0' + (transmit_count/10) % 10;
//	data[3] = '0' + (transmit_count/100) % 10;
//	data[2] = '0' + (transmit_count/1000) % 10;
//	data[1] = '0' + (transmit_count/10000) % 10;
//	data[0] = '0' + (transmit_count/100000) % 10;
//	data[6] = '\0';

	HAL_UART_Transmit_IT(&huart1, transmit_str, 10);
	int g = 99;

	return transmit_count;
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

	read_settings_from_module();

	set_WOR_RX_mode();

	HAL_Delay(100);
}
//-------------------------------------------------------------------------------------------------
void read_settings_from_module(void)
{
	// Turn on configuration mode
	set_config_deep_sleep_mode();
	HAL_Delay(10);

	static uint8_t data[10] = {0};
	// Read module address, serial port, and airspeed COMMAND
	data[0] = 0xC1;
	data[1] = 0x00;
	data[2] = 0x08;

	HAL_UART_Transmit_IT(&huart1, data, 3);
	HAL_Delay(100);

    // ЗЧИТИТИ ДАНІ З LORA МОДУЛЯ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// I ВЕРНУТИ ЗНАЧЕННЯ ІНІЦІАЛІЗАЦІї !!!!!!!!!!!!!!!!!!!

	// Return:
	// 0xC1 0x00 0x03 0x12 0x34 0x62
	// 0x12 0x34 - Adders 1234
	// 0x62 - 9600, 8n1 and 2,4 k air data rate

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














