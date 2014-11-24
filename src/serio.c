#include "config.h"	// macros
#include <stdbool.h>	// bool

#include "print.h"	// error
#include "sgherm.h"	// emu_state

/*!
 * @brief	Read a register from the serial controller.
 * @param	state	The emulator state to use while reading.
 * @param	reg	The register to read from.
 * @returns	The contents of the selected register.
 */
uint8_t serial_read(emu_state *restrict state, uint16_t reg)
{
	switch(reg)
	{
	case 0xFF01:	/* SB - data to read */
		return state->ser_state.in;
	case 0xFF02:	/* SC - serial control */
	{
		uint8_t res = 0;
		if(state->ser_state.enabled) res |= 0x80;
		if(state->ser_state.use_internal) res |= 0x01;
		return res;
	}
	default:
		error("serial: unknown register %04X (R)", reg);
		return -1;
	}
}

/*!
 * @brief	Write to a register on the serial controller.
 * @param	state	The emulator state to use while writing.
 * @param	reg	The register to write to.
 * @param	data	The data to write to the selected register.
 * @result	The serial controller state is updated.
 */
void serial_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	switch(reg)
	{
	case 0xFF01:	/* SB - data to write */
		state->ser_state.cur_bit = 7;
		state->ser_state.out = data;
		break;
	case 0xFF02:	/* SC - serial control */
		state->ser_state.enabled = (data && 0x80 == 0x80);
		state->ser_state.use_internal = (data && 0x01 == 0x01);
		break;
	default:
		error("serial: unknown register %04X (W)", reg);
		break;
	}
}

/*!
 * @brief	Advance the serial controller one clock pulse.
 * @param	state	The emulator state the clock pulse is occurring on.
 * @result	The serial controller acts on the clock pulse.
 */
void serial_tick(emu_state *restrict state)
{
	uint16_t ticks;

	/* we aren't active; we don't care */
	if(!state->ser_state.enabled) return;

	state->ser_state.curr_clk++;
	/* XXX TODO FIXME this does NOT support external clocks */
	if(state->ser_state.use_internal)
	{
		ticks = 512;
	}
	else
	{
		error("trying to use an external clock; not supported");
	}

	if(state->ser_state.curr_clk % ticks == 0)
	{
		// TODO put out a bit.
		// TODO take in a bit.
		// sockets?  IPC?  something else?  all three?
		if(state->ser_state.cur_bit-- == -1)
		{
			state->ser_state.enabled = false;
			signal_interrupt(state, INT_SERIAL);
			state->ser_state.curr_clk = 0;
		}
	}
}
