#include "config.h"	// macros, uint[XX]_t

#include <assert.h>	// assert

#include "ctl_unit.h"	// signal_interrupt
#include "input.h"	// input_key, defines
#include "print.h"	// error
#include "sgherm.h"	// emu_state
#include "util.h"	// UNUSED


static inline int key_to_index(input_key key)
{
	switch(key)
	{
	case INPUT_RIGHT:
		return 0;
	case INPUT_LEFT:
		return 1;
	case INPUT_UP:
		return 2;
	case INPUT_DOWN:
		return 3;
	case INPUT_A:
		return 4;
	case INPUT_B:
		return 5;
	case INPUT_SELECT:
		return 6;
	case INPUT_START:
		return 7;
	default:
		return -1;
	}
}

int key_scan(emu_state *restrict state)
{
	int val = 0xf;
	int i = 0;

	if(!(state->input.col))
	{
		return 0;
	}

	for(i = 0; i < 8; i++)
	{
		int input = state->input.pressed[i];
		int lower = input & 0xf, upper = input >> 4;

		if(upper & state->input.col)
		{
			continue;
		}

		val &= ~lower;
	}

	return val;
}

void joypad_signal(emu_state *restrict state, input_key key, bool down)
{
	int k = key_to_index(key);

	assert(k >= 0);

	if(down)
	{
		state->stop = false;
		state->input.pressed[k] = key;
	}
	else
	{
		state->input.pressed[k] = 0;
	}

	// TODO fake propagation delay and maybe switch bounce

	state->input.row = key_scan(state);
}
