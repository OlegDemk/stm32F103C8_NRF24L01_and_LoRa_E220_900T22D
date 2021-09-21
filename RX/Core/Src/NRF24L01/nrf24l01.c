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

// Flags for turn off/on generate Callback from IRQ pin NRF module
// in TX mode program waiting on falling adge signal for
bool tx_mode = true;
bool rx_mode = false;
bool tx_or_rx_mode = true;


#define TX_ADR_WIDTH 3
#define TX_PLOAD_WIDTH 10
uint8_t TX_ADDRESS_0[TX_ADR_WIDTH] = {0xb3,0xb4,0x01};	// TX Pipe 0
uint8_t TX_ADDRESS_1[TX_ADR_WIDTH] = {0xb7,0xb5,0xa1};  // TX Pipe 1
uint8_t RX_BUF[TX_PLOAD_WIDTH] = {0};

////////  RX part of variables
volatile uint8_t rx_flag = 0;			// Flag for show that new data is received
uint8_t config_array[15] = {0};			// Array for save config register there
uint8_t buf1[20]={0};					// RX buffer for read address
uint8_t pipe = 0;						// Number of pipes
uint8_t ErrCnt_Fl = 0; 					// Error counter (Can count only to 15)

////////  TX part of variables
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xb3,0xb4,0x01};   // Address for pipe 0
uint32_t i=1,retr_cnt_full=0, cnt_lost=0;

bool read_config_registers(void);

// common functions for TX and RX modes
__STATIC_INLINE void DelayMicro(__IO uint32_t micros);
uint8_t NRF24_ReadReg(uint8_t addr);
void NRF24_WriteReg(uint8_t addr, uint8_t dt);
void NRF24_ToggleFeatures(void);
void NRF24_Read_Buf(uint8_t addr,uint8_t *pBuf,uint8_t bytes);
void NRF24_Write_Buf(uint8_t addr,uint8_t *pBuf,uint8_t bytes);
void NRF24_FlushRX(void);
void NRF24_FlushTX(void);
bool read_config_registers(void);

// Functions prototypes for RX
void nrf_RX(void);								// Main function RX
void NRF24L01_RX_Mode(void);
bool NRF24L01_Receive(void);
void NRF24_init_RX_mode(void);
void IRQ_Callback(void);

// Functions prototypes for TX
void nrf_TX(void);								// Main function TX
void NRF24L01_RX_Mode_for_TX_mode(void);
void NRF24_init_TX_mode(void);
void NRF24L01_TX_Mode(uint8_t *pBuf);
void NRF24_Transmit(uint8_t addr,uint8_t *pBuf,uint8_t bytes);
uint8_t NRF24L01_Send(uint8_t *pBuf);
void NRF24L01_Transmission(void);


