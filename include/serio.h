#ifndef __SERIO_H_
#define __SERIO_H_

#include "config.h"	// macros,  uint[XX]_t
#include "typedefs.h"	// typedefs


struct ser_state_t
{
	uint16_t curr_clk;		//! ticks passed
	uint8_t in, out;		//! in / out values
	int8_t cur_bit;			//! the current bit
	bool enabled;			//! transfer active
	bool use_internal;		//! clock source
};


uint8_t serial_read(emu_state *restrict, uint16_t);
void serial_write(emu_state *restrict, uint16_t, uint8_t);
void serial_tick(emu_state *restrict state, int count);

#endif /*!__SERIO_H_*/
