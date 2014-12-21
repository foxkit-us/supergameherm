#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend

#include <caca.h>	// libcaca
#include <stdbool.h>	// bool

int libcaca_event_loop(emu_state *state)
{
	debug(state, "Executing libcaca event loop");

	do
	{
		frontend_input_return ret;
		uint8_t mode = state->lcdc.stat.mode_flag;
		uint_fast16_t clock = state->lcdc.curr_clk;

		step_emulator(state);

		if(unlikely(mode == 1 && clock == 1 && state->input.col))
		{
			GET_KEY(state, &ret);
			if(ret.key > 0)
			{
				joypad_signal(state, ret.key, ret.press);
			}
		}
	} while(!do_exit);

	return 0;
}
