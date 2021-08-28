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

/*				READ ME
  	Для додавання нового меню потрібно зробити такі кроки:
  	1. Створити массив з відповідною кількістю елементів структур MenuItem_t імя_нового_пункту[кількість елементі в в ньому];

  	2. Заповнити відповідні поля спасивів структур у функції Menu_Init:
  		Example:
  			items[0].up = 0;											// Адрес елемента меню, на який елемент буде здійснено перехід при натисеанні кнопки UP
			items[0].down = &items[1];									// Адрес елемента меню, на який елемент буде здійснено перехід при натисеанні кнопки Down
			items[0].child = &items_menu_1[0];							// Адрес нульового елемента нового підменю
			items[0].id = 1;											// Порядковий номер елемента меню
			items[0].name = "LoRa E220 RX";								// Назва меню
			items[0].updateScreen_up = p_print_rows_on_oled_if_up;		// Показник на функцію, яка робить навігацію вверх, і відотсовує її на екрані
			items[0].updateScreen_down = p_print_rows_on_oled_if_down;	// Показник на функцію, яка робить навігацію вниз, і відотсовує її на екрані
			items[0].makeAction = 0;									// Показник на функцію, яка робить дію при натисканні на даний пункт меню
	makeAction - не використовується тоді коли є поле child (тобто коли є підменю). Це зроблено для того, щоб сумістити
	дві функції в одній кнопці.

	3.
 *
 */

extern UART_HandleTypeDef huart3;


typedef struct Struct{
	struct MenuItem* up;						// pointer on up element of list
	struct MenuItem* down;						// pointer on down element of list
	struct MenuItem* child;

	uint8_t id;									// Number of menu
	char *name;									// Name menu
	void ( *updateScreen_up ) (void );
	void ( *updateScreen_down ) (void );
	void ( *makeAction) ( void );
}MenuItem_t;

uint8_t button = 0;								// Pressed button
extern int button_processed_status;				// For interrupt work only one time

#define MENU_ITEM_NUM 7							// How many menu items
#define MENU_1_ITEM_NUM 5
#define MENU_2_ITEM_NUM 3

MenuItem_t items[MENU_ITEM_NUM];				// Create main menu item array structure
MenuItem_t items_menu_1[MENU_1_ITEM_NUM];
MenuItem_t items_menu_2[MENU_2_ITEM_NUM];

MenuItem_t * currentItem = &items[0];			// Create and set pointer on first element of list

char str_pointer[4] = "->";						// How look pointer on menu item

