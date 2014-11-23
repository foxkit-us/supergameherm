#ifndef __SERIO_H_
#define __SERIO_H_

#include "config.h"	// macros

#include <stdint.h>	// uint[XX]_t

typedef struct _ser_state
{
	uint16_t curr_clk;		/*! ticks passed */
	uint8_t in, out;		/*! in / out values */
	int8_t cur_bit;			/*! the current bit */
	bool enabled;			/*! transfer active */
	bool use_internal;		/*! clock source */
} ser;


typedef struct _emu_state emu_state;

uint8_t serial_read(emu_state *restrict, uint16_t);
void serial_write(emu_state *restrict, uint16_t, uint8_t);
void serial_tick(emu_state *restrict state);

#endif /*!__SERIO_H_*/