// TODO
// fill in reset_nrf24l01() before init nrf module

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                RX PART
//----------------------------------------------------------------------------------------
// Function for test TX mode
void nrf_RX(void)				// Main function RX
{
	NRF24_init_RX_mode();
	while(1)
	{
		NRF24L01_Receive();
	}
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
			char str_rx_oled_buffer_pipe_0[15] = {0};
			strcpy(str_rx_oled_buffer_pipe_0, "P0 data:");
			strcat(str_rx_oled_buffer_pipe_0, RX_BUF);
			ssd1306_WriteString(str_rx_oled_buffer_pipe_0,  Font_7x10, White);
		}
		if(pipe == 1)
		{
			ssd1306_SetCursor(0, 26);
			char str_rx_oled_buffer_pipe_1[15] = {0};
			strcpy(str_rx_oled_buffer_pipe_1, "P1 data:");
			strcat(str_rx_oled_buffer_pipe_1, RX_BUF);
			ssd1306_WriteString(str_rx_oled_buffer_pipe_1,  Font_7x10, White);
		}
		// Print RX data on OLED
		ssd1306_UpdateScreen();
		rx_flag = 0;
	}
}
//----------------------------------------------------------------------------------------
void NRF24_init_RX_mode(void)                  // RECEIVE
{
	reset_nrf24l01();	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	tx_or_rx_mode = rx_mode;		// For block interrupt HAL_GPIO_EXTI_Callback

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
// Callback generate when stm32 get falling  signal from IRQ pin (NRF module show that it has data in buffer)
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                TX PART

//----------------------------------------------------------------------------------------
// Function for test TX mode
void nrf_TX(void)		// Main function TX
{
	NRF24_init_TX_mode();
	while(1)
	{
		NRF24L01_Transmission();
	}
}
//----------------------------------------------------------------------------------------
void NRF24L01_RX_Mode_for_TX_mode(void)
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
void NRF24_init_TX_mode(void)    // TRANSMITTER
{
	reset_nrf24l01();	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	tx_or_rx_mode = tx_mode;		// For block interrupt HAL_GPIO_EXTI_Callback

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
	NRF24_WriteReg(RF_SETUP, 0x26);  		// TX_PWR:0dBm, Datarate:250kbps

	NRF24_Write_Buf(TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);			// Write TX address

	NRF24_Write_Buf(RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);		// Set up pipe 0 address
	NRF24_WriteReg(RX_PW_P0, TX_PLOAD_WIDTH);				 	// Number of bytes in TX buffer

	NRF24L01_RX_Mode_for_TX_mode();

	read_config_registers();	// For debug
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

  NRF24L01_RX_Mode_for_TX_mode();

  return regval;
}
//----------------------------------------------------------------------------------------
void NRF24L01_Transmission(void)
{
	char ctr[5] = {0};
	char ctr_buf[5] = {0};

	uint8_t retr_cnt, dt = 0;

	int test_data = 0;

	while(1)
	{
		// Test transmit data

		uint8_t buf2[20]={0};
		sprintf(buf2, "%d", test_data);

		// Print transmit data
		uint8_t test[25] = {0};
		uint8_t test_i[10] = {0};

		ssd1306_SetCursor(0, 16);
		strcpy(test, "Data:");

		ssd1306_WriteString(test,  Font_7x10, White);

		dt = NRF24L01_Send(buf2);						// Transmit data

		strcat(test, buf2);
		ssd1306_WriteString(test,  Font_7x10, White);
		ssd1306_UpdateScreen();

		//dt = NRF24L01_Send(buf2);						// Transmit data

		retr_cnt = dt & 0xF;
		retr_cnt_full += retr_cnt;

		// Print transmit counter
		memset(test, 0, sizeof(test));
		memset(test_i, 0, sizeof(test_i));

		ssd1306_SetCursor(0, 26);
		strcpy(test, "Conut trans:");
		// number in string
		itoa(i, test_i, 10);
		strcat(test, test_i);
		ssd1306_WriteString(test,  Font_7x10, White);

		// Print retransmeet counter
		memset(test, 0, sizeof(test));
		memset(test_i, 0, sizeof(test_i));

		ssd1306_SetCursor(0, 36);
		strcpy(test, "Retransm:");
		itoa(retr_cnt_full, test_i, 10);
		strcat(test, test_i);
		ssd1306_WriteString(test,  Font_7x10, White);
		ssd1306_UpdateScreen();

		// Print lost pacets
		memset(test, 0, sizeof(test));
		memset(test_i, 0, sizeof(test_i));

		cnt_lost = dt >> 4;

		ssd1306_SetCursor(0, 46);
		strcpy(test, "Lost:");
		itoa(cnt_lost, test_i, 10);
		strcat(test, test_i);
		ssd1306_WriteString(test,  Font_7x10, White);
		ssd1306_UpdateScreen();

		test_data++;
		i++;

		HAL_Delay(1000);
	}
}
//----------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    						Common functions for work with NRF module
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
// Read config data from nrf registers
bool read_config_registers(void)
{
	HAL_Delay(100);

	config_array[0] = NRF24_ReadReg(CONFIG);			// 0x0B
	config_array[1] = NRF24_ReadReg(EN_AA);			    // 0x01
	config_array[2] = NRF24_ReadReg(EN_RXADDR); 		// 0x01
	config_array[3] = NRF24_ReadReg(STATUS_NRF);		// 0x0E
	config_array[4] = NRF24_ReadReg(RF_SETUP);		    // 0x06

	NRF24_Read_Buf(TX_ADDR,buf1,3);
	NRF24_Read_Buf(RX_ADDR_P0,buf1,3);

	if(config_array[0] == 0)		// Data from nrf module was read
	{
		return false;
	}
	else
	{
		return true;
	}
}
//----------------------------------------------------------------------------------------
void reset_nrf24l01(void)   // reconfigure module
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
	NRF24_WriteReg(STATUS_NRF, 0x00); 		// Reset flags for IRQ   // WAS NRF24_WriteReg(STATUS_NRF, 0x70); 		// Reset flags for IRQ
	NRF24_WriteReg(RF_CH, 76); 				// Frequency = 2476 MHz
	NRF24_WriteReg(RF_SETUP, 0x26);  		// TX_PWR:0dBm, Datarate:250kbps

	uint8_t TX_ADDRESS_RESET[TX_ADR_WIDTH] = {0x00,0x00,0x00};   // Address for pipe 0
	NRF24_Write_Buf(TX_ADDR, TX_ADDRESS_RESET, TX_ADR_WIDTH);			// Write TX address

	NRF24_Write_Buf(RX_ADDR_P0, TX_ADDRESS_RESET, TX_ADR_WIDTH);		// Set up pipe 0 address
	NRF24_WriteReg(RX_PW_P0, TX_PLOAD_WIDTH);				 	// Number of bytes in TX buffer

	NRF24L01_RX_Mode_for_TX_mode();
}
//----------------------------------------------------------------------------------------
























