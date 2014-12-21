#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend
#include "frontends/sdl2/sdl_inc.h"     // SDL

#include <stdlib.h>	// calloc

int sdl2_event_loop(emu_state *state)
{
	debug(state, "Executing sdl event loop");

	SDL_Init(0);

	do
	{
		uint8_t mode = state->lcdc.stat.params.mode_flag;
		uint_fast16_t clock = state->lcdc.curr_clk;
		frontend_input_return ret;

		step_emulator(state);

		if(unlikely(mode == 1 && clock == 1 && state->input.col))
		{
			GET_KEY(state, &ret);
			if(ret.key != INPUT_NONE)
			{
				joypad_signal(state, ret.key, ret.press);
			}
		}
	} while(!do_exit);

	SDL_Quit();

	return 0;
}
