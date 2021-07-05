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

extern SPI_HandleTypeDef hspi1;

#define TX_ADR_WIDTH 3
#define TX_PLOAD_WIDTH 5
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xAA,0xBB,0x01};
uint8_t RX_BUF[TX_PLOAD_WIDTH] = {0};

char str1[20]={0};
uint8_t buf1[20]={0};

uint8_t config_array[15] = {0};		// For save registers

void read_config_registers_nrf(void);

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
  regval |= (1<<PWR_UP)|(1<<PRIM_RX);	 // Power up module. Write PWR_UP и PRIM_RX bits
  NRF24_WriteReg(CONFIG,regval);
  CE_SET;
  DelayMicro(150);						 // Delay 130 us
  // Flush buffers
  NRF24_FlushRX();
  NRF24_FlushTX();
}
//----------------------------------------------------------------------------------------
void NRF24_ini(void)    // TRANSMITTER
{
	CE_RESET;
	DelayMicro(5000);

	NRF24_WriteReg(CONFIG, 0x0a); 			// Set PWR_UP bit, enable CRC(1 byte) &Prim_RX:0 (Transmitter)

	DelayMicro(5000);

	NRF24_WriteReg(EN_AA, 0x01); 			// Enable Pipe0
	NRF24_WriteReg(EN_RXADDR, 0x01); 		// Enable Pipe0
	NRF24_WriteReg(SETUP_AW, 0x01); 		// Setup address width=3 bytes
	NRF24_WriteReg(SETUP_RETR, 0x5F); 		// 1500us, 15 retrans
	NRF24_ToggleFeatures();
	NRF24_WriteReg(FEATURE, 0);
	NRF24_WriteReg(DYNPD, 0);
	NRF24_WriteReg(STATUS_NRF, 0x70); 		// Reset flags for IRQ
	NRF24_WriteReg(RF_CH, 76); 				// Frequency = 2476 MHz
	NRF24_WriteReg(RF_SETUP, 0x06); 		//TX_PWR:0dBm, Datarate:1Mbps

	NRF24_Write_Buf(TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);			// Write TX address
	NRF24_Write_Buf(RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);		//
	NRF24_WriteReg(RX_PW_P0, TX_PLOAD_WIDTH);				 	//Number of bytes in RX

	NRF24L01_RX_Mode();

	read_config_registers_nrf();
}
//----------------------------------------------------------------------------------------
// Read config data from nrf registers
void read_config_registers_nrf(void)
{
	HAL_Delay(1);

	config_array[0] = NRF24_ReadReg(CONFIG);			// 0x0B
	config_array[1] = NRF24_ReadReg(EN_AA);			    // 0x01
	config_array[2] = NRF24_ReadReg(EN_RXADDR); 		// 0x01
	config_array[3] = NRF24_ReadReg(STATUS_NRF);		// 0x0E
	config_array[4] = NRF24_ReadReg(RF_SETUP);		    // 0x06

	NRF24_Read_Buf(TX_ADDR,buf1,3);
	NRF24_Read_Buf(RX_ADDR_P0,buf1,3);

}
//----------------------------------------------------------------------------------------
void NRF24L01_TX_Mode(uint8_t *pBuf)
{
  NRF24_Write_Buf(TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
  CE_RESET;
  // Flush buffers
  NRF24_FlushRX();
  NRF24_FlushTX();
}
//----------------------------------------------------------------------------------------
void NRF24_Transmit(uint8_t addr,uint8_t *pBuf,uint8_t bytes)
{
  CE_RESET;
  CS_ON;
  HAL_SPI_Transmit(&hspi1,&addr,1,1000);	//Send address in NRF module
  DelayMicro(1);
  HAL_SPI_Transmit(&hspi1,pBuf,bytes,1000); //Send buff in NRF module
  CS_OFF;
  CE_SET;
}
//----------------------------------------------------------------------------------------
uint8_t NRF24L01_Send(uint8_t *pBuf)
{
  uint8_t status=0x00, regval=0x00;
  NRF24L01_TX_Mode(pBuf);
  regval = NRF24_ReadReg(CONFIG);
  // If module in sleep mode, wake up it send PWR_UP and PRIM_RX bits in CONFIG
  regval |= (1<<PWR_UP);			// Set power up
  regval &= ~(1<<PRIM_RX);			// Set TX mode
  NRF24_WriteReg(CONFIG,regval);
  DelayMicro(150); 					// Delay more then 130 us
  NRF24_Transmit(WR_TX_PLOAD, pBuf, TX_PLOAD_WIDTH);	// Send data

  CE_SET;
  DelayMicro(15); 					//minimum 10us high pulse (Page 21)
  CE_RESET;

  // Waiting interrupt signal from IRQ
  while((GPIO_PinState)IRQ == GPIO_PIN_SET){}

  status = NRF24_ReadReg(STATUS_NRF);		// Read status sent data to RX
  if(status & TX_DS) 	     //TX_DS == 0x20   // When transmitted data was receive, and we take back ACK answer
  {
      NRF24_WriteReg(STATUS_NRF, 0x20);
  }
  else if(status & MAX_RT)   //MAX_RT == 0x10  // Retransmeet data flag
  {
    NRF24_WriteReg(STATUS_NRF, 0x10);
    NRF24_FlushTX();
  }

  regval = NRF24_ReadReg(OBSERVE_TX);   // Return Count lost packets and count transmitted packets

  // Switch on RX mode
  NRF24L01_RX_Mode();

  return regval;
}
//----------------------------------------------------------------------------------------
void nrf_communication_test(void)
{
	NRF24_ini();

	// Print config array config_array[0]  (Config registers)
	char ctr[5] = {0};
	char ctr_buf[5] = {0};

	uint8_t x = 0;
	for (int i = 0; i <=4; i++)
	{
		itoa(config_array[i], ctr, 16);
		strcat(ctr_buf, ctr);
		ssd1306_SetCursor(x, 16);
		ssd1306_WriteString(ctr_buf,  Font_7x10, White);
		ssd1306_UpdateScreen();
		x = x + 24;

		memset(ctr_buf, 0, sizeof(ctr_buf));
		memset(ctr, 0, sizeof(ctr));
	}

	uint8_t retr_cnt, dt;
	uint16_t i=1;
	uint16_t retr_cnt_full =0;

	int test_data = 0;

	while(1)
	{
		// Test transmit data
		sprintf(buf1, "%d", test_data);

		// Print transmit data
		uint8_t test[25] = {0};
		uint8_t test_i[10] = {0};

		ssd1306_SetCursor(0, 32);
		strcpy(test, "Data:");
		strcat(test, buf1);
		ssd1306_WriteString(test,  Font_7x10, White);
		ssd1306_UpdateScreen();

		dt = NRF24L01_Send(buf1);			// Transmit data

		retr_cnt = dt & 0xF;
		i++;
		retr_cnt_full += retr_cnt;

		// Print transmit counter
		memset(test, 0, sizeof(test));
		memset(test_i, 0, sizeof(test_i));

		ssd1306_SetCursor(0, 42);
		strcpy(test, "Conut trans:");
		// number in string
		itoa(i, test_i, 10);
		strcat(test, test_i);
		ssd1306_WriteString(test,  Font_7x10, White);
		ssd1306_UpdateScreen();

		// Print transmit error
		memset(test, 0, sizeof(test));
		memset(test_i, 0, sizeof(test_i));

		ssd1306_SetCursor(0, 52);
		strcpy(test, "Retransm:");
		itoa(retr_cnt_full, test_i, 10);
		strcat(test, test_i);
		ssd1306_WriteString(test,  Font_7x10, White);
		ssd1306_UpdateScreen();

		test_data++;

		HAL_Delay(250);
	}
}
//----------------------------------------------------------------------------------------



