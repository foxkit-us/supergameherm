#include "config.h"	// Various macros

#include <stdint.h>	// uint[XX]_t

#include "print.h"
#include "sgherm.h"	// emu_state

uint8_t sound_read(emu_state *restrict state, uint16_t reg)
{
	return -1;
}

void sound_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
}

void sound_tick(emu_state *restrict state)
{
}