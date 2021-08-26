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
extern UART_HandleTypeDef huart3;

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
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//typedef struct{
//	struct MenuItem* up;				// pointer on up element of list
//	struct MenuItem* down;				// pointer on down element of list
//
//	uint8_t id;							// Number of menu
//	char *name;							// Name menu
//	void ( *updateScreen ) (void );
//	void ( *makeAction) ( void );
//}MenuItem_t;
//
//static struct MenuItem_t items[6];   // What is it ????????? I have ERROR
//
//MenuItem_t * currentItem = &items[0];			// Create and set pointer on first element of list
//
//// ----------------------------------------------------------------------------------------
//void print_rows_on_oled(void)
//{
//	// Function must receive current number menu
//
//	// Print only 4 lines of menu
//	// Because OLED has 4 free lines
//	for(uint8_t row = 0; row<= 4; row++)
//	{
//
//	}
//}
//// ----------------------------------------------------------------------------------------
//void action(void)
//{
//	char str_buffer[30] = "Enter!\n\r";
//	clearn_pointer_on_menu();
//
//	ssd1306_SetCursor(0, 0);
//	ssd1306_WriteString(str_buffer,  Font_7x10, White);
//	ssd1306_UpdateScreen();
//}
//// ----------------------------------------------------------------------------------------
//void Menu_Init ( void )
//{
//	// Make pointers on funscions
//	void (*p_print_rows_on_oled) (void);			// Create pointer on function
//	void (*p_action) (void);						// Create pointer on function
//	p_print_rows_on_oled = print_rows_on_oled;		// Save function print on pointer print_p
//	p_action = action;								// Save function action on pointer action_p
//
//	// Fill in elements(nodes) of list (7 items)
//	items[0].up = 0;
//	items[0].down = &items[1];
//	items[0].id = 0;
//	items[0].name = "Item_0";						// Name of item
//	items[0].updateScreen = p_print_rows_on_oled;
//	items[0].makeAction = p_action;
//
//	items[1].up = &items[0];
//	items[1].down = &items[2];
//	items[1].id = 1;
//	items[1].name = "Item_1";
//	items[1].updateScreen = p_print_rows_on_oled;
//	items[1].makeAction = p_action;
//
//	items[2].up = &items[1];
//	items[2].down = &items[3];
//	items[2].id = 2;
//	items[2].name = "Item_2";
//	items[2].updateScreen = p_print_rows_on_oled;
//	items[2].makeAction = p_action;
//
//	items[3].up = &items[2];
//	items[3].down = &items[4];
//	items[3].id = 3;
//	items[3].name = "Item_3";
//	items[3].updateScreen = p_print_rows_on_oled;
//	items[3].makeAction = p_action;
//
//	items[4].up = &items[3];
//	items[4].down = &items[5];
//	items[4].id = 4;
//	items[4].name = "Item_4";
//	items[4].updateScreen = p_print_rows_on_oled;
//	items[4].makeAction = p_action;
//
//	items[5].up = &items[4];
//	items[5].down = &items[6];
//	items[5].id = 5;
//	items[5].name = "Item_5";
//	items[5].updateScreen = p_print_rows_on_oled;
//	items[5].makeAction  = p_action;
//
//	items[6].up = &items[5];
//	items[6].down = 0;
//	items[6].id = 6;
//	items[6].name = "Item_6";
//	items[6].updateScreen = p_print_rows_on_oled;
//	items[6].makeAction  = p_action;
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
//	//1. Simulation press up, down and enter
//
//    // For example:
//	// Show rows on OLED( only 4 rows)
//	//		down
//	// Show rows on OLED( only 4 rows)
//	//		down
//	// Show rows on OLED( only 4 rows)
//	//		down
//	// Show rows on OLED( only 4 rows)
//	//		up
//	// Show rows on OLED( only 4 rows)
//	// 		up
//	// Show rows on OLED( only 4 rows)
//
//}
//

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

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

