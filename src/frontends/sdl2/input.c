#include "sgherm.h"	// emu_state
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend
#include "frontends/sdl2/sdl_inc.h"	// SDL


bool sdl2_init_input(emu_state *state UNUSED)
{
	info(state, "Initalising the SDL input frontend!");

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
	{
		error(state, "Failed to initalise input frontend: %s", SDL_GetError());
		return false;
	}

	return true;
}

void sdl2_finish_input(emu_state *state UNUSED)
{
	info(state, "SDL input frontend is outta here!");

	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void sdl2_get_key(emu_state *state UNUSED, frontend_input_return *ret)
{
	SDL_Event ev;
	if(SDL_PollEvent(&ev) == 0)
	{
		ret->key = 0;
		return;
	}

	if(ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP)
	{
		ret->press = (ev.type == SDL_KEYDOWN ? true : false);

		switch(ev.key.keysym.sym)
		{
		case SDLK_UP:
			ret->key = INPUT_UP;
			break;

		case SDLK_DOWN:
			ret->key = INPUT_DOWN;
			break;

		case SDLK_LEFT:
			ret->key = INPUT_LEFT;
			break;

		case SDLK_RIGHT:
			ret->key = INPUT_RIGHT;
			break;

		case SDLK_a:
		case SDLK_z:
			ret->key = INPUT_A;
			break;

		case SDLK_s:
		case SDLK_x:
			ret->key = INPUT_B;
			break;

		case SDLK_BACKSPACE:
			ret->key = INPUT_SELECT;
			break;

		case SDLK_RETURN:
			ret->key = INPUT_START;
			break;

		case SDLK_ESCAPE:
			do_exit = true;

		case SDLK_SPACE:
			if(ev.type == SDL_KEYDOWN)
			{
				state->debug.instr_dump ^= 1;
			}

		default:
			ret->key = 0;
			break;
		}
	}
	else if(ev.type == SDL_QUIT)
	{
		ret->key = 0;
		do_exit = true;
	}
	else
	{
		ret->key = 0;
	}
}


const frontend_input sdl2_frontend_input =
{
	&sdl2_init_input,
	&sdl2_finish_input,
	&sdl2_get_key,
	NULL,
};
