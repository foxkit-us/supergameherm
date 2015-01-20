#include "config.h"	// bool, etc
#include "sgherm.h"	// emu_state
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontends/sdl2/frontend.h"	// frontend
#include "frontends/sdl2/sdl_inc.h"	// SDL


static inline input_key get_key(emu_state *state, SDL_Event *ev)
{
	switch(ev->key.keysym.sym)
	{
	case SDLK_UP:
		return INPUT_UP;

	case SDLK_DOWN:
		return INPUT_DOWN;

	case SDLK_LEFT:
		return INPUT_LEFT;

	case SDLK_RIGHT:
		return INPUT_RIGHT;

	case SDLK_a:
	case SDLK_z:
		return INPUT_A;

	case SDLK_s:
	case SDLK_x:
		return INPUT_B;

	case SDLK_BACKSPACE:
		return INPUT_SELECT;

	case SDLK_RETURN:
		return INPUT_START;

	case SDLK_ESCAPE:
		do_exit = true;
		return 0;

	case SDLK_SPACE:
		if(ev->type == SDL_KEYDOWN)
		{
			state->debug.instr_dump ^= 1;
		}

	default:
		return 0;
	}
}

int sdl2_event_loop(emu_state *state)
{
	debug(state, "Executing sdl event loop");

	if(SDL_Init(SDL_INIT_EVENTS))
	{
		error(state, "Failed to initalise input frontend: %s", SDL_GetError());
		return -1;
	}

	do
	{
		const uint8_t mode = LCDC_STAT_MODE_FLAG(state);
		const uint_fast16_t clock = state->lcdc.curr_clk;
		SDL_Event ev;

		step_emulator(state);

		if(unlikely(mode != 1 && clock != 0))
		{
			continue;
		}

		// Exhaust events
		while(SDL_PollEvent(&ev))
		{
			if(ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP)
			{
				bool pressed = (ev.type == SDL_KEYDOWN);
				input_key key = get_key(state, &ev);

				if(!key)
				{
					continue;
				}

				joypad_signal(state, key, pressed);
			}
			else if(unlikely(ev.type == SDL_WINDOWEVENT_RESIZED))
			{
				sdl2_video_data *video = state->front.video.data;
				SDL_RenderClear(video->render);
				BLIT_CANVAS(state);
			}
			else if(unlikely(ev.type == SDL_QUIT))
			{
				do_exit = true;
			}
		}
	} while(!do_exit);

	SDL_Quit();

	return 0;
}
