/*
 * state_machine.h
 *
 *  Created on: Aug 18, 2021
 *      Author: odemki
 */

#ifndef INC_STATE_MACHINE_STATE_MACHINE_H_
#define INC_STATE_MACHINE_STATE_MACHINE_H_

typedef enum{
	ST_1,
	ST_2,
	ST_3,
	ST_4
}STATE_t;

void state_machine(void);

STATE_t state_get(void);
void state_set(STATE_t new_state);

#endif /* INC_STATE_MACHINE_STATE_MACHINE_H_ */
