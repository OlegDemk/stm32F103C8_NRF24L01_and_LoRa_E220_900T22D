/*
 * nrf24l01.c
 *
 *  Created on: Jul 3, 2021
 *      Author: odemki
 */


#include <NRF24L01/nrf24l01.h>
#include "OLED/ssd1306.h"
#include "OLED/fonts.h"
#include "OLED/oled_ssd1306.h"

#include <stdbool.h>

extern SPI_HandleTypeDef hspi1;

#define TX_ADR_WIDTH 3
#define TX_PLOAD_WIDTH 10
uint8_t TX_ADDRESS_0[TX_ADR_WIDTH] = {0xb3,0xb4,0x01};	// TX Pipe 0
uint8_t TX_ADDRESS_1[TX_ADR_WIDTH] = {0xb7,0xb5,0xa1};  // TX Pipe 1
uint8_t RX_BUF[TX_PLOAD_WIDTH] = {0};

volatile uint8_t rx_flag = 0;			// Flag for show that new data is received
uint8_t config_array[15] = {0};			// Array for save config register there
uint8_t buf1[20]={0};					// RX buffer
uint8_t pipe = 0;						// Number of pipes
uint8_t ErrCnt_Fl = 0; 					// Error counter (Can count only to 15)

void read_config_registers(void);

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
uint8_t NRF24_ReadReg(uint8_t addr)
{
  uint8_t dt=0, cmd;
  CS_ON;
  HAL_SPI_TransmitReceive(&hspi1,&addr,&dt,1,1000);
  if (addr!=STATUS_NRF)
  {
	  cmd=0xFF;
	  HAL_SPI_TransmitReceive(&hspi1,&cmd,&dt,1,1000);
  }
  CS_OFF;
  return dt;
}
//----------------------------------------------------------------------------------------
void NRF24_WriteReg(uint8_t addr, uint8_t dt)
{
  addr |= W_REGISTER;								// Add write bit
  CS_ON;
  HAL_SPI_Transmit(&hspi1,&addr,1,1000);			// Send address in bus
  HAL_SPI_Transmit(&hspi1,&dt,1,1000);				// Send data in bus
  CS_OFF;
}
//----------------------------------------------------------------------------------------
void NRF24_ToggleFeatures(void)
{
  uint8_t dt[1] = {ACTIVATE};
  CS_ON;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  DelayMicro(1);
  dt[0] = 0x73;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  CS_OFF;
}
//----------------------------------------------------------------------------------------
void NRF24_Read_Buf(uint8_t addr,uint8_t *pBuf,uint8_t bytes)
{
  CS_ON;
  HAL_SPI_Transmit(&hspi1,&addr,1,1000);			// Send address in bus
  HAL_SPI_Receive(&hspi1,pBuf,bytes,1000);			// Save data in buffer
  CS_OFF;
}
//----------------------------------------------------------------------------------------
void NRF24_Write_Buf(uint8_t addr,uint8_t *pBuf,uint8_t bytes)
{
  addr |= W_REGISTER;								//Add write bit
  CS_ON;
  HAL_SPI_Transmit(&hspi1,&addr,1,1000);			// Send address in bus
  DelayMicro(1);
  HAL_SPI_Transmit(&hspi1,pBuf,bytes,1000);			// Send data in buffer
  CS_OFF;
}
//----------------------------------------------------------------------------------------
void NRF24_FlushRX(void)
{
  uint8_t dt[1] = {FLUSH_RX};
  CS_ON;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  DelayMicro(1);
  CS_OFF;
}
//----------------------------------------------------------------------------------------
void NRF24_FlushTX(void)
{
  uint8_t dt[1] = {FLUSH_TX};
  CS_ON;
  HAL_SPI_Transmit(&hspi1,dt,1,1000);
  DelayMicro(1);
  CS_OFF;
}
//----------------------------------------------------------------------------------------
void NRF24L01_RX_Mode(void)
{
  uint8_t regval=0x00;
  regval = NRF24_ReadReg(CONFIG);

  regval |= (1<<PWR_UP) | (1<<PRIM_RX);    	// Power up module. Write PWR_UP and PRIM_RX bits

  NRF24_WriteReg(CONFIG,regval);

  NRF24_WriteReg(CONFIG, 0x33);     		//  IRQ WORK  ~ 130 us

  CE_SET;
  DelayMicro(150); // Delay 130 us
  // Flush buffers
  NRF24_FlushRX();
  NRF24_FlushTX();
}
//----------------------------------------------------------------------------------------
// Function waite Low IRQ signal from NRF module
bool NRF24L01_Receive(void)
{
	if(rx_flag == 1)
	{
		// Detect what's pipes data come from
		if(pipe == 0)
		{
			ssd1306_SetCursor(0, 16);
			char test_main[15] = {0};
			strcpy(test_main, "P0 data:");
			strcat(test_main, RX_BUF);
			ssd1306_WriteString(test_main,  Font_7x10, White);
		}
		if(pipe == 1)
		{
			ssd1306_SetCursor(0, 26);
			char test_main[15] = {0};
			strcpy(test_main, "P1 data:");
			strcat(test_main, RX_BUF);
			ssd1306_WriteString(test_main,  Font_7x10, White);
		}

		// Print RX data on OLED
		ssd1306_UpdateScreen();

		rx_flag = 0;
	}
}
//----------------------------------------------------------------------------------------
void NRF24_ini(void)                  // RECEIVE
{
	CE_RESET;
	DelayMicro(5000);

	NRF24_WriteReg(CONFIG, 0x0a); // Set PWR_UP bit, enable CRC(1 byte) &Prim_RX:0 (Transmitter) 0b 0000 1010

	DelayMicro(5000);

	NRF24_WriteReg(EN_AA, 0x03); 			// Enable Pipe0 and Pipe1
	NRF24_WriteReg(EN_RXADDR, 0x03); 		// Enable Pipe0 and Pipe1
	NRF24_WriteReg(SETUP_AW, 0x01); 		// Setup address width=3 bytes
	NRF24_WriteReg(SETUP_RETR, 0x5F); 		// 1500us, 15 retrans
	NRF24_ToggleFeatures();					// Send activated command
	NRF24_WriteReg(FEATURE, 0);				// Turn off all FEATURE register
	NRF24_WriteReg(DYNPD, 0); 				// Turn off all payload length data pipe
	NRF24_WriteReg(STATUS_NRF, 0x70); 		// Reset flags for IRQ
	NRF24_WriteReg(RF_CH, 76); 				// Frequency = 2476 MHz  // was 76
	NRF24_WriteReg(RF_SETUP, 0x26); 	    // TX_PWR:0dBm, Datarate:250kbps

	NRF24_Write_Buf(TX_ADDR, TX_ADDRESS_0, TX_ADR_WIDTH);		//	set up Transmit address. Used for a PTX device only.

	NRF24_Write_Buf(RX_ADDR_P0, TX_ADDRESS_0, TX_ADR_WIDTH);    // Set up pipe 0 address
	NRF24_Write_Buf(RX_ADDR_P1, TX_ADDRESS_1, TX_ADR_WIDTH);    // Set up pipe 1 address

	NRF24_WriteReg(RX_PW_P0, TX_PLOAD_WIDTH);				 //Number of bytes in RX pipe 0
	NRF24_WriteReg(RX_PW_P1, TX_PLOAD_WIDTH);				 //Number of bytes in RX pipe 1

	NRF24L01_RX_Mode();
}
//----------------------------------------------------------------------------------------
// Read config data from nrf registers
void read_config_registers(void)
{
	HAL_Delay(100);

	config_array[0] = NRF24_ReadReg(CONFIG);			// 0x0B
	config_array[1] = NRF24_ReadReg(EN_AA);			    // 0x01
	config_array[2] = NRF24_ReadReg(EN_RXADDR); 		// 0x01
	config_array[3] = NRF24_ReadReg(STATUS_NRF);		// 0x0E
	config_array[4] = NRF24_ReadReg(RF_SETUP);		    // 0x06

	NRF24_Read_Buf(TX_ADDR,buf1,3);
	NRF24_Read_Buf(RX_ADDR_P0,buf1,3);
}
//----------------------------------------------------------------------------------------
void nrf_communication_test(void)
{
	NRF24L01_Receive();
}
//----------------------------------------------------------------------------------------
void IRQ_Callback(void)
{
	uint8_t status=0x01;
	uint16_t dt=0;

	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

	DelayMicro(10);

	status = NRF24_ReadReg(STATUS_NRF);
	if(status & 0x40)									    //	Flag: Data ready in FIFO  (Check RX_DR flag)
	{
		pipe = (status>>1) & 0x07;
		NRF24_Read_Buf(RD_RX_PLOAD,RX_BUF,TX_PLOAD_WIDTH);
		NRF24_WriteReg(STATUS_NRF, 0x40);					// For turn down interrupt in nrf module
		rx_flag = 1;
	}
}
//----------------------------------------------------------------------------------------

