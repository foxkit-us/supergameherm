#include "config.h"	// macros

#include "ctl_unit.h"	// signal_interrupt, INT_TIMER
#include "print.h"	// error
#include "sgherm.h"	// emu_state


void timer_tick(emu_state *restrict state, int count)
{
	int i, div_clk = state->timer.div_clk;

	// DIV increases even if the timer is disabled
	div_clk += count;
	state->timer.div += (div_clk >> 8);
	state->timer.div_clk = (div_clk & 0xFF);

	// but nothing else does.
	if(!state->timer.enabled)
	{
		return;
	}

	for(i = 0; i < count; i++)
	{
		if(++state->timer.curr_clk % state->timer.ticks_per_tima == 0)
		{
			if(++state->timer.tima == 0)	// overflow!
			{
				state->timer.rounds++;
				signal_interrupt(state, INT_TIMER);
			}
		}
	}
}
