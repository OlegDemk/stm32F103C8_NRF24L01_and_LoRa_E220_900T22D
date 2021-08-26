/*
 * oled_simulation_menu.c
 *
 *  Created on: Aug 23, 2021
 *      Author: odemki
 */
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

#include <OLED/fonts.h>
#include <OLED/oled_ssd1306.h>
#include <OLED/ssd1306.h>
#include <OLED/oled_main.h>


extern UART_HandleTypeDef huart3;


typedef struct{
	struct MenuItem* up;				// pointer on up element of list
	struct MenuItem* down;				// pointer on down element of list

	uint8_t id;							// Number of menu
	char *name;							// Name menu
	void ( *updateScreen_up ) (void );
	void ( *updateScreen_down ) (void );
	void ( *makeAction) ( void );
}MenuItem_t;

#define MENU_ITEM_NUM 7							// How many menu items

MenuItem_t items[MENU_ITEM_NUM];				// Create array structure
MenuItem_t * currentItem = &items[0];			// Create and set pointer on first element of list

// ----------------------------------------------------------------------------------------
void clear_menu_items (bool first, bool second, bool third, bool fourth)
{
	char str[30] = "                     ";
	if(first == true)
	{
		ssd1306_SetCursor(15, 16);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	if(second == true)
	{
		ssd1306_SetCursor(15, 28);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	if(first == true)
	{
		ssd1306_SetCursor(15, 40);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	if(fourth == true)
	{
		ssd1306_SetCursor(15, 52);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	ssd1306_UpdateScreen();

}
// ----------------------------------------------------------------------------------------
void print_rows_on_oled_if_up(void)
{
	char str_pointer[4] = "->";
	char str[30] = {0};
	//clearn_oled();
	clear_menu_items (true , true , true , true );

	// Print pointer on first item menu
	ssd1306_SetCursor(0, 16);
	ssd1306_WriteString(str_pointer,  Font_7x10, White);

	MenuItem_t * currentItem_buff_up = currentItem;       // Чому тут currentItem == 0 item
	for (uint8_t row = 16; row <= 52; row = row + 12)
	{
		// Print number of menu item
		itoa(currentItem_buff_up -> id, str, 10);
		ssd1306_SetCursor(15, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		// Print name of menu item
		strncpy(str, currentItem_buff_up -> name, 25);
		ssd1306_SetCursor(30, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		currentItem_buff_up = currentItem_buff_up -> down;
		if(currentItem_buff_up == 0)
		{
			break;
		}
	}
	ssd1306_UpdateScreen();
}
// ----------------------------------------------------------------------------------------
void print_rows_on_oled_if_down(void)	// print text menu item
{
	char str_pointer[4] = "->";
	char str[30] = {0};

	clear_menu_items (true , true , true , true );

	// Print pointer on first item menu
	ssd1306_SetCursor(0, 16);
	ssd1306_WriteString(str_pointer,  Font_7x10, White);

	MenuItem_t * currentItem_buff = currentItem;
	for (uint8_t row = 16; row <= 52; row = row + 12)
	{
		// Print number of menu item
        itoa(currentItem_buff -> id, str, 10);
		ssd1306_SetCursor(15, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		// Print name of menu item
		memset(str, 0, sizeof(str));
	    strncpy(str, currentItem_buff -> name, 25);
	    ssd1306_SetCursor(30, row);
	    ssd1306_WriteString(str,  Font_7x10, White);

	    currentItem_buff = currentItem_buff -> down;
	    if(currentItem_buff == 0)		// End of menu
	    {
	    	break;
	    }
	 }
	 ssd1306_UpdateScreen();
}
// ----------------------------------------------------------------------------------------
// Print first 4 items of menu (only first time)
void print_menu_init(void)
{
	char str[30] = {0};
	MenuItem_t * currentItem_buff = currentItem;

	for (uint8_t row = 16; row <= 52; row = row + 12)
	{
		if(row == 16)
		{
			// Print pointer on menu
			char str_pointer[4] = "->";
			ssd1306_SetCursor(0, row);
			ssd1306_WriteString(str_pointer,  Font_7x10, White);
			ssd1306_UpdateScreen();
		}
		// Print number of menu item
		itoa(currentItem_buff -> id, str, 10);
		ssd1306_SetCursor(15, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		// Print menu of menu item
		strncpy(str, currentItem_buff -> name, 25);
		ssd1306_SetCursor(30, row);
		ssd1306_WriteString(str,  Font_7x10, White);
		ssd1306_UpdateScreen();

		currentItem_buff = currentItem_buff -> down;
	}
}
// ----------------------------------------------------------------------------------------
void action(void)
{
	clearn_oled();

	char str_buffer[30] = "Enter!";
	clearn_pointer_on_menu();

	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(str_buffer,  Font_7x10, White);
	ssd1306_UpdateScreen();
}
// ----------------------------------------------------------------------------------------
void Menu_Init (void)
{
	// Make pointers on funscions
	void (*p_print_rows_on_oled_if_up) (void);
	void (*p_print_rows_on_oled_if_down) (void);			// Create pointer on function
	void (*p_action) (void);						// Create pointer on function
	//p_print_rows_on_oled_if_up = print_rows_on_oled_if_up;
	p_print_rows_on_oled_if_up = print_rows_on_oled_if_up;
	p_print_rows_on_oled_if_down = print_rows_on_oled_if_down;		// Save function print on pointer print_p
	p_action = action;								// Save function action on pointer action_p

	// Fill in elements(nodes) of list (7 items)
	items[0].up = 0;
	items[0].down = &items[1];
	items[0].id = 1;
	items[0].name = "LoRa E220 RX";						// Name of item
	items[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items[0].makeAction = p_action;

	items[1].up = &items[0];
	items[1].down = &items[2];
	items[1].id = 2;
	items[1].name = "LoRa E220 TX";
	items[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items[1].makeAction = p_action;

	items[2].up = &items[1];
	items[2].down = &items[3];
	items[2].id = 3;
	items[2].name = "NRF24L01 RX";
	items[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items[2].makeAction = p_action;

	items[3].up = &items[2];
	items[3].down = &items[4];
	items[3].id = 4;
	items[3].name = "NRF24L01 RX";
	items[3].updateScreen_up = p_print_rows_on_oled_if_up;
	items[3].updateScreen_down = p_print_rows_on_oled_if_down;
	items[3].makeAction = p_action;

	items[4].up = &items[3];
	items[4].down = &items[5];
	items[4].id = 5;
	items[4].name = "Item_5";
	items[4].updateScreen_up = p_print_rows_on_oled_if_up;
	items[4].updateScreen_down = p_print_rows_on_oled_if_down;
	items[4].makeAction = p_action;

	items[5].up = &items[4];
	items[5].down = &items[6];
	items[5].id = 6;
	items[5].name = "Item_6";
	items[5].updateScreen_up = p_print_rows_on_oled_if_up;
	items[5].updateScreen_down = p_print_rows_on_oled_if_down;
	items[5].makeAction  = p_action;

	items[6].up = &items[5];
	items[6].down = 0;
	items[6].id = 7;
	items[6].name = "Item_7";
	items[6].updateScreen_up = p_print_rows_on_oled_if_up;
	items[6].updateScreen_down = p_print_rows_on_oled_if_down;
	items[6].makeAction  = p_action;

}
// ----------------------------------------------------------------------------------------
void up(void)
{
	if (currentItem->up)
	{
	    currentItem = currentItem->up;
	    if (currentItem->updateScreen_up )
	    {
	        currentItem->updateScreen_up();
	    }
	}
}
// ----------------------------------------------------------------------------------------
void down(void)
{
	if (currentItem->down)
	{
	    currentItem = currentItem->down;
	    if (currentItem->updateScreen_down )
	    {
	        currentItem->updateScreen_down();
	    }
	}
}
// ----------------------------------------------------------------------------------------
void enter(void)
{
	if (currentItem->makeAction)
	{
		//currentItem = currentItem->makeAction();
		currentItem->makeAction();
	}
}
// ----------------------------------------------------------------------------------------
void simulation_navigation_on_menu(void)
{
	Menu_Init();

	print_menu_init();
	HAL_Delay(1000);

	while(1)
	{
		//Надрукувати перші 4 меню
			int delay = 500;

			down();
			HAL_Delay(delay);
			down();
			HAL_Delay(delay);
			down();
			HAL_Delay(delay);
			down();
			HAL_Delay(delay);
			down();
			HAL_Delay(delay);
			down();
			HAL_Delay(2000);


			enter();

			HAL_Delay(2000);

			up();
			HAL_Delay(delay);
			up();
			HAL_Delay(delay);
			up();

			HAL_Delay(2000);

			down();
			HAL_Delay(delay);
			up();
			HAL_Delay(delay);
			down();
			HAL_Delay(delay);
			up();
			HAL_Delay(delay);


	}
}


// ---------------------------------------------------------------------------------


