#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend
#include "frontends/sdl2/sdl_inc.h"	// SDL


bool sdl2_init_audio(emu_state *state UNUSED)
{
	 info(state, "Initalising the SDL audio frontend");

	 if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
	 {
		 error(state, "Failed to initalise audio frontend: %s", SDL_GetError());
		 return false;
	 }

	 return true;
}

void sdl2_finish_audio(emu_state *state UNUSED)
{
	info(state, "SDL audio frontend finishing up");

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void sdl2_output_sample(emu_state *state UNUSED)
{
	// TODO
	return;
}


const frontend_audio sdl2_frontend_audio =
{
	&sdl2_init_audio,
	&sdl2_finish_audio,
	&sdl2_output_sample,
	NULL,
};
