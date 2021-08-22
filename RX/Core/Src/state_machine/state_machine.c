/*
 * state_machine.c
 *
 *  Created on: Aug 18, 2021
 *      Author: odemki
 */
#include "main.h"
#include <state_machine/state_machine.h>

#include <OLED/fonts.h>
#include <OLED/oled_ssd1306.h>
#include <OLED/ssd1306.h>
#include <OLED/oled_main.h>

volatile STATE_t state = ST_1;
bool button_was_pressed = true;			// Pointer on menu item
uint8_t utton_enter_pressed_flag = 0;		// This variable set by "ENTER" button

extern bool flag_command_received;
extern char str[1];
extern char uart_rx_data[50];
extern UART_HandleTypeDef huart1;

int pressed_batton_counter = 0;


// ----------------------------------------------------------------------------------------
STATE_t state_get(void)
{
	return state;
}

// ----------------------------------------------------------------------------------------
void state_set(STATE_t new_state)
{
	state = new_state;
}
// ----------------------------------------------------------------------------------------

//typedef struct{
//	struct MenuItem* up;				// показник на попередній массив
//	struct MenuItem* down;				// показник наступний массив
//
//	void ( *updateScreen ) (void );		// Функція яка відмальовує при переході в даний пункт
//	void ( *makeAction) ( void );		// Функція яка робить дію при натисканні кнопки ENTER
//}MenuItem_t;
//
//static struct MenuItem_t items[MENU_ITEM_NUM];   // What is it ?????????
//
//MenuItem_t * currentItem = &items[0];	// встановлюємо показний на нульовий адрес списку
//
//void Menu_Init ( void )
//{
//	items[0].up = 0;
//	items[0].down = &items[1];
//	items[0].updateScreen = 0;
//	items[0].makeAction = 0;
//
//	items[1].up = &items[0];
//	items[1].down = &items[2];
//	items[1].updateScreen = 0;
//	items[1].makeAction = 0;
//
//	items[2].up = &items[1];
//	items[2].down = &items[3];
//	items[2].updateScreen = 0;
//	items[2].makeAction = 0;
//
//	items[3].up =&items[2];
//	items[3].down =0;
//	items[3].updateScreen = 0;
//	items[3].makeAction  =0;
//
//}
//// ----------------------------------------------------------------------------------------
//void up(void)
//{
//	if (currentItem->up)
//	{
//	    currentItem = currentItem->up;
//	    if (currentItem->updateScreen )
//	    {
//	        currentItem->updateScreen();
//	    }
//	}
//}
//// ----------------------------------------------------------------------------------------
//void down(void)
//{
//	if (currentItem->down)
//	{
//	    currentItem = currentItem->down;
//	    if (currentItem->updateScreen )
//	    {
//	        currentItem->updateScreen();
//	    }
//	}
//}
//// ----------------------------------------------------------------------------------------
//void enter(void)
//{
//	if (currentItem->makeAction)
//	{
//	    currentItem = currentItem->makeAction();
//	}
//}
//// ----------------------------------------------------------------------------------------
//void menu_simulation(void)
//{
//
//
//}

// ----------------------------------------------------------------------------------------
void state_machine(void)
{
	// Init OLED
	// Init nrf module
	// Init LoRa module
	// Print status modules
	char str[20] = {0};


	// Only 4
	char menu_items[4][30] = {
		"LoRa E220   RX",
		"LoRa E220   TX",
		"NRF 24L01   RX",
		"NRF 24L01   TX"
	};

	clearn_oled();

	bool print_all_menu = true;

	while(1)   // Main loop
	{
		if(print_all_menu == true)
		{
			// Print all menus on OLED
			uint8_t row = 20;
			for(uint8_t i = 0; i <=4; i++)
			{
				ssd1306_SetCursor(20, row);
				memset(str, 0, sizeof(str));
				strcpy(str, menu_items[i]);
				ssd1306_WriteString(str,  Font_7x10, White);
				ssd1306_UpdateScreen();
				row = row + 10;
			}
		}

		if(button_was_pressed == true)				// From timer_1 interrupt (New button was pressed)
		{

			switch(state_get())
			{
				case ST_1:
					// Print data on OLED
					clearn_pointer_on_menu();

					ssd1306_SetCursor(0, 20);
					memset(str, 0, sizeof(str));
					strcpy(str, "->");
					ssd1306_WriteString(str,  Font_7x10, White);
					ssd1306_UpdateScreen();
					button_was_pressed = false;
					print_all_menu == false;

					if(utton_enter_pressed_flag == 1)	// If "Enter" button was press"
					{
						print_all_menu == true;
						clearn_oled();
						while (utton_enter_pressed_flag == 1)
						{
							// LoRa module working where
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
							HAL_Delay(50);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
							HAL_Delay(50);
						}
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
						utton_enter_pressed_flag = 0;
					}

					break;

				case ST_2:
					// Print data on OLED
					clearn_pointer_on_menu();

					ssd1306_SetCursor(0, 30);
					memset(str, 0, sizeof(str));
					strcpy(str, "->");
					ssd1306_WriteString(str,  Font_7x10, White);
					ssd1306_UpdateScreen();
					button_was_pressed = false;
					print_all_menu == false;

					if(utton_enter_pressed_flag == 1)	// If "Enter" button was press"
					{
						clearn_oled();
						print_all_menu == true;
						while (utton_enter_pressed_flag == 1)
						{
							// LoRa module working where
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
							HAL_Delay(100);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
							HAL_Delay(100);
						}
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
						utton_enter_pressed_flag = 0;
					}

					break;

				case ST_3:
					// Print data on OLED
					clearn_pointer_on_menu();

					ssd1306_SetCursor(0, 40);
					memset(str, 0, sizeof(str));
					strcpy(str, "->");
					ssd1306_WriteString(str,  Font_7x10, White);
					ssd1306_UpdateScreen();
					button_was_pressed = false;
					print_all_menu == false;

					if(utton_enter_pressed_flag == 1)	// If "Enter" button was press"
					{
						clearn_oled();
						print_all_menu == true;
						while (utton_enter_pressed_flag == 1)
						{
							// LoRa module working where
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
							HAL_Delay(300);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
							HAL_Delay(300);
						}
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
						utton_enter_pressed_flag = 0;
					}

					break;

				case ST_4:
					// Print data on OLED
					clearn_pointer_on_menu();

					ssd1306_SetCursor(0, 50);
					memset(str, 0, sizeof(str));
					strcpy(str, "->");
					ssd1306_WriteString(str,  Font_7x10, White);
					ssd1306_UpdateScreen();
					button_was_pressed = false;
					print_all_menu == false;

					if(utton_enter_pressed_flag == 1)	// If "Enter" button was press"
					{
						clearn_oled();
						print_all_menu == true;
						while (utton_enter_pressed_flag == 1)
						{
							// LoRa module working where
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
							HAL_Delay(600);
							HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
							HAL_Delay(600);
						}
						HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
						utton_enter_pressed_flag = 0;
					}

					break;

			}
		}
	}
}
// ----------------------------------------------------------------------------------------
void clearn_pointer_on_menu(void)
{
	for(uint8_t i =20; i <= 50; i = i+10)
	{
		char str[5] = {0};
		ssd1306_SetCursor(0, i);
		memset(str, 0, sizeof(str));
		strcpy(str, "  ");
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	ssd1306_UpdateScreen();

}

