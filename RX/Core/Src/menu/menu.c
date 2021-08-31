/*
 * oled_simulation_menu.c
 *
 *  Created on: Aug 23, 2021
 *      Author: odemki
 */
#include <menu/menu.h>
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

#include <OLED/fonts.h>
#include <OLED/oled_ssd1306.h>
#include <OLED/ssd1306.h>
#include <OLED/oled_main.h>


/*							READ ME
 	  	  Дане меню використовує OLED екран з драйвером ssd1306.  0.96 дюйма 128x64 I2C
      	  Зчитування кнопок:
  	  Дані про натиснуті кнопки беруться з файлу stm32f1xx_it.c.
  	  В файлі stm32f1xx_it.c відбуваються переривання при натисканні будь якої з тьох кнопок.
  	  Після переривання вмикається таймер, яки відрахоувє певний час, який кномпка має бути зажата, щоб
  	  відсіяти дребізг контактів кнопки.

      	  Меню:
      	  Принцип роботи меню:
      1. Створюється структура, в якій елементи структури ссилаються на інші структури.В подальшому
      ці елементи використовуються для того, щоб мати можливість рухатись по чотирьохзаязному списку структур.
      Також в є елементи структури в які буде записується назва елемена, порядковий номер, і такох адреса функці
      для відрисовки графіки на екрані.
      2. Створюється массив таких структур з відповідною кількістю елементів для кожного меню.
      3. Створюється показний на найперший пункт основного меню. Він буде використовуватись як основний показник
      на структуру елемента меню, на якому зараз знаходиться.
      4. В функції Menu_Init відбувається створення показника на функції які будуть вписуватися в поля структур нижще.
      5. Заповнення структур в функції Menu_Init. Заповнюються всі елементи структури відповідно до задачі.
      В заледності від задачі можна додавати або відміняти кількість елементів структури.
      6. Навігація відбувається в функції menu.


      Редагування меню:
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

	3. Дописати виконавчу функцію. Потім записати її тут: items[0].makeAction, замість нуля.
 *
 */



//ПРоблеми !!!!!!!!!!!!!!!
//3. Обводити квадратом назву в верхні частині меню
//5. Зробити як окремий файл
//6. Залити на гіт, як окремий проект





// 1. При переході в відповідне меню, назву вибраного меню виводити на верх екрану.  DONE
// 4. При виході з функцій items_menu_1_set_par_1 і  do_it_function_menu_1 друкувати меню на екран від першого меню  DONE
//2. З права виводити масштаб меню у вигляді полоски   DONE
//7. Зробити ці змінні екстерном або в h файл      DONE
//uint8_t BOTTON_DOESENT_PRESS = 0;
//uint8_t BOTTON_UP = 1;
//uint8_t BUTTON_ENTER = 2;
//uint8_t BUTTON_DOWN = 3;

extern UART_HandleTypeDef huart3;

// ----------------------------------------------------------------------------------
// Main struct menu
typedef struct Struct{
	struct Struct* up;							// pointer on up element of list
	struct Struct* down;						// pointer on down element of list
	struct Struct* child;						// Pointer on child menu
	struct Struct* parent;						// Pointer on parent menu

	uint8_t id;									// Number of menu
	char *name;									// Name menu
	void ( *updateScreen_up ) (void );			// Function print graphic on OLED
	void ( *updateScreen_down ) (void );		// Function print graphic on OLED
	void ( *makeAction) ( void );
}MenuItem_t;

// How many menu items in menu
#define MENU_ITEM_NUM 7
#define MENU_1_ITEM_NUM 5
#define MENU_2_ITEM_NUM 3
#define MENU_3_ITEM_NUM 2
#define MENU_4_ITEM_NUM 10

// Create menu item array structure for all menus
MenuItem_t items[MENU_ITEM_NUM];
MenuItem_t items_menu_1[MENU_1_ITEM_NUM];
MenuItem_t items_menu_2[MENU_2_ITEM_NUM];
MenuItem_t items_menu_3[MENU_3_ITEM_NUM];
MenuItem_t items_menu_4[MENU_4_ITEM_NUM];

