#ifndef __SERIO_H_
#define __SERIO_H_

#include "config.h"	// macros

#include <stdint.h>	// uint[XX]_t


uint8_t serial_read(emulator_state *restrict, uint16_t);
void serial_write(emulator_state *restrict, uint16_t, uint8_t);
void serial_tick(emulator_state *restrict state);

#endif /*!__SERIO_H_*/