// ----------------------------------------------------------------------------------------
// Clear some menu items.
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
	char str[30] = {0};

	clear_menu_items (true , true , true , true );

	// Print pointer on first item menu
	ssd1306_SetCursor(0, 16);
	ssd1306_WriteString(str_pointer,  Font_7x10, White);

	MenuItem_t * currentItem_buff_up = currentItem;				// Create buffer on selected current item pointer.
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

		currentItem_buff_up = currentItem_buff_up -> down;		// Make a step down

		// Print only existing items
		if(currentItem_buff_up == 0)							// If no next item
		{
			break;
		}
	}
	ssd1306_UpdateScreen();
}
// ----------------------------------------------------------------------------------------
void print_rows_on_oled_if_down(void)	// print text menu item
{
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

	    currentItem_buff = currentItem_buff -> down;			// Make a step down

	    // Print only existing items
	    if(currentItem_buff == 0)		 						// If no next item
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
	MenuItem_t * currentItem_buff = currentItem;

	//Print selected name of menu
	char str[30] = ">> MAIN MENU <<";
	ssd1306_SetCursor(10, 0);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	for (uint8_t row = 16; row <= 52; row = row + 12)
	{
		if(row == 16)
		{
			// Print pointer on menu (On top)
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

	 //Print selected name of menu
	char str[30] = {0};
	strncpy(str, currentItem -> name, 25);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	//currentItem = &items_menu_1[0];	// Redefine pointer


	//char str[30] = {0};
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
		if(currentItem_buff == 0)
		{
			break;
		}
	}
}
// ----------------------------------------------------------------------------------------
void return_from_menu(void)
{
	currentItem = &items[0];		// Jump to main menu
	clearn_oled();

	// Print "MAIN MENU:"
	char str[30] = ">> MAIN MENU <<";
	//strncpy(str, currentItem -> name, 25);
	ssd1306_SetCursor(10, 0);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	//action();
	print_menu_init();

}

// ----------------------------------------------------------------------------------------
void Menu_Init (void)
{
	// Make pointers on funscions
	void (*p_print_rows_on_oled_if_up) (void);
	void (*p_print_rows_on_oled_if_down) (void);			// Create pointer on function
	void (*p_return_from_menu)(void);
	void (*p_action) (void);							// Create pointer on function
	//p_print_rows_on_oled_if_up = print_rows_on_oled_if_up;
	p_print_rows_on_oled_if_up = print_rows_on_oled_if_up;
	p_print_rows_on_oled_if_down = print_rows_on_oled_if_down;		// Save function print on pointer print_p
	p_action = action;								// Save function action on pointer action_p
	p_return_from_menu = return_from_menu;
	// Fill in elements(nodes) of list (7 items)
	// Main menu items
	/////////////////////////////////////////////////////////////////
//	В першому рівні меню, одна клавіша ентер переходить на наступне меню
//	в наступному меню, та сама клавіша ентер, викликає специфічну функцібю певного меню

	items[0].up = 0;
	items[0].down = &items[1];
	items[0].child = &items_menu_1[0];
	items[0].id = 1;
	items[0].name = "LoRa E220 RX";
	items[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items[0].makeAction = 0;

	items[1].up = &items[0];
	items[1].down = &items[2];
	items[1].child = &items_menu_2[0];
	items[1].id = 2;
	items[1].name = "LoRa E220 TX";
	items[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items[1].makeAction = 0;

	items[2].up = &items[1];
	items[2].down = &items[3];
	items[2].child = 0;
	items[2].id = 3;
	items[2].name = "NRF24L01 RX";
	items[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items[2].makeAction = 0;

	items[3].up = &items[2];
	items[3].down = &items[4];
	items[3].child = 0;
	items[3].id = 4;
	items[3].name = "NRF24L01 RX";
	items[3].updateScreen_up = p_print_rows_on_oled_if_up;
	items[3].updateScreen_down = p_print_rows_on_oled_if_down;
	items[3].makeAction = 0;

	items[4].up = &items[3];
	items[4].down = &items[5];
	items[4].child = 0;
	items[4].id = 5;
	items[4].name = "Item_5________";
	items[4].updateScreen_up = p_print_rows_on_oled_if_up;
	items[4].updateScreen_down = p_print_rows_on_oled_if_down;
	items[4].makeAction = 0;

	items[5].up = &items[4];
	items[5].down = &items[6];
	items[5].child = 0;
	items[5].id = 6;
	items[5].name = "Item_6________";
	items[5].updateScreen_up = p_print_rows_on_oled_if_up;
	items[5].updateScreen_down = p_print_rows_on_oled_if_down;
	items[5].makeAction  = 0;

	items[6].up = &items[5];
	items[6].down = 0;
	items[5].child = 0;
	items[6].id = 7;
	items[6].name = "Item_7________";
	items[6].updateScreen_up = p_print_rows_on_oled_if_up;
	items[6].updateScreen_down = p_print_rows_on_oled_if_down;
	items[6].makeAction  = 0;

	///////////////////////////////////////////////////////////////////
	// Creating second menu
	items_menu_1[0].up = 0;
	items_menu_1[0].down = &items_menu_1[1];
	items_menu_1[0].id = 1;
	items_menu_1[0].child = 0;
	items_menu_1[0].name = "menu_1_1";
	items_menu_1[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[0].makeAction = p_action;

	items_menu_1[1].up = &items_menu_1[0];
	items_menu_1[1].down = &items_menu_1[2];
	items_menu_1[1].child = 0;
	items_menu_1[1].id = 2;
	items_menu_1[1].name = "menu_1_2";						// Name of item
	items_menu_1[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[1].makeAction = p_action;

	items_menu_1[2].up = &items_menu_1[1];
	items_menu_1[2].down = &items_menu_1[3];
	items_menu_1[2].child = 0;
	items_menu_1[2].id = 3;
	items_menu_1[2].name = "menu_1_3";						// Name of item
	items_menu_1[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[2].makeAction = p_action;

	items_menu_1[3].up = &items_menu_1[2];
	items_menu_1[3].down = &items_menu_1[4];
	items_menu_1[3].child = 0;
	items_menu_1[3].id = 4;
	items_menu_1[3].name = "menu_1_4";						// Name of item
	items_menu_1[3].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[3].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[3].makeAction = p_action;

	items_menu_1[4].up = &items_menu_1[3];
	items_menu_1[4].down = 0;
	items_menu_1[4].child = 0;
	items_menu_1[4].id = 5;
	items_menu_1[4].name = "EXIT";						// Name of item
	items_menu_1[4].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[4].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[4].makeAction = p_return_from_menu;

	///////////////////////////////////////////////////////////////////
	// Creating second menu
	items_menu_2[0].up = 0;
	items_menu_2[0].down = &items_menu_2[1];
	items_menu_2[0].child = 0;
	items_menu_2[0].id = 1;
	items_menu_2[0].name = "menu_2_1";						// Name of item
	items_menu_2[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[0].makeAction = p_action;

	items_menu_2[1].up = &items_menu_2[0];
	items_menu_2[1].down = &items_menu_2[2];
	items_menu_1[1].child = 0;
	items_menu_2[1].id = 2;
	items_menu_2[1].name = "menu_2_2";						// Name of item
	items_menu_2[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[1].makeAction = p_action;

	items_menu_2[2].up = &items_menu_2[1];
	items_menu_2[2].down = 0;
	items_menu_1[2].child = 0;
	items_menu_2[2].id = 3;
	items_menu_2[2].name = "EXIT";						// Name of item
	items_menu_2[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[2].makeAction = p_return_from_menu;


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
	bool status = true;
	// Якщо є функція "makeAction" тоді виконати її
	if (currentItem->makeAction)
	{
		currentItem->makeAction();
		status = false;
	}
	// Якщо є перехід на  "child" і "makeAction" не була виконана, тоді виконати перехід
	if((currentItem->child) && (status == true))
	{
		currentItem = currentItem->child;
		action();
	}
}
// ----------------------------------------------------------------------------------------
void simulation_navigation_on_menu(void)
{
	Menu_Init();

	print_menu_init();
	HAL_Delay(10);

	while(1)
	{
		if(button_processed_status == 1)	// If buttons was pressed
		{

			button_processed_status = 1;
			switch (button)
			{
				case BOTTON_UP:
					up();
					break;
				case BUTTON_ENTER:
					enter();
					break;
				case BUTTON_DOWN:
					down();
					break;
			}
			button = 0;
		}
	}
}


// ---------------------------------------------------------------------------------


