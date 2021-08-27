/*
 * lists.c
 *
 *  Created on: Aug 22, 2021
 *      Author: odemki
 */

#include <OLED/fonts.h>
#include <OLED/oled_ssd1306.h>
#include <OLED/ssd1306.h>
#include <OLED/oled_main.h>

typedef struct Struct{
	int id;
	void ( *function) ( int k );
	void ( *function_2) (struct  Struct *item_2 );
	struct Struct* down;
} Struct_t;

Struct_t item_1[3];							// Create array of struct
Struct_t item_2[3];

Struct_t * cittent_item = &item_1[0];
// ----------------------------------------------------------------------------------------
void function_do(int k)
{
	char str[30] = "TEST";

	if(k == 1)
	{
		ssd1306_SetCursor(30, 16);
		ssd1306_WriteString(str,  Font_7x10, White);
		ssd1306_UpdateScreen();
	}
	if(k == 2)
	{
		ssd1306_SetCursor(30, 28);
		ssd1306_WriteString(str,  Font_7x10, White);
		ssd1306_UpdateScreen();
	}
	if(k == 3)
	{
		ssd1306_SetCursor(30, 40);
		ssd1306_WriteString(str,  Font_7x10, White);
		ssd1306_UpdateScreen();
	}


}
// ----------------------------------------------------------------------------------------
void function_2 (struct Struct *p)
{
	char str[30] = "function_2";
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(str,  Font_7x10, White);
	ssd1306_UpdateScreen();

	int n = p -> id;       // <<<<<<<<<<<<<<<  ERROR


	int j = 0;

}
// ----------------------------------------------------------------------------------------
void lists (void)
{
	void (*p_function) (int k);
	void (*p_function_2) (struct Struct * item_2);
	p_function = function_do;
	p_function_2 = function_2;

	//simply_linced_lists();
	item_1[0].id = 1;
	item_1[0].function = p_function;
	item_1[0].down = &item_1[1];

	item_1[1].id = 2;
	item_1[1].function = p_function;
	item_1[1].down = &item_1[2];

	item_1[2].id = 3;
	item_1[2].function = p_function;
	item_1[2].down = &item_1[0];

	////////////////////////////////////////////////////////////
	item_2[0].id = 99;
	item_2[0].function = p_function;
	item_2[0].down = &item_2[1];

	item_2[1].id = 2;
	item_2[1].function = p_function;
	item_2[1].down = 0;


	while(1)
	{
		function_2 (&item_2[0]);     // <<<<<<<

		for(int i = 1; i < 4; i++)
		{
			cittent_item -> function(i);
		    cittent_item = cittent_item -> down;
		    HAL_Delay(300);
		}
		clearn_oled();
		HAL_Delay(1000);

	}






}
// ----------------------------------------------------------------------------------------

