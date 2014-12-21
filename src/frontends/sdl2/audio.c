#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend

// SDL 2 whines. *sigh*
#ifdef HAVE_STDINT_H
#	undef HAVE_STDINT_H
#endif
#ifdef HAVE_CLOCK_GETTIME
#	undef HAVE_CLOCK_GETTIME
#endif
#ifdef HAVE_NANOSLEEP
#	undef HAVE_NANOSLEEP
#endif

#include <SDL.h>	// SDL
#ifdef HAVE_STDBOOL_H
#	include <stdbool.h>	// bool
#endif

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
