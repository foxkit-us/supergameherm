#include "config.h"	// macros

#include "ctl_unit.h"	// signal_interrupt, INT_TIMER
#include "print.h"	// error
#include "sgherm.h"	// emu_state


// XXX don't loop, compute this dynamically
void timer_tick(emu_state *restrict state, int count)
{
	int i;

	// DIV increases even if the timer is disabled
	for(i = 0; i < count; i++)
	{
		if(++state->timer.div_clk == 0)
		{
			state->timer.div++;
		}
	}

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
