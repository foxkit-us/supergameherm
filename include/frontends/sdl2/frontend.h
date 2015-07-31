#ifndef __FRONTEND_SDL2_FRONTEND_H__
#define __FRONTEND_SDL2_FRONTEND_H__

#include "frontends/sdl2/sdl_inc.h"

#if defined(HAVE_MACH_CLOCK_H)
#	undef bool
typedef _Bool bool;
#endif

typedef struct sdl2_video_data_t
{
	SDL_Window *window;
	SDL_Renderer *render;
	SDL_Texture *texture;
} sdl2_video_data;

typedef struct sdl2_audio_data_t
{
	int dummy;
} sdl2_audio_data;


extern const frontend_audio sdl2_frontend_audio;
extern const frontend_video sdl2_frontend_video;
int sdl2_event_loop(emu_state *);


#define SDL2_AUDIO &sdl2_frontend_audio
#define SDL2_VIDEO &sdl2_frontend_video
#define SDL2_LOOP sdl2_event_loop

#endif
