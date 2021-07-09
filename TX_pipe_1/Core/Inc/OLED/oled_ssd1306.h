/*
 * oled_ssd1306.h
 *
 *  Created on: Jul 3, 2021
 *      Author: odemki
 */

#ifndef INC_OLED_OLED_SSD1306_H_
#define INC_OLED_OLED_SSD1306_H_

#include "stdbool.h"

void init_oled(void);
void test_oled(void);
void print_main_menu(void);
void OLED_prinr_all_data(int GPS_SELECTED);

void claen_oled_lines(bool first, bool second, bool third, bool fourth, bool fifth);
void print_text_on_OLED(uint8_t column, uint8_t row, bool update_oled, char text[]);


#endif /* INC_OLED_OLED_SSD1306_H_ */
