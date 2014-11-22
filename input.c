#include "config.h"	// macros
#include <assert.h>	// assert
#include <stdint.h>	// uint[XX]_t

#include "ctl_unit.h"	// signal_interrupt
#include "print.h"	// error
#include "input.h"	// input_key, defines
#include "sgherm.h"	// emu_state


uint8_t joypad_read(emu_state *restrict state, uint16_t reg)
{
	assert(reg == 0xFF00);
	return state->memory[reg];
}

void joypad_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	// TODO fake propagation delay and maybe switch bounce
	state->memory[reg] = data;
	return;
}

void joypad_signal(emu_state *restrict state, input_key key, bool down)
{
	uint8_t key_mask = mem_read8(state, 0xFF00);
	uint8_t key_col = key_mask & (RAW_INPUT_P14 | RAW_INPUT_P15);

	if(!(key & key_col))
	{
		// Unsolicited key event
		return;
	}

	uint8_t report_key = key & ~key_col;

	if(down)
	{
		mem_write8(state, 0xFF00, ~report_key);
	}
	else
	{
		signal_interrupt(state, INT_JOYPAD);
		mem_write8(state, 0xFF00, report_key);
	}
}
