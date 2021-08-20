/*
 * oled_main.c
 *
 *  Created on: Aug 17, 2021
 *      Author: odemki
 */
#include <OLED/fonts.h>
#include <OLED/oled_ssd1306.h>
#include <OLED/ssd1306.h>

void init_oled(void)
{
	  ssd1306_Init();
	  ssd1306_Fill(Black);
	  ssd1306_UpdateScreen();
}
//----------------------------------------------------------------------------------------
void clearn_oled(void)
{
	 ssd1306_Fill(Black);
	 ssd1306_UpdateScreen();
}
//----------------------------------------------------------------------------------------
