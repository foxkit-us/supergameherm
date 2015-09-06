#include "config.h"	// macros, bool

#include "print.h"	// error
#include "sgherm.h"	// emu_state

/*!
 * @brief	Advance the serial controller one clock pulse.
 * @param	state	The emulator state the clock pulse is occurring on.
 * @result	The serial controller acts on the clock pulse.
 * TODO: update this comment
 */
void serial_tick(emu_state *restrict state, int count)
{
	uint16_t ticks;

#ifdef DEFENSIVE
	// we aren't active; we don't care
	if(!state->ser.enabled)
	{
		return;
	}
#endif

	for(; count > 0; count--)
	{
		state->ser.curr_clk++;
		// XXX TODO FIXME this does NOT support external clocks
		if(state->ser.use_internal)
		{
			ticks = 512;
		}
		else
		{
			ticks = 8;
		}

		if((state->ser.curr_clk % ticks) == 0)
		{
			// TODO put out a bit.
			state->ser.out <<= 1;

			// TODO take in a bit.
			state->ser.out |= 1;

			// sockets?  IPC?  something else?  all three?
			if(state->ser.cur_bit-- == -1)
			{
				state->ser.enabled = false;
				signal_interrupt(state, INT_SERIAL);
				state->ser.curr_clk = 0;
			}
		}
	}
}

