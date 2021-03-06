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

#include <LoRa_E220_900T22D/e220_900t22d.h>

#include <NRF24L01/nrf24l01.h>

#include <am2302/am2302.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim2;

//extern struct am3202_sensor;
extern bool ready_to_work;
extern bool error_state;
extern bool am2302_ready;
int measure_counter = 0;		// For am3202
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
#define MENU_ITEM_NUM 3				// Main menu		(first menu level)
#define MENU_1_ITEM_NUM 3			// LoRa menu 		(second menu level)
#define MENU_1_1_ITEM_NUM 3			// LoRa TX menu		(third menu level)
#define MENU_2_ITEM_NUM 3			// NRF menu			(second menu level)
#define MENU_2_1_ITEM_NUM 3			// NRF TX menu 		(third menu level)
#define MENU_3_ITEM_NUM 4			// Sensor menu		(second menu level)

//#define MENU_4_ITEM_NUM 10

// Create menu item array structure for all menus
MenuItem_t items[MENU_ITEM_NUM];
MenuItem_t items_menu_1[MENU_1_ITEM_NUM];
MenuItem_t items_menu_2[MENU_2_ITEM_NUM];
MenuItem_t items_menu_3[MENU_3_ITEM_NUM];
MenuItem_t items_menu_1_1[MENU_1_1_ITEM_NUM];
MenuItem_t items_menu_2_1[MENU_2_1_ITEM_NUM];
//MenuItem_t items_menu_4[MENU_4_ITEM_NUM];

MenuItem_t * currentItem = &items[0];								// Create and set pointer on first element of list

bool block_interrupt_form_up_and_down_buttons = false;				// Flag for lock interrupt from 'up' and 'down' buttons in some cases
uint8_t button_status = BOTTON_DOESENT_PRESS;						// Pressed button
extern int button_processed_status;									// For interrupt work only one time

char str_pointer[4] = "->";											// How look pointer on menu item

char str_buf_1[16] = {0};

// OLED Rows coordinates
uint16_t first_menu_row = 16;
uint16_t second_menu_row = 28;
uint16_t third_menu_row = 40;
uint16_t fourth_menu_row = 52;
uint8_t row_step = 12;
uint16_t start_print_id_menu_x = 15;
uint16_t start_print_name_menu_x = 30;

// AM2302
extern  uint32_t i; 											// Counter transmitted data
extern int test_data;									  		// Init test data for transmit
extern uint32_t retr_cnt_full;
extern uint32_t cnt_lost;

void scroll_bar(void);
void print_rectangle_on_head(void);
void clear_menu_items (bool first, bool second, bool third, bool fourth);
void print_rows_on_oled_if_up(void);											// print text menu items
void print_rows_on_oled_if_down(void);											// print text menu items
void print_menu_init(void);
void print_menu_items(void);
void return_from_menu(void);
// Don't used functions
void items_menu_1_set_par_1(void);
void items_menu_1_set_par_2(void);
void items_menu_2_set_par_1(void);
void do_it_function_menu_3(void);
//

