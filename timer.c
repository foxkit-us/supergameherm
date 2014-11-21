#include "config.h"	// macros

#include "print.h"	// error
#include "sgherm.h"	// emu_state

uint8_t timer_read(emu_state *restrict state, uint16_t reg)
{
	switch(reg)
	{
	/*
	 * DIV - divider register
	 * I am aware this /technically/ a CPU thing, not a timer chip
	 * thing, but it logically fits in with timers and it's simpler
	 * this way.  So it's here.
	 */
	case 0xFF04:
		return state->timer_state.div;
	/*
	 * TIMA - stepper (inc'd once every timer tick)
	 */
	case 0xFF05:
		return state->timer_state.tima;
	/*
	 * TMA - how many times has TIMA overflowed?
	 */
	case 0xFF06:
		return state->timer_state.rounds;
	/*
	 * TAC - timer control
	 */
	case 0xFF07:
	{
		uint8_t res = 0;

		if(state->timer_state.enabled) res |= 0x04;

		switch(state->timer_state.ticks_per_tima)
		{
		case 1024:
			break;
			/* fall through on purpose; it's fun! */
		case 128:
			res++;		/* 16.384KHz = 3 */
		case 64:
			res++;		/* 65.536KHz = 2 */
		case 16:
			res++;		/* 262.14KHz = 1 */
		}

		return res;
	}
	default:
		error("timer: unrecognised register %04X (R)", reg);
		return -1;
	}
}

void timer_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	switch(reg)
	{
	/*
	 * DIV - any write to this immediately resets to 0.
	 */
	case 0xFF04:
		/* nope, data is ignored.  reset to 0. */
		state->timer_state.div = 0;
		return;
	/*
	 * TIMA - XXX FIXME does any game do this?
	 * should it reset to 0 ala DIV or does it keep data?
	 */
	case 0xFF05:
		state->timer_state.tima = data;
		return;
	/*
	 * TMA - I guess writing to this maybe makes sense
	 * maybe...
	 */
	case 0xFF06:
		state->timer_state.rounds = data;
		return;
	/*
	 * TAC - timer control
	 */
	case 0xFF07:
	{
		static const uint16_t ticks[4] = { 1024, 16, 64, 128 };

		state->timer_state.enabled = (data & 0x04);
		state->timer_state.ticks_per_tima = ticks[(data & 3)];

		return;
	}
	default:
		error("timer: unrecognised register %04X (W)", reg);
	}
}

void timer_tick(emu_state *restrict state)
{
	/* DIV increases even if the timer is disabled */
	if(++state->timer_state.curr_clk % 128 == 0)
	{
		state->timer_state.div++;
	}

	/* but nothing else does. */
	if(!state->timer_state.enabled)
	{
		return;
	}

	if(state->timer_state.curr_clk % state->timer_state.ticks_per_tima == 0)
	{
		if(++state->timer_state.tima == 0)	/* overflow! */
		{
			state->timer_state.rounds++;
			/* TODO raise an interrupt */
		}
	}
}