MenuItem_t * currentItem = &items[0];			// Create and set pointer on first element of list

bool block_interrupt_form_up_and_down_buttons = false;				// Flag for lock interrupt from 'up' and 'down' buttons in some cases
uint8_t button_status = BOTTON_DOESENT_PRESS;								// Pressed button
extern int button_processed_status;				// For interrupt work only one time

char str_pointer[4] = "->";						// How look pointer on menu item

// Rows coordinates
uint16_t first_menu_row = 16;
uint16_t second_menu_row = 28;
uint16_t third_menu_row = 40;
uint16_t fourth_menu_row = 52;
uint8_t row_step = 12;
uint16_t start_print_id_menu_x = 15;
uint16_t start_print_name_menu_x = 30;

// ----------------------------------------------------------------------------------------
/*
This function print scrollbar on right part of OLED.

 */
void scroll_bar(void)
{
	// Work size scrollbar
	uint16_t start_x_scrollbar = 124;
	uint16_t start_y_scrollbar = 17;
	uint16_t active_width = 3;
	uint16_t lenght_all_scrollbar = 48;

	uint8_t menu_items_counter = 1;

	// Знайти скільки емементві в меню. Відповідно до кількості елементів вирахоується довжини скролбару
	// 1. Скопіювати показник на меню в буффер
	// 2. Піднятися до останнього елемента меню
	// 3. Інкрементувати лічильник елементів меню, до останього елемента меню
	// 4. записати нараховані емементи в

	MenuItem_t * currentItem_buff = currentItem;

	if ((currentItem_buff -> up) != 0)							// Step up if existing up
	{
		do{
			currentItem_buff = currentItem_buff -> up;			// Step up to the top of menu
		}
		while ((currentItem_buff -> up) != 0);					// If the top of item doesen't found
	}

	if ((currentItem_buff -> up) == 0)							// If found top of menu item
	{
		do{
			currentItem_buff = currentItem_buff -> down;
			menu_items_counter++;								// Count how many list on menu
		}
		while ((currentItem_buff -> down) != 0);
	}
	 // Очистити частину керану де є скролбар
	ssd1306FillRect(start_x_scrollbar, start_y_scrollbar, active_width, lenght_all_scrollbar - 2, Black);

	// Вивести подовгастий квадрат на край екрану
	ssd1306_DrawRectangle(start_x_scrollbar - 1, start_y_scrollbar - 1, start_x_scrollbar + active_width, 63, White);
	ssd1306_UpdateScreen();

	 // Print scroling line
	int id_for_line = currentItem -> id;				// Скопіювати порядковий номер меню
	// Print scroling line
	uint16_t line_lenght = (lenght_all_scrollbar/menu_items_counter + 1);					   	// Довжина лінії яка відповідає одному меню
	uint16_t start_lenght = 16 + ((id_for_line - 1)*line_lenght);			// Початок лінії
	// Print active scrollbar part (line)
	ssd1306FillRect(start_x_scrollbar, start_lenght, active_width, line_lenght, White);

	ssd1306_UpdateScreen();
}
// ----------------------------------------------------------------------------------------
void print_rectangle_on_head(void)
{
	ssd1306_DrawRectangle(0, 0, 127 , 15, White);
	ssd1306_UpdateScreen();
}
// ----------------------------------------------------------------------------------------
// Clear some menu items.
void clear_menu_items (bool first, bool second, bool third, bool fourth)
{
	uint8_t start_row_x = 15;
	char str[16] = "              ";   // Must be 15

	if(first == true)
	{
		ssd1306_SetCursor(start_row_x, first_menu_row);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	if(second == true)
	{
		ssd1306_SetCursor(start_row_x, second_menu_row);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	if(first == true)
	{
		ssd1306_SetCursor(start_row_x, third_menu_row);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	if(fourth == true)
	{
		ssd1306_SetCursor(start_row_x, fourth_menu_row);
		ssd1306_WriteString(str,  Font_7x10, White);
	}
	ssd1306_UpdateScreen();
}
// ----------------------------------------------------------------------------------------
void print_rows_on_oled_if_up(void)
{
	char str[16] = {0};

	clear_menu_items (true , true , true , true );

	print_rectangle_on_head();

	// Print pointer on first item menu
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str_pointer,  Font_7x10, White);

	MenuItem_t * currentItem_buff_up = currentItem;				// Create buffer on selected current item pointer.
	for (uint8_t row = first_menu_row; row <= fourth_menu_row; row = row + row_step)
	{
		// Print number of menu item
		itoa(currentItem_buff_up -> id, str, 10);
		ssd1306_SetCursor(start_print_id_menu_x, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		// Print name of menu item
		strncpy(str, currentItem_buff_up -> name, 15);
		ssd1306_SetCursor(start_print_name_menu_x, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		currentItem_buff_up = currentItem_buff_up -> down;		// Make a step down

		// Print only existing items
		if(currentItem_buff_up == 0)							// If no next item
		{
			break;
		}
	}
	ssd1306_UpdateScreen();
	scroll_bar();
}
// ----------------------------------------------------------------------------------------
void print_rows_on_oled_if_down(void)	// print text menu item
{
	char str[16] = {0};

	clear_menu_items (true , true , true , true );

	print_rectangle_on_head();

	// Print pointer on first item menu
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str_pointer,  Font_7x10, White);

	MenuItem_t * currentItem_buff = currentItem;
	for (uint8_t row = first_menu_row; row <= fourth_menu_row; row = row + row_step)
	{
		// Print number of menu item
        itoa(currentItem_buff -> id, str, 10);
		ssd1306_SetCursor(start_print_id_menu_x, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		// Print name of menu item
		memset(str, 0, sizeof(str));
	    strncpy(str, currentItem_buff -> name, 15);
	    ssd1306_SetCursor(start_print_name_menu_x, row);
	    ssd1306_WriteString(str,  Font_7x10, White);

	    currentItem_buff = currentItem_buff -> down;			// Make a step down

	    // Print only existing items
	    if(currentItem_buff == 0)		 						// If no next item
	    {
	    	break;
	    }
	 }
	 ssd1306_UpdateScreen();
	 scroll_bar();
}
// ----------------------------------------------------------------------------------------
// Print first 4 items of menu (only first time)
void print_menu_init(void)
{
	MenuItem_t * currentItem_buff = currentItem;

	print_rectangle_on_head();

	//Print selected name of menu
	char str[20] = ">> MAIN MENU <<";
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	for (uint8_t row = first_menu_row; row <= fourth_menu_row; row = row + row_step)
	{
		if(row == first_menu_row)
		{
			// Print pointer on menu (On top)
			char str_pointer[4] = "->";
			ssd1306_SetCursor(0, row);
			ssd1306_WriteString(str_pointer,  Font_7x10, White);
		}
		// Print number of menu item
		itoa(currentItem_buff -> id, str, 10);
		ssd1306_SetCursor(start_print_id_menu_x, row);
		ssd1306_WriteString(str,  Font_7x10, White);
		memset(str, 0, sizeof(str));

		// Print menu of menu item
		strncpy(str, currentItem_buff -> name, 15);
		ssd1306_SetCursor(start_print_name_menu_x, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		ssd1306_UpdateScreen();

		currentItem_buff = currentItem_buff -> down;
	}
	scroll_bar();
}
// ----------------------------------------------------------------------------------------
void action(void)
{
	char str[16] = {0};

	clearn_oled();

	print_rectangle_on_head();

	//Print selected name of menu on top of OLED
	MenuItem_t * currentItem_buff_parent = currentItem;
	currentItem_buff_parent = currentItem_buff_parent -> parent;

	strncpy(str, currentItem_buff_parent -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	MenuItem_t * currentItem_buff = currentItem;

	for (uint8_t row = first_menu_row; row <= fourth_menu_row; row = row + row_step)
	{
		if(row == first_menu_row)
		{
			// Print pointer on menu
			char str_pointer[4] = "->";
			ssd1306_SetCursor(0, row);
			ssd1306_WriteString(str_pointer,  Font_7x10, White);
		}
		// Print number of menu item
		itoa(currentItem_buff -> id, str, 10);
		ssd1306_SetCursor(start_print_id_menu_x, row);
		ssd1306_WriteString(str,  Font_7x10, White);

		// Print menu of menu item
		strncpy(str, currentItem_buff -> name, 15);
		ssd1306_SetCursor(start_print_name_menu_x, row);
		ssd1306_WriteString(str,  Font_7x10, White);
		ssd1306_UpdateScreen();

		currentItem_buff = currentItem_buff -> down;
		if(currentItem_buff == 0)
		{
			break;
		}
	}
	scroll_bar();
}
// ----------------------------------------------------------------------------------------
void return_from_menu(void)
{
	currentItem = &items[0];											// Jump to main menu
	clearn_oled();

	// Print "MAIN MENU:"
	char str[20] = ">> MAIN MENU <<";
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	print_menu_init();													// Print all start menu
}
// ----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void do_it_function_menu_1(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Doing something 1", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_Delay(200);

	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_1[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_1_set_par_1(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 1", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_1[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_1_set_par_2(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 2", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_1[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_1_set_par_3(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 3", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_1[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void do_it_function_menu_2(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Doing something 2", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_Delay(50);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_Delay(50);
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_2[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_2_set_par_1(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 1", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_2[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void do_it_function_menu_3(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Doing something 3", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_Delay(500);

	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_3[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------------------------
void do_it_function_menu_4(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Doing something 4", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_Delay(20);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_Delay(500);

	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_1(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 1", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_2(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 2", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_3(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 3", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_4(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 4", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_5(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 5", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_6(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 6", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_7(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 7", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void items_menu_4_set_par_8(void)
{
	clearn_oled();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	strncpy(str, "Set parametr 8", sizeof(str));
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	do{
		// Doing settings <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_4[0];										// Set global pointer on first menu
	action();															// Print items on OLED
}
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
void Menu_Init (void)
{
	// Make pointers on funsctions
	// Main functions
	void (*p_print_rows_on_oled_if_up) (void);
	p_print_rows_on_oled_if_up = print_rows_on_oled_if_up;

	void (*p_print_rows_on_oled_if_down) (void);					// Create pointer on function
	p_print_rows_on_oled_if_down = print_rows_on_oled_if_down;		// Save function print on pointer print_p

	void (*p_return_from_menu)(void);
	p_return_from_menu = return_from_menu;

	void (*p_action) (void);										// Create pointer on function
	p_action = action;												// Save function action on pointer action_p

	// items_menu_1 menu functions
	void (*p_do_it_function_menu_1) (void);						// Function "Do it". Works when select it
	p_do_it_function_menu_1 = do_it_function_menu_1;

	void (*p_items_menu_1_set_par_1) (void);
	p_items_menu_1_set_par_1 = items_menu_1_set_par_1;

	void (*p_items_menu_1_set_par_2) (void);
	p_items_menu_1_set_par_2 = items_menu_1_set_par_2;

	void (*p_items_menu_1_set_par_3) (void);
	p_items_menu_1_set_par_3 = items_menu_1_set_par_3;

	// items_menu_2 menu functions
	void (*p_do_it_function_menu_2) (void);						// Function "Do it". Works when select it
	p_do_it_function_menu_2 = do_it_function_menu_2;

	void (*p_items_menu_2_set_par_1) (void);
	p_items_menu_2_set_par_1 = items_menu_2_set_par_1;

	// items_menu_3 menu functions
	void (*p_do_it_function_menu_3) (void);						// Function "Do it". Works when select it
	p_do_it_function_menu_3 = do_it_function_menu_3;

	// items_menu_4 menu functions
	void (*p_items_menu_4_set_par_1) (void);
	p_items_menu_4_set_par_1 = items_menu_4_set_par_1;

	void (*p_items_menu_4_set_par_2) (void);
	p_items_menu_4_set_par_2 = items_menu_4_set_par_2;

	void (*p_items_menu_4_set_par_3) (void);
	p_items_menu_4_set_par_3 = items_menu_4_set_par_3;

	void (*p_items_menu_4_set_par_4) (void);
	p_items_menu_4_set_par_4 = items_menu_4_set_par_4;

	void (*p_items_menu_4_set_par_5) (void);
	p_items_menu_4_set_par_5 = items_menu_4_set_par_5;

	void (*p_items_menu_4_set_par_6) (void);
	p_items_menu_4_set_par_6 = items_menu_4_set_par_6;

	void (*p_items_menu_4_set_par_7) (void);
	p_items_menu_4_set_par_7 = items_menu_4_set_par_7;

	void (*p_items_menu_4_set_par_8) (void);
	p_items_menu_4_set_par_8 = items_menu_4_set_par_8;

	void (*p_do_it_function_menu_4) (void);						// Function "Do it". Works when select it
	p_do_it_function_menu_4 = do_it_function_menu_4;

	// Fill in elements(nodes) of list (7 items)
	// Main menu items
	/////////////////////////////////////////////////////////////////

	items[0].up = 0;
	items[0].down = &items[1];
	items[0].child = &items_menu_1[0];
	items[0].parent = 0;
	items[0].id = 1;
	items[0].name = "Menu_1";
	items[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items[0].makeAction = 0;

	items[1].up = &items[0];
	items[1].down = &items[2];
	items[1].child = &items_menu_2[0];
	items[1].parent = 0;
	items[1].id = 2;
	items[1].name = "Menu_2";
	items[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items[1].makeAction = 0;

	items[2].up = &items[1];
	items[2].down = &items[3];
	items[2].child = &items_menu_3[0];
	items[2].parent = 0;
	items[2].id = 3;
	items[2].name = "Menu_3";
	items[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items[2].makeAction = 0;

	items[3].up = &items[2];
	items[3].down = &items[4];
	items[3].child = &items_menu_4[0];
	items[3].parent = 0;
	items[3].id = 4;
	items[3].name = "Menu_4";
	items[3].updateScreen_up = p_print_rows_on_oled_if_up;
	items[3].updateScreen_down = p_print_rows_on_oled_if_down;
	items[3].makeAction = 0;

	items[4].up = &items[3];
	items[4].down = &items[5];
	items[4].child = 0;
	items[4].parent = 0;
	items[4].id = 5;
	items[4].name = "Menu_5";
	items[4].updateScreen_up = p_print_rows_on_oled_if_up;
	items[4].updateScreen_down = p_print_rows_on_oled_if_down;
	items[4].makeAction = 0;

	items[5].up = &items[4];
	items[5].down = &items[6];
	items[5].child = 0;
	items[5].parent = 0;
	items[5].id = 6;
	items[5].name = "Menu_6";
	items[5].updateScreen_up = p_print_rows_on_oled_if_up;
	items[5].updateScreen_down = p_print_rows_on_oled_if_down;
	items[5].makeAction  = 0;

	items[6].up = &items[5];
	items[6].down = 0;
	items[6].child = 0;
	items[6].parent = 0;
	items[6].id = 7;
	items[6].name = "Menu_7";
	items[6].updateScreen_up = p_print_rows_on_oled_if_up;
	items[6].updateScreen_down = p_print_rows_on_oled_if_down;
	items[6].makeAction  = 0;

	///////////////////////////////////////////////////////////////////
	// Creating next menu
	items_menu_1[0].up = 0;
	items_menu_1[0].down = &items_menu_1[1];
	items_menu_1[0].id = 1;
	items_menu_1[0].child = 0;
	items_menu_1[0].parent = &items[0];
	items_menu_1[0].name = "set par 1";
	items_menu_1[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[0].makeAction = p_items_menu_1_set_par_1;

	items_menu_1[1].up = &items_menu_1[0];
	items_menu_1[1].down = &items_menu_1[2];
	items_menu_1[1].child = 0;
	items_menu_1[1].parent = &items[0];
	items_menu_1[1].id = 2;
	items_menu_1[1].name = "set par 2";
	items_menu_1[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[1].makeAction = p_items_menu_1_set_par_2;

	items_menu_1[2].up = &items_menu_1[1];
	items_menu_1[2].down = &items_menu_1[3];
	items_menu_1[2].child = 0;
	items_menu_1[2].parent = &items[0];
	items_menu_1[2].id = 3;
	items_menu_1[2].name = "set par 3";
	items_menu_1[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[2].makeAction = p_items_menu_1_set_par_3;

	items_menu_1[3].up = &items_menu_1[2];
	items_menu_1[3].down = &items_menu_1[4];
	items_menu_1[3].child = 0;
	items_menu_1[3].parent = &items[0];
	items_menu_1[3].id = 4;
	items_menu_1[3].name = "DO IT";						// Name of item
	items_menu_1[3].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[3].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[3].makeAction = p_do_it_function_menu_1;

	items_menu_1[4].up = &items_menu_1[3];
	items_menu_1[4].down = 0;
	items_menu_1[4].child = 0;
	items_menu_1[4].parent = &items[0];
	items_menu_1[4].id = 5;
	items_menu_1[4].name = "EXIT";						// Name of item
	items_menu_1[4].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[4].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[4].makeAction = p_return_from_menu;

	///////////////////////////////////////////////////////////////////
	// Creating next menu
	items_menu_2[0].up = 0;
	items_menu_2[0].down = &items_menu_2[1];
	items_menu_2[0].child = 0;
	items_menu_2[0].parent = &items[1];
	items_menu_2[0].id = 1;
	items_menu_2[0].name = "set par 1";						// Name of item
	items_menu_2[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[0].makeAction = p_items_menu_2_set_par_1;

	items_menu_2[1].up = &items_menu_2[0];
	items_menu_2[1].down = &items_menu_2[2];
	items_menu_2[1].child = 0;
	items_menu_2[1].parent = &items[1];
	items_menu_2[1].id = 2;
	items_menu_2[1].name = "DO IT";						// Name of item
	items_menu_2[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[1].makeAction = p_do_it_function_menu_2;

	items_menu_2[2].up = &items_menu_2[1];
	items_menu_2[2].down = 0;
	items_menu_2[2].child = 0;
	items_menu_2[2].parent = &items[1];
	items_menu_2[2].id = 3;
	items_menu_2[2].name = "EXIT";						// Name of item
	items_menu_2[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[2].makeAction = p_return_from_menu;

	///////////////////////////////////////////////////////////////////
	// Creating next menu
	items_menu_3[0].up = 0;
	items_menu_3[0].down = &items_menu_3[1];
	items_menu_3[0].child = 0;
	items_menu_3[0].parent = &items[2];
	items_menu_3[0].id = 1;
	items_menu_3[0].name = "DO IT";						// Name of item
	items_menu_3[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_3[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_3[0].makeAction = p_do_it_function_menu_3;

	items_menu_3[1].up = &items_menu_3[0];
	items_menu_3[1].down = 0;
	items_menu_3[1].child = 0;
	items_menu_3[1].parent = &items[2];
	items_menu_3[1].id = 2;
	items_menu_3[1].name = "EXIT";						// Name of item
	items_menu_3[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_3[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_3[1].makeAction = p_return_from_menu;

	///////////////////////////////////////////////////////////////////
	// Creating next menu
	items_menu_4[0].up = 0;
	items_menu_4[0].down = &items_menu_4[1];
	items_menu_4[0].child = 0;
	items_menu_4[0].parent = &items[3];
	items_menu_4[0].id = 1;
	items_menu_4[0].name = "set par 1";						// Name of item
	items_menu_4[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[0].makeAction = p_items_menu_4_set_par_1;

	items_menu_4[1].up = &items_menu_4[0];
	items_menu_4[1].down = &items_menu_4[2];
	items_menu_4[1].child = 0;
	items_menu_4[1].parent = &items[3];
	items_menu_4[1].id = 2;
	items_menu_4[1].name = "set par 2";						// Name of item
	items_menu_4[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[1].makeAction = p_items_menu_4_set_par_2;

	items_menu_4[2].up = &items_menu_4[1];
	items_menu_4[2].down = &items_menu_4[3];
	items_menu_4[2].child = 0;
	items_menu_4[2].parent = &items[3];
	items_menu_4[2].id = 3;
	items_menu_4[2].name = "set par 3";						// Name of item
	items_menu_4[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[2].makeAction = p_items_menu_4_set_par_3;

	items_menu_4[3].up = &items_menu_4[2];
	items_menu_4[3].down = &items_menu_4[4];
	items_menu_4[3].child = 0;
	items_menu_4[3].parent = &items[3];
	items_menu_4[3].id = 4;
	items_menu_4[3].name = "set par 4";						// Name of item
	items_menu_4[3].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[3].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[3].makeAction = p_items_menu_4_set_par_4;

	items_menu_4[4].up = &items_menu_4[3];
	items_menu_4[4].down = &items_menu_4[5];
	items_menu_4[4].child = 0;
	items_menu_4[4].parent = &items[3];
	items_menu_4[4].id = 5;
	items_menu_4[4].name = "set par 5";						// Name of item
	items_menu_4[4].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[4].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[4].makeAction = p_items_menu_4_set_par_5;

	items_menu_4[5].up = &items_menu_4[4];
	items_menu_4[5].down = &items_menu_4[6];
	items_menu_4[5].child = 0;
	items_menu_4[5].parent = &items[3];
	items_menu_4[5].id = 6;
	items_menu_4[5].name = "set par 6";						// Name of item
	items_menu_4[5].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[5].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[5].makeAction = p_items_menu_4_set_par_6;

	items_menu_4[6].up = &items_menu_4[5];
	items_menu_4[6].down = &items_menu_4[7];
	items_menu_4[6].child = 0;
	items_menu_4[6].parent = &items[3];
	items_menu_4[6].id = 7;
	items_menu_4[6].name = "set par 7";						// Name of item
	items_menu_4[6].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[6].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[6].makeAction = p_items_menu_4_set_par_7;

	items_menu_4[7].up = &items_menu_4[6];
	items_menu_4[7].down = &items_menu_4[8];
	items_menu_4[7].child = 0;
	items_menu_4[7].parent = &items[3];
	items_menu_4[7].id = 8;
	items_menu_4[7].name = "set par 8";						// Name of item
	items_menu_4[7].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[7].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[7].makeAction = p_items_menu_4_set_par_8;

	items_menu_4[8].up = &items_menu_4[7];
	items_menu_4[8].down = &items_menu_4[9];
	items_menu_4[8].child = 0;
	items_menu_4[8].parent = &items[3];
	items_menu_4[8].id = 9;
	items_menu_4[8].name = "DO IT";						// Name of item
	items_menu_4[8].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[8].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[8].makeAction = p_do_it_function_menu_4;

	items_menu_4[9].up = &items_menu_4[8];
	items_menu_4[9].down = 0;
	items_menu_4[9].child = 0;
	items_menu_4[9].parent = &items[3];
	items_menu_4[9].id = 10;
	items_menu_4[9].name = "EXIT";						// Name of item
	items_menu_4[9].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_4[9].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_4[9].makeAction = p_return_from_menu;
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
void menu(void)
{
	Menu_Init();									// Init all structures

	print_menu_init();								// Print start menu and scrolingbar

	HAL_Delay(10);

	while(1)
	{
		if(button_processed_status == 1)			// If buttons was pressed
		{

			button_processed_status = 1;
			switch (button_status)
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
			button_status = BOTTON_DOESENT_PRESS;
		}
	}
}


// ---------------------------------------------------------------------------------


