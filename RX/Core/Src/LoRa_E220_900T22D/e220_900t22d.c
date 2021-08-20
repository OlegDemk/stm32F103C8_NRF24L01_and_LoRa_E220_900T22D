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

extern UART_HandleTypeDef huart1;

extern bool flag_command_received;
extern char uart_rx_data[50];

bool init_lora(void);
void test_uart(void);



//----------------------------------------------------------------------------------------
bool init_lora(void)
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














