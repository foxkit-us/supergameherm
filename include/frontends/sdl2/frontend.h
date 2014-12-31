#ifndef __FRONTEND_SDL2_FRONTEND_H__
#define __FRONTEND_SDL2_FRONTEND_H__

#include "frontends/sdl2/sdl_inc.h"


typedef struct sdl2_video_data_t
{
	SDL_Window *window;
	SDL_Renderer *render;
	SDL_Texture *texture;
} sdl2_video_data;


extern const frontend_audio sdl2_frontend_audio;
extern const frontend_video sdl2_frontend_video;
int sdl2_event_loop(emu_state *);

#endif
