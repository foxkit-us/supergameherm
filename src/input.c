#include "config.h"	// macros, uint[XX]_t

#include <assert.h>	// assert

#include "ctl_unit.h"	// signal_interrupt
#include "input.h"	// input_key, defines
#include "print.h"	// error
#include "sgherm.h"	// emu_state
#include "util.h"	// unused


uint8_t joypad_read(emu_state *restrict state, uint16_t reg unused)
{
	uint8_t val = (state->input.col << 4) | (state->input.row);

	assert(reg == 0xFF00);

	return val;
}

void joypad_write(emu_state *restrict state, uint16_t reg unused, uint8_t data)
{
	assert(reg == 0xFF00);

	state->input.col = data >> 4;
	state->input.row = data & 0xc0;

	return;
}

void joypad_signal(emu_state *restrict state, input_key key, bool down)
{
	uint8_t col_sel = (uint8_t)(key >> 4);
	uint8_t row_sel = (uint8_t)(key & 0xC0);

	// TODO fake propagation delay and maybe switch bounce

	if(!(state->input.col & col_sel))
	{
		// Unsolicited key event
		debug("Unsolicited key event");
		return;
	}

	if(down)
	{
		state->input.row &= ~row_sel;
		debug("Pressing key %02X", key);
	}
	else
	{
		// Bring system back up
		state->stop = false;

		signal_interrupt(state, INT_JOYPAD);

		state->input.row |= row_sel;

		debug("Depressing key %02X", key);
	}
}
