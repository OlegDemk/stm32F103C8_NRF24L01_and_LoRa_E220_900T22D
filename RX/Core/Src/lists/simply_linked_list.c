/*
 * lists.c
 *
 *  Created on: Aug 21, 2021
 *      Author: odemki
 */

#include "main.h"
#include <stdio.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart3;

// Structure list element (node)
typedef struct s_kist{
	int id;
	char *name;

	struct s_list *next;
}t_list;


t_list *create_node(int set_id, char *set_name);
void push_front(t_list **list, int set_id, char *set_name);
void push_back(t_list **list, int set_id, char *set_name);
void add_list(t_list **list, int id);

void print_all_list(t_list **list);

// ---------------------------------------------------------------------------------
int simply_linced_lists()
{
	static char test_str[] = "Simply linced lists\n\r";
    HAL_UART_Transmit(&huart3, test_str, sizeof(test_str), 1000);
 	HAL_Delay(1000);
 	memset(test_str, 0, sizeof(test_str));



    // Створення указателя на структуру t_list і присвоєння йому адреса новоствореного першого елемента списку
	t_list *list = create_node(0, "Name 0");			// Створення нульового елемента списку

	push_back(&list, 1, "Back:Name -1");
	push_back(&list, 2, "Back:Name -2");

	push_front(&list, 2, "Front:Name 2");					// Додати новий елемент на початок списку
	push_front(&list, 3, "Front:Name 3");
	push_front(&list, 4, "Front:Name 4");


	add_list(&list, 0);
	// Додати новий елемент списку в середину списку ////
	t_list *tmp = list;
	while(tmp -> next != NULL)		// шукаємо останній елемент списку
	{
		if(tmp -> id == 0)			// Якщо	знайдений елемент з 0 айдішкою
		{
			t_list *new_element = create_node(1000, "INSERT");
			new_element -> next = tmp -> next;
			tmp -> next = new_element;
		}
		tmp = tmp -> next;
	}
	/////////////////////////////////////////////////////
	// Видалити елемент списку (Видаляє найперший елемент списку)
	t_list *to_delete = list;
	list = list -> next;
	free(to_delete);

	/////////////////////////////////////////////////////

//	print_all_list(&list);

		char str_id[10] = {0};
	 	char str_name[20] = {0};
	    char str_buffer[80] = {0};

	while (list != NULL)
	{
		itoa(list -> id, str_id, 10);
		strcat(str_name, list -> name);
		strcat(str_buffer, str_id);
		strcat(str_buffer, ": ");
		strcat(str_buffer, str_name);
		strcat(str_buffer, "\r\n");


		HAL_UART_Transmit(&huart3, str_buffer, sizeof(str_buffer), 1000);

		list = list -> next;						// перейти на наступний елемент списку

		HAL_Delay(100);
		memset(str_id, 0, sizeof(str_id));
		memset(str_name, 0, sizeof(str_name));
		memset(str_buffer, 0, sizeof(str_buffer));
	}

	while(1)
	{

	}
	return 0;
}
// ---------------------------------------------------------------------------------
void add_list(t_list **list, int id )
{
	t_list *tmp = list;
	while(tmp -> next != NULL)		// шукаємо останній елемент списку
	{
		if(tmp -> id == 0)			// Якщо	знайдений елемент з 0 айдішкою
		{
			t_list *new_element = create_node(1000, "INSERT");
			new_element -> next = tmp -> next;
			tmp -> next = new_element;
		}
		tmp = tmp -> next;
	}
}

// ---------------------------------------------------------------------------------
t_list *create_node(int set_id, char *set_name)
{
	t_list *node = (t_list *)malloc(sizeof(t_list));	// Виділення памяті для структури

	node -> id = set_id;			// Заповнення поля нового елемента списку
	node -> name = set_name;

	node -> next = NULL;			// Адрес на наступний елемент

	return node;
}
// ---------------------------------------------------------------------------------
void push_front(t_list **list, int set_id, char *set_name)
{
	t_list *new_element = create_node(set_id, set_name);

	new_element -> next = *list;					// Адрес нового на наступний елемент ставиться попередній
	*list = new_element;							// Присвоюємо адрес тільки що доданого елемента
}
// ---------------------------------------------------------------------------------
void push_back(t_list **list, int set_id, char *set_name)
{
	t_list *new_element = create_node(set_id, set_name);	// Створюємо новий елемент списку
	t_list *tmp = *list;									// його адрес записуємо в буфер

	while(tmp -> next != NULL)								// Знайти останній елемент списку (tmp -> next == NULL)
	{
		tmp = tmp -> next;			// Прохід по полям структури з адресом
	}
	// Якщо знайдено останній елемент списку
	tmp -> next = new_element;		 // тоді ми записуємо в поле адресу останнього, адрес на доданий елемент списку
}
// ---------------------------------------------------------------------------------
//print_all_list(t_list **list)
//{
//	char str_id[10] = {0};
// 	char str_name[20] = {0};
//    char str_buffer[80] = {0};
//
//	while (list != NULL)
//		{
//			itoa(list -> id, str_id, 10);
//			strcat(str_name, list -> name);
//			strcat(str_buffer, str_id);
//			strcat(str_buffer, ": ");
//			strcat(str_buffer, str_name);
//			strcat(str_buffer, "\r\n");
//
//
//			HAL_UART_Transmit(&huart3, str_buffer, sizeof(str_buffer), 1000);
//
//			list = list -> next;						// перейти на наступний елемент списку
//
//			HAL_Delay(100);
//			memset(str_id, 0, sizeof(str_id));
//			memset(str_name, 0, sizeof(str_name));
//			memset(str_buffer, 0, sizeof(str_buffer));
//		}
//}