void lora_tx_mode(void);						// Transmit test data (counter)
void lora_tx_mode_send_T_and_H(void);			// Transmit H and H
void lora_rx_mode(void);						// Receive data from LoRa module
void nrf_tx_mode_send_test_number(void);		// Transmit test data (counter)
void nrf_tx_mode_send_T_and_H(void);			// Transmit H and H
void nrf_rx_mode(void);							// Receive data from NRF module
void am2302(void);								// Measure T and H using TIMER2 (timer will be off if out from this function)
void periodic_measurement_am2302_on(void);		// On Measure T and H using TIMER2 (Use it for TX data by NRF or LoRa)
void periodic_measurement_am2302_off(void);		// OFF Measure T and H using TIMER2

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
	// ------------------------------------------------------

	// LoRa  menu functions
	void (*p_lora_rx_mode) (void);						// Function "Do it". Works when select it
	p_lora_rx_mode = lora_rx_mode;
	void (*p_lora_tx_mode) (void);						// Function "Do it". Works when select it
	p_lora_tx_mode = lora_tx_mode;
	void (*p_lora_tx_mode_send_T_and_H) (void);
	p_lora_tx_mode_send_T_and_H = lora_tx_mode_send_T_and_H;


	// ------------------------------------------------------
	// NRF menu functions
	void (*p_nrf_tx_mode_send_test_number) (void);						// Function "Do it". Works when select it
	p_nrf_tx_mode_send_test_number = nrf_tx_mode_send_test_number;
	void (*p_nrf_tx_mode_send_T_and_H) (void);
	p_nrf_tx_mode_send_T_and_H = nrf_tx_mode_send_T_and_H;
	void (*p_nrf_rx_mode) (void);						// Function "Do it". Works when select it
	p_nrf_rx_mode = nrf_rx_mode;

	// ------------------------------------------------------
	// NRF menu functions
	void (*p_am2302_measure) (void);
	p_am2302_measure = am2302;
	// ------------------------------------------------------
	void (*p_periodic_measurement_am2302_on) (void);
	p_periodic_measurement_am2302_on = periodic_measurement_am2302_on;
	// ------------------------------------------------------
	void (*p_periodic_measurement_am2302_off) (void);
	p_periodic_measurement_am2302_off = periodic_measurement_am2302_off;
	// ------------------------------------------------------

//	void (*p_items_menu_1_set_par_2) (void);			// Doesen't use yet
//	p_items_menu_1_set_par_2 = items_menu_1_set_par_2;

	// items_menu_2 menu functions
//	void (*p_do_it_function_menu_2) (void);						// Function "Do it". Works when select it
//	p_do_it_function_menu_2 = do_it_function_menu_2;

//	void (*p_items_menu_2_set_par_1) (void);
//	p_items_menu_2_set_par_1 = items_menu_2_set_par_1;

