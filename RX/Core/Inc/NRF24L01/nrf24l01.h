/*
 * nrf24l01.h
 *
 *  Created on: Jul 3, 2021
 *      Author: odemki
 */

#ifndef INC_NRF24L01_NRF24L01_H_
#define INC_NRF24L01_NRF24L01_H_

#include "main.h"
#include <string.h>
#include <stdbool.h>

// PC4
#define CS_GPIO_PORT GPIOA
#define CS_PIN GPIO_PIN_4
#define CS_ON HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET)
#define CS_OFF HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET)

// PA3
#define CE_GPIO_PORT GPIOA
#define CE_PIN GPIO_PIN_3
#define CE_RESET HAL_GPIO_WritePin(CE_GPIO_PORT, CE_PIN, GPIO_PIN_RESET)
#define CE_SET HAL_GPIO_WritePin(CE_GPIO_PORT, CE_PIN, GPIO_PIN_SET)

// PC2
#define IRQ_GPIO_PORT GPIOA
#define IRQ_PIN GPIO_PIN_2   //IRQ_nrf_Pin
#define IRQ HAL_GPIO_ReadPin(IRQ_GPIO_PORT, IRQ_PIN)
#define LED_GPIO_PORT GPIOC

//------------------------------------------------
#define ACTIVATE 0x50 //
#define RD_RX_PLOAD 0x61 // Define RX payload register address
#define WR_TX_PLOAD 0xA0 // Define TX payload register address
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
//------------------------------------------------

#define CONFIG 0x00 //'Config' register address
#define EN_AA 0x01 //'Enable Auto Acknowledgment' register address
#define EN_RXADDR 0x02 //'Enabled RX addresses' register address
#define SETUP_AW 0x03 //'Setup address width' register address
#define SETUP_RETR 0x04 //'Setup Auto. Retrans' register address
#define RF_CH 0x05 //'RF channel' register address
#define RF_SETUP 0x06 //'RF setup' register address
#define STATUS_NRF 0x07 //'Status' register address
#define OBSERVE_TX 0x08 //'Transmit observe' register
#define RX_ADDR_P0 0x0A //'RX address pipe0' register address
#define RX_ADDR_P1 0x0B //'RX address pipe1' register address
#define TX_ADDR 0x10 //'TX address' register address
#define RX_PW_P0 0x11 //'RX payload width, pipe0' register address
#define RX_PW_P1 0x12 //'RX payload width, pipe1' register address
#define FIFO_STATUS 0x17 //'FIFO Status Register' register address
#define DYNPD 0x1C
#define FEATURE 0x1D

//------------------------------------------------

#define PRIM_RX 0x00 //RX/TX control (1: PRX, 0: PTX)
#define PWR_UP 0x01 //1: POWER UP, 0:POWER DOWN
#define RX_DR 0x40 //Data Ready RX FIFO interrupt
#define TX_DS 0x20 //Data Sent TX FIFO interrupt
#define MAX_RT 0x10 //Maximum number of TX retransmits interrupt

//------------------------------------------------
#define W_REGISTER 0x20 //запись в регистр
//------------------------------------------------



void NRF24_ini(void);
void nrf_communication_test(void);
void read_config_registers(void);
bool NRF24L01_Receive(void);
void IRQ_Callback(void);

#endif /* INC_NRF24L01_NRF24L01_H_ */
