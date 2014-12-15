#include "config.h"	// macros, uint[XX]_t

#include <assert.h>	// assert

#include "ctl_unit.h"	// signal_interrupt
#include "input.h"	// input_key, defines
#include "print.h"	// error
#include "sgherm.h"	// emu_state
#include "util.h"	// UNUSED


uint8_t joypad_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	uint8_t val = (state->input.col << 4) | (state->input.row);

	assert(reg == 0xFF00);

	return val;
}

void joypad_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	assert(reg == 0xFF00);

	state->input.col = data >> 4;
	if(state->input.col)
	{
		state->input.row = 0xf & ~(state->input.key_row);
	}
	else
	{
		state->input.row = 0;
	}

	return;
}

void joypad_signal(emu_state *restrict state, input_key key, bool down)
{
	if(down)
	{
		state->stop = false;

		state->input.key_col |= key >> 4;
		state->input.key_row |= key & 0xf;
	}
	else
	{
		state->input.key_col &= ~(key >> 4);
		state->input.key_row &= ~(key & 0xf);
	}

	// TODO fake propagation delay and maybe switch bounce

	state->input.row = 0xf & ~(state->input.key_row);
}
