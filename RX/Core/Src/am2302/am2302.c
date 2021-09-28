/*
 * am2302.c
 *
 *  Created on: Sep 28, 2021
 *      Author: odemki
 */
#include "main.h"

bool ready_to_work = true;
bool error_state = false;

struct{
	uint8_t temterature;
	uint8_t humidity;
	bool status;
}am3202_sensor;
//----------------------------------------------------------------------------------------
/*
 * Function make us delay
 */
__STATIC_INLINE void DelayMicro(__IO uint32_t micros)
{
	uint32_t test_micros = SystemCoreClock;
	micros *= (SystemCoreClock / 100000) /84;
	while (micros--);
}
//----------------------------------------------------------------------------------------
void init_am2302(void)
{
//	GPIO_InitTypeDef GPIO_InitStruct = {0};		// Make struct
//	GPIO_InitStruct.Pin = GPIO_PIN_12;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_Delay(2000); 							// First init must be 2 seconds delay
//	am2302_measure(); 							// For fill in i2c_device.AM2302_ready_status


//	GPIOC->MODER |= GPIO_MODER_MODER1_0;            // Output mode GPIOC0
//	GPIOC->OTYPER &= ~GPIO_OTYPER_OT_1;             // Push-pull mode
//	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0_1;     // Speed
//	GPIOC->ODR ^= 0x02; 							// set GPIOC pin 1 on high
//	HAL_Delay(2000); 							// First init must be 2 seconds delay
//	am2302_measure(); 							// For fill in i2c_device.AM2302_ready_status
}
//----------------------------------------------------------------------------------------
void am2302_measure(void)
{
	//  function must use less than one time per 2-3 seconds.
/* Init work with sensor:
 *
 * From microcontroller
 * 						            From sensor
 * 	   Low 10 msec	      High 39 us|	80us Pull down     80us Pull up 	Start Receive data from sensor
 * ____                     _______	|					 __________________
 * 	   \	   	           /	   \|					/				   \
 * 	   	\_________________/			\__________________/		 			\_______
 *
 * Receive '0' Bit
 *     Low 50 us    High 26 - 28 us
 * __                ___________
 * 	 \			    /			\
 * 	  \____________/			 \_
 *
 * Receive '1' Bit
 *     Low 50 us             High 70 us
 * __                ________________________
 * 	 \			    /				         \
 * 	  \____________/			              \_
 */

	bool get_data_status = false;
	int j = 0;   							// Counter bytes
	int i = 0;								// Counter bits
	uint8_t data[4] = {0};					// Buffer for incoming data from sensor
	float temper, hum;						// Buffer variables

	// Init GPIO like output
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
	GPIO_InitTypeDef GPIO_InitStruct = {0};		// Make struct
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Make output pin C1
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	DelayMicro(18000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	DelayMicro(39);

	// Make input pin C1
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))
	{
		am3202_sensor.status = error_state; 					// Error. Sensor not response
	}
	else
	{
		am3202_sensor.status = ready_to_work;
	}

	DelayMicro(80);
	if(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)))
	{
		am3202_sensor.status = error_state; 					// Error. Sensor not response
	}
	else
	{
		am3202_sensor.status = ready_to_work;
	}

	DelayMicro(80);

	if(am3202_sensor.status == ready_to_work)
	{
		for(j = 0; j <5; j++)							// Reading 5 bytes
		{
			data[4-j] = 0;
			for(i = 0; i < 8; i++)						// Reading 8 bits
			{

				while(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)));    // While signal is "0"
				DelayMicro(30);

				if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))   // If signal is high when wrute "1" in buffer (data[])
				{
					data[4-j] |= (1 << (7 - i));        // Shift received bite
				}

				while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12));		// Wait end of "1" signal
			}
			am3202_sensor.status = true;										// Data was been written okay
		}

		temper = (float)((*(uint16_t*)(data+1)) & 0x3FFF) /10;
		if((*(uint16_t*)(data+1)) & 0x8000) temper  *= -1.0;
		am3202_sensor.temterature = temper;

		hum = (float)(*(int16_t*)(data+3)) / 10;
		am3202_sensor.humidity = hum;
	}
	else
	{
		am3202_sensor.status = error_state;
		// PRINT MESSAGE ON OLLED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}

}
//----------------------------------------------------------------------------------------
//void am2302_measure(void)
//{
//	//  function must use less than one time per 2-3 seconds.
///* Init work with sensor:
// *
// * From microcontroller
// * 						            From sensor
// * 	   Low 10 msec	      High 39 us|	80us Pull down     80us Pull up 	Start Receive data from sensor
// * ____                     _______	|					 __________________
// * 	   \	   	           /	   \|					/				   \
// * 	   	\_________________/			\__________________/		 			\_______
// *
// * Receive '0' Bit
// *     Low 50 us    High 26 - 28 us
// * __                ___________
// * 	 \			    /			\
// * 	  \____________/			 \_
// *
// * Receive '1' Bit
// *     Low 50 us             High 70 us
// * __                ________________________
// * 	 \			    /				         \
// * 	  \____________/			              \_
// */
//
//	bool get_data_status = false;
//	int j = 0;   							// Counter bytes
//	int i = 0;								// Counter bits
//	uint8_t data[4] = {0};					// Buffer for incoming data from sensor
//	float temper, hum;						// Buffer variables
//
//	// Init GPIO like output
//	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
//	GPIO_InitTypeDef GPIO_InitStruct = {0};		// Make struct
//	GPIO_InitStruct.Pin = GPIO_PIN_12;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//	// Init GPIO like output
////	GPIOC->MODER |= GPIO_MODER_MODER1_0;            // Output mode GPIOC0
////	GPIOC->OTYPER &= ~GPIO_OTYPER_OT_1;             // Push-pull mode
////	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0_1;     // Speed
//
//	// Make output pin C1
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
//	DelayMicro(18000);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
//	DelayMicro(39);
//
////	// Make output pin C1
////	GPIOC->ODR &= ~0x02;		// Low level
////	DelayMicro(18000);
////	GPIOC->ODR ^= 0x02;			// High level
////	DelayMicro(39);
//
//	// Make input pin C1
//	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
//	GPIO_InitStruct.Pin = GPIO_PIN_12;
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//	GPIO_InitStruct.Pull = GPIO_PULLUP;
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
////	// Make input pin C1
////	GPIOC->MODER &= ~0x04;  	// Set Pin C1 Input   (MODER GPIOC_1 Must be 00)
////	GPIOC->PUPDR &= ~0x04;		// Set Pin C1 Pull up
//
//
//	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))
//	{
//		get_data_status = false; 					// Error. Sensor not response
//	}
//	else
//	{
//		get_data_status = true;
//	}
////	if(GPIO->IDR & GPIO_IDR_ID1)		// Sensor must pull down
////	{
////		get_data_status = false; 					// Error. Sensor not response
////	}
////	else
////	{
////		get_data_status = true;
////	}
//
//	DelayMicro(80);
//	if(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)))
//	{
//		get_data_status = false; 					// Error. Sensor not response
//	}
//	else
//	{
//		get_data_status = true;
//	}
////	if(!(GPIOC->IDR & GPIO_IDR_ID1))  	// Sensor must pull up
////	{
////		get_data_status = false; 					// Error. Sensor not response
////	}
////	else
////	{
////		get_data_status = true;
////	}
//	DelayMicro(80);
//
//	if(get_data_status == true)
//	{
//		for(j = 0; j <5; j++)							// Reading 5 bytes
//		{
//			data[4-j] = 0;
//			for(i = 0; i < 8; i++)						// Reading 8 bits
//			{
//				//while(!(GPIOC->IDR & GPIO_IDR_ID1));	// While signal is "0"
//				while(!(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)));    // While signal is "0"
//				DelayMicro(30);
//				//if(GPIOC->IDR & GPIO_IDR_ID1)			// If signal is high when wrute "1" in buffer (data[])
//				if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))   // If signal is high when wrute "1" in buffer (data[])
//				{
//					data[4-j] |= (1 << (7 - i));        // Shift received bite
//				}
//				// while(GPIOC->IDR & GPIO_IDR_ID1);		// Wait end of "1" signal
//				while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12));		// Wait end of "1" signal
//			}
//			get_data_status = true;										// Data was been written okay
//		}
//
//		temper = (float)((*(uint16_t*)(data+1)) & 0x3FFF) /10;
//		if((*(uint16_t*)(data+1)) & 0x8000) temper  *= -1.0;
//
////		i2c_device.AM2302_temperature = temper;
//
//		hum = (float)(*(int16_t*)(data+3)) / 10;
////		i2c_device.AM2302_humidity = hum;
////
////		i2c_device.AM2302_ready_status = true;
//	}
//	else
//	{
//		int g = 999;
////		i2c_device.AM2302_ready_status = false;
//	}
//
//	//HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
//}