//	// items_menu_3 menu functions
//	void (*p_do_it_function_menu_3) (void);						// Function "Do it". Works when select it
//	p_do_it_function_menu_3 = do_it_function_menu_3;


	//Баг, коли виходити з NFR RX меню, якщо передавалися дані T і H    <<<<<<<<<<<>

	// Fill in elements(nodes) of list (7 items)
	// Main menu items
	/////////////////////////////////////////////////////////////////
	items[0].up = 0;
	items[0].down = &items[1];
	items[0].child = &items_menu_1[0];
	items[0].parent = 0;
	items[0].id = 1;
	items[0].name = "LoRa E220";
	items[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items[0].makeAction = 0;

	items[1].up = &items[0];
	items[1].down = &items[2];
	items[1].child = &items_menu_2[0];
	items[1].parent = 0;
	items[1].id = 2;
	items[1].name = "NRF24L01";
	items[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items[1].makeAction = 0;

	items[2].up = &items[1];
	items[2].down = 0;
	items[2].child = &items_menu_3[0];
	items[2].parent = 0;
	items[2].id = 3;
	items[2].name = "AM2302 sensor";
	items[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items[2].makeAction = 0;

	///////////////////////////////////////////////////////////////////
	// Creating LoRa menu
	items_menu_1[0].up = 0;
	items_menu_1[0].down = &items_menu_1[1];
	items_menu_1[0].id = 1;
	items_menu_1[0].child = 0;
	items_menu_1[0].parent = &items[0];
	items_menu_1[0].name = "LoRa RX";
	items_menu_1[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[0].makeAction = p_lora_rx_mode;

	items_menu_1[1].up = &items_menu_1[0];
	items_menu_1[1].down = &items_menu_1[2];
	items_menu_1[1].child = &items_menu_1_1[0];
	items_menu_1[1].parent = &items[0];
	items_menu_1[1].id = 2;
	items_menu_1[1].name = "LoRa TX";
	items_menu_1[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[1].makeAction = 0;

	items_menu_1[2].up = &items_menu_1[1];
	items_menu_1[2].down = 0;
	items_menu_1[2].child = 0;
	items_menu_1[2].parent = &items[0];
	items_menu_1[2].id = 3;
	items_menu_1[2].name = "EXIT";						// Name of item
	items_menu_1[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1[2].makeAction = p_return_from_menu;


	// LoRa TX menu
	items_menu_1_1[0].up = 0;
	items_menu_1_1[0].down = &items_menu_1_1[1];
	items_menu_1_1[0].child = 0;
	items_menu_1_1[0].parent = &items_menu_1[0];
	items_menu_1_1[0].id = 1;
	items_menu_1_1[0].name = "TX Test data";
	items_menu_1_1[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1_1[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1_1[0].makeAction = p_lora_tx_mode;

	items_menu_1_1[1].up = &items_menu_1_1[0];
	items_menu_1_1[1].down = &items_menu_1_1[2];
	items_menu_1_1[1].child = 0;
	items_menu_1_1[1].parent = &items_menu_1[0];
	items_menu_1_1[1].id = 2;
	items_menu_1_1[1].name = "TX T & H";
	items_menu_1_1[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1_1[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1_1[1].makeAction = p_lora_tx_mode_send_T_and_H;

	items_menu_1_1[2].up = &items_menu_1_1[1];
	items_menu_1_1[2].down = 0;
	items_menu_1_1[2].child = 0;
	items_menu_1_1[2].parent = &items_menu_1[0];
	items_menu_1_1[2].id = 3;
	items_menu_1_1[2].name = "EXIT";
	items_menu_1_1[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_1_1[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_1_1[2].makeAction = p_return_from_menu;

	///////////////////////////////////////////////////////////////////
	// Creating NRF menu
	items_menu_2[0].up = 0;
	items_menu_2[0].down = &items_menu_2[1];
	items_menu_2[0].child = 0;
	items_menu_2[0].parent = &items[0]; 	//&items[0];    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	items_menu_2[0].id = 1;
	items_menu_2[0].name = "NRF RX";						// Name of item
	items_menu_2[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[0].makeAction = p_nrf_rx_mode;

	items_menu_2[1].up = &items_menu_2[0];
	items_menu_2[1].down = &items_menu_2[2];
	items_menu_2[1].child = &items_menu_2_1[0];
	items_menu_2[1].parent = &items[0];
	items_menu_2[1].id = 2;
	items_menu_2[1].name = "NRF TX";						// Name of item
	items_menu_2[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[1].makeAction = 0;

	items_menu_2[2].up = &items_menu_2[1];
	items_menu_2[2].down = 0;
	items_menu_2[2].child = 0;
	items_menu_2[2].parent = &items[0];
	items_menu_2[2].id = 3;
	items_menu_2[2].name = "EXIT";						// Name of item
	items_menu_2[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2[2].makeAction = p_return_from_menu;

	// NRF TX menu
	items_menu_2_1[0].up = 0;
	items_menu_2_1[0].down = &items_menu_2_1[1];
	items_menu_2_1[0].child = 0;
	items_menu_2_1[0].parent = &items_menu_2[0];
	items_menu_2_1[0].id = 1;
	items_menu_2_1[0].name = "TX Test data";
	items_menu_2_1[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2_1[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2_1[0].makeAction = p_nrf_tx_mode_send_test_number;

	items_menu_2_1[1].up = &items_menu_2_1[0];
	items_menu_2_1[1].down = &items_menu_2_1[2];
	items_menu_2_1[1].child = 0;
	items_menu_2_1[1].parent = &items_menu_2[0];
	items_menu_2_1[1].id = 2;
	items_menu_2_1[1].name = "TX T & H";
	items_menu_2_1[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2_1[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2_1[1].makeAction = p_nrf_tx_mode_send_T_and_H;

	items_menu_2_1[2].up = &items_menu_2_1[1];
	items_menu_2_1[2].down = 0;
	items_menu_2_1[2].child = 0;
	items_menu_2_1[2].parent = &items_menu_2[0];
	items_menu_2_1[2].id = 3;
	items_menu_2_1[2].name = "EXIT";
	items_menu_2_1[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_2_1[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_2_1[2].makeAction = p_return_from_menu;

	///////////////////////////////////////////////////////////////////
	// Creating sensor menu
	items_menu_3[0].up = 0;
	items_menu_3[0].down = &items_menu_3[1];
	items_menu_3[0].child = 0;
	items_menu_3[0].parent = &items[2];
	items_menu_3[0].id = 1;
	items_menu_3[0].name = "Measure T & H";						// Name of item
	items_menu_3[0].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_3[0].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_3[0].makeAction = p_am2302_measure;

	items_menu_3[1].up = &items_menu_3[0];
	items_menu_3[1].down = &items_menu_3[2];
	items_menu_3[1].child = 0;
	items_menu_3[1].parent = &items[2];
	items_menu_3[1].id = 2;
	items_menu_3[1].name = "Per Meas: ON";						// Name of item
	items_menu_3[1].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_3[1].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_3[1].makeAction = p_periodic_measurement_am2302_on;

	items_menu_3[2].up = &items_menu_3[1];
	items_menu_3[2].down = &items_menu_3[3];
	items_menu_3[2].child = 0;
	items_menu_3[2].parent = &items[2];
	items_menu_3[2].id = 3;
	items_menu_3[2].name = "Per Meas: OFF";						// Name of item
	items_menu_3[2].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_3[2].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_3[2].makeAction = p_periodic_measurement_am2302_off;

	items_menu_3[3].up = &items_menu_3[2];
	items_menu_3[3].down = 0;
	items_menu_3[3].child = 0;
	items_menu_3[3].parent = &items[2];
	items_menu_3[3].id = 4;
	items_menu_3[3].name = "EXIT";						// Name of item
	items_menu_3[3].updateScreen_up = p_print_rows_on_oled_if_up;
	items_menu_3[3].updateScreen_down = p_print_rows_on_oled_if_down;
	items_menu_3[3].makeAction = p_return_from_menu;
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
		print_menu_items();
	}
}
// ----------------------------------------------------------------------------------------

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
	// 1. Скопіювати показник на меню в буффер.
	// 2. Піднятися до першого елемента меню.
	// 3. Опускатися вниз до останнього пункту меню і інкрементувати лічильник елементів меню.
	// 4. Використати лічильник пунктів меню для вираховування довжини полоси прокрутки і її координат.

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
	int id_for_line = currentItem -> id;				// Скопіювати порядковий номер меню (Для того щоб взнати на якому пункті меню зараз стоїмо)
	// Print scroling line
	uint16_t line_lenght = (lenght_all_scrollbar/menu_items_counter + 1);					   	// Довжина лінії яка відповідає одному меню
	uint16_t start_lenght = 16 + ((id_for_line - 1)*line_lenght);								// Початок лінії
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
	char str[16] = "               ";   						// Must be 15

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
void print_rows_on_oled_if_up(void)								// print text menu items
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
		// Fill in OLED buffer
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
void print_rows_on_oled_if_down(void)							// print text menu items
{
	char str[16] = {0};
	clear_menu_items (true , true , true , true );
	print_rectangle_on_head();
	// Print pointer on first item menu
	ssd1306_SetCursor(0, first_menu_row);
	ssd1306_WriteString(str_pointer,  Font_7x10, White);

	MenuItem_t * currentItem_buff = currentItem;				// Create buffer on selected current item pointer.
	for (uint8_t row = first_menu_row; row <= fourth_menu_row; row = row + row_step)
	{
		// Fill in OLED buffer
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
	MenuItem_t * currentItem_buff = currentItem;		// Create buffer on selected current item pointer.
	print_rectangle_on_head();

	// Print ">> MAIN MENU <<" on head of OLED
	char str[20] = ">> MAIN MENU <<";
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	for(uint8_t row = first_menu_row; row <= fourth_menu_row; row = row + row_step)
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
		if(currentItem_buff == 0)
		{
			break;
		}
	}
	scroll_bar();
}
// ----------------------------------------------------------------------------------------
// print pointers of menu
void print_menu_items(void)
{
	char str[16] = {0};
	clearn_oled();
	print_rectangle_on_head();

	//Print selected name of menu on top of OLED (in rectangle)
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
// LORA FUNCTIONS
void lora_rx_mode(void)
{
	clearn_oled();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	// waiting for press enter(SW2) button
	do{
		LoRa_RX(true);
	}while (button_status != BUTTON_ENTER);
	LoRa_RX(false);

	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_1[0];										// Set global pointer on first menu
	print_menu_items();													// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void lora_tx_mode(void)
{
	clearn_oled();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));


	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	// waiting for press enter(SW2) button
	do{
		LoRa_TX_send_test_number(true);

	}while (button_status != BUTTON_ENTER);
	LoRa_TX_send_test_number(false);

	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_1[0];										// Set global pointer on first menu
	print_menu_items();													// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void lora_tx_mode_send_T_and_H(void)
{
	clearn_oled();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	// waiting for press enter(SW2) button
	do{
		LoRa_TX_send_T_and_H(true);

	}while (button_status != BUTTON_ENTER);
	LoRa_TX_send_T_and_H(false);

	block_interrupt_form_up_and_down_buttons = false;

	// Return to first item of current menu
	currentItem = &items_menu_1[0];										// Set global pointer on first menu
	print_menu_items();													// Print items on OLED
}
// ----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// NRF FUNCTIONS
void nrf_rx_mode(void)
{
	clearn_oled();
	NRF24_init_RX_mode();
	print_rectangle_on_head();

	char str_buf_1[25] = {0};
	strncpy(str_buf_1, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str_buf_1,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str_buf_1, 0, sizeof(str_buf_1));
	//-------------------------------------------------

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	// waiting for press enter(SW2) button
	do{
		NRF24L01_Receive();
	}while (button_status != BUTTON_ENTER);

	NRF24_WriteReg(CONFIG, 0x00); 										// STOP work with nrf module  (Power off)
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);				// Turn off GREEN LED
	block_interrupt_form_up_and_down_buttons = false;
	// Return to first item of current menu
	currentItem = &items_menu_2[0];										// Set global pointer on first menu
	print_menu_items();													// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void nrf_tx_mode_send_test_number(void)
{
	clearn_oled();
	NRF24_init_TX_mode();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	// waiting for press enter(SW2) button
	do{
		NRF24L01_Transmission_counter();
	}while (button_status != BUTTON_ENTER);
	i = 1; 																// Counter transmitted data
	test_data = 0;									  					// Init test data for transmit data
	retr_cnt_full = 0;
	cnt_lost = 0;

	NRF24_WriteReg(CONFIG, 0x00); 										// STOP work with nrf module  (Power off)
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);				// Turn off GREEN LED
	block_interrupt_form_up_and_down_buttons = false;
	// Return to first item of current menu
	currentItem = &items_menu_2[0];										// Set global pointer on first menu
	print_menu_items();													// Print items on OLED
}
// ----------------------------------------------------------------------------------------
void nrf_tx_mode_send_T_and_H(void)
{
	clearn_oled();
	NRF24_init_TX_mode();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	// waiting for press enter(SW2) button
	do{
		NRF24L01_Transmission_t_and_h();
	}while (button_status != BUTTON_ENTER);
	i = 1; 																// Counter transmitted data
	test_data = 0;									  					// Init test data for transmit data
	retr_cnt_full = 0;
	cnt_lost = 0;

	NRF24_WriteReg(CONFIG, 0x00); 										// STOP work with nrf module  (Power off)
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);				// Turn off GREEN LED
	block_interrupt_form_up_and_down_buttons = false;
	// Return to first item of current menu
	currentItem = &items_menu_2[0];										// Set global pointer on first menu
	print_menu_items();													// Print items on OLED
}

// ----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// SENSORS FUNCTIONS
// Function uses Tim2 for periodic measuring.
void am2302(void)
{
	HAL_TIM_Base_Start_IT(&htim2);		// For sensor measure
	HAL_Delay(100);
	clearn_oled();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;	// Lock interrupt from UP and DOWN buttons
	measure_counter = 1;
	// waiting for press enter(SW2) button
	do{
		// Print data
		if(am2302_ready == true)						// Flug from interrup tim2
		{
			char str_temperature[10] = {0};
			char str_humidity[10] = {0};
			char str_1[10] = {0};

			// Clear data place on OLED
			memset(str, ' ', sizeof(str));
			ssd1306_SetCursor(10, first_menu_row);
			ssd1306_WriteString(str,  Font_7x10, White);
			ssd1306_UpdateScreen();
			ssd1306_SetCursor(10, second_menu_row);
			ssd1306_WriteString(str,  Font_7x10, White);
			ssd1306_UpdateScreen();
			memset(str, 0, sizeof(str));

			// Print T and H on OLED
			itoa(am3202_sensor.temterature , str, 10);
			strcat(str_temperature, "T = ");
			strcat(str_temperature, str);
			strcat(str_temperature, " C");
			ssd1306_SetCursor(10, first_menu_row);
			ssd1306_WriteString(str_temperature,  Font_7x10, White);
			memset(str, 0,sizeof(str));

			itoa(am3202_sensor.humidity , str, 10);
			strcat(str_humidity, "H = ");
			strcat(str_humidity, str);
			strcat(str_humidity, " %");
			ssd1306_SetCursor(10, second_menu_row);
			ssd1306_WriteString(str_humidity,  Font_7x10, White);

			// Print counter
			memset(str, 0,sizeof(str));
			strcat(str, "Counter: ");
			itoa(measure_counter , str_1, 10);
			strcat(str, str_1);
			ssd1306_SetCursor(10, third_menu_row);
			ssd1306_WriteString(str,  Font_7x10, White);
			ssd1306_UpdateScreen();

			measure_counter++;
			am2302_ready = false;
		}

	}while (button_status != BUTTON_ENTER);
	HAL_TIM_Base_Stop_IT(&htim2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // Turn off LED
	block_interrupt_form_up_and_down_buttons = false;	// Unlock  UP and DOWN buttons interrupt

	am3202_sensor.temterature = 0;
	am3202_sensor.humidity = 0;

	// Return to first item of current menu
	currentItem = &items_menu_3[0];										// Set global pointer on first menu
	print_menu_items();
}
// ----------------------------------------------------------------------------------------
// Function uses Tim2 for periodic measuring.
void periodic_measurement_am2302_on(void)
{
	HAL_TIM_Base_Start_IT(&htim2);		// For sensor measure
	HAL_Delay(100);
	clearn_oled();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	measure_counter = 1;
	// waiting for press enter(SW2) button
	do{
		// Print data
		if(am2302_ready == true)	// Flug from interrup tim2
		{
			char str_temperature[10] = {0};
			char str_humidity[10] = {0};
			char str_1[10] = {0};

			// Clear data place on OLED
			memset(str, ' ', sizeof(str));
			ssd1306_SetCursor(10, first_menu_row);
			ssd1306_WriteString(str,  Font_7x10, White);
			ssd1306_UpdateScreen();
			ssd1306_SetCursor(10, second_menu_row);
			ssd1306_WriteString(str,  Font_7x10, White);
			ssd1306_UpdateScreen();

			memset(str, 0, sizeof(str));
			// Print T and H on OLED

			itoa(am3202_sensor.temterature , str, 10);
			strcat(str_temperature, "T = ");
			strcat(str_temperature, str);
			strcat(str_temperature, " C");
			ssd1306_SetCursor(10, first_menu_row);
			ssd1306_WriteString(str_temperature,  Font_7x10, White);

			memset(str, 0,sizeof(str));
			itoa(am3202_sensor.humidity , str, 10);
			strcat(str_humidity, "H = ");
			strcat(str_humidity, str);
			strcat(str_humidity, " %");
			ssd1306_SetCursor(10, second_menu_row);
			ssd1306_WriteString(str_humidity,  Font_7x10, White);

			// Print counter
			memset(str, 0,sizeof(str));
			strcat(str, "Counter: ");
			itoa(measure_counter , str_1, 10);
			strcat(str, str_1);
			ssd1306_SetCursor(10, third_menu_row);
			ssd1306_WriteString(str,  Font_7x10, White);
			ssd1306_UpdateScreen();

			measure_counter++;
			am2302_ready = false;
		}

	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;	// Unlock  UP and DOWN buttons interrupt
	// Return to first item of current menu
	currentItem = &items_menu_3[0];										// Set global pointer on first menu
	print_menu_items();
}
// ----------------------------------------------------------------------------------------
// Function turn off Tim2 for periodic measuring.
void periodic_measurement_am2302_off(void)
{
	HAL_TIM_Base_Start_IT(&htim2);		// For sensor measure
	HAL_Delay(100);
	clearn_oled();
	print_rectangle_on_head();

	// Print selected name of menu
	char str[16] = {0};
	strncpy(str, currentItem -> name, 15);
	ssd1306_SetCursor(10, 3);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	button_status = BOTTON_DOESENT_PRESS;
	block_interrupt_form_up_and_down_buttons = true;
	measure_counter = 1;
	HAL_TIM_Base_Stop_IT(&htim2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // Turn off LED

	ssd1306_SetCursor(10, second_menu_row);
	strcat(str, "STOP measuring");
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();
	memset(str, 0, sizeof(str));

	am3202_sensor.temterature = 0;
	am3202_sensor.humidity = 0;

	// waiting for press enter(SW2) button
	do{

	}while (button_status != BUTTON_ENTER);
	block_interrupt_form_up_and_down_buttons = false;	// Unlock  UP and DOWN buttons interrupt

	// Return to first item of current menu
	currentItem = &items_menu_3[0];										// Set global pointer on first menu
	print_menu_items();

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

	strncpy(str, "Set parameter 1", sizeof(str));
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
	print_menu_items();													// Print items on OLED
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

	strncpy(str, "Set parameter 2", sizeof(str));
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
	print_menu_items();													// Print items on OLED
}
// ----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//void do_it_function_menu_2(void)
//{
//	clearn_oled();
//
//	// Print selected name of menu
//	char str[16] = {0};
//	strncpy(str, currentItem -> name, 15);
//	ssd1306_SetCursor(10, 3);
//	ssd1306_WriteString(str,  Font_7x10, White);
//	ssd1306_UpdateScreen();
//	memset(str, 0, sizeof(str));
//
//	strncpy(str, "Doing something 2", sizeof(str));
//	ssd1306_SetCursor(0, first_menu_row);
//	ssd1306_WriteString(str,  Font_7x10, White);
//	ssd1306_UpdateScreen();
//
//	button_status = BOTTON_DOESENT_PRESS;
//	block_interrupt_form_up_and_down_buttons = true;
//	do{
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
//		HAL_Delay(50);
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
//		HAL_Delay(50);
//	}while (button_status != BUTTON_ENTER);
//	block_interrupt_form_up_and_down_buttons = false;
//
//	// Return to first item of current menu
//	currentItem = &items_menu_2[0];										// Set global pointer on first menu
//	print_menu_items();													// Print items on OLED
//}
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
	print_menu_items();													// Print items on OLED
}
// ----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void do_it_function_menu_3(void)        // Print T and H
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
	print_menu_items();													// Print items on OLED
}


