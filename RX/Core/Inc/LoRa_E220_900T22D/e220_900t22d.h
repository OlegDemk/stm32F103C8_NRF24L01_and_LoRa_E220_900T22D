/*
 * e220_900t22d.h
 *
 *  Created on: Jul 17, 2021
 *      Author: odemki
 */

#ifndef INC_LORA_E220_900T22D_E220_900T22D_H_
#define INC_LORA_E220_900T22D_E220_900T22D_H_

bool init_lora_RX(void);
bool init_lora_TX(void);

void LoRa_RX(bool flag);
void LoRa_TX(bool flag);

#endif /* INC_LORA_E220_900T22D_E220_900T22D_H_ */
