#ifndef __CTL_UNIT_H_
#define __CTL_UNIT_H_

#include <stdint.h>	// uint[XX]_t

#include "sgherm.h"	// emulator_state


void init_ctl(emulator_state *, char);
bool execute(emulator_state *);

uint8_t int_flag_read(emulator_state *, uint16_t);
void int_flag_write(emulator_state *, uint16_t, uint8_t);

#endif /*!__CTL_UNIT_H_*/
