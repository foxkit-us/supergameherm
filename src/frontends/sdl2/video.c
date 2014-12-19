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
#include <stdbool.h>	// bool
#include <stdlib.h>	// calloc


#define LEN 144
#define WID 160
#define BPP 32
#define PITCH sizeof(uint32_t) * WID

#define RED	0x000000ff
#define GREEN	0x0000ff00
#define BLUE	0x00ff0000
#define ALPHA	0xff000000


typedef struct sdl2_video_data_t
{
	SDL_Window *window;
	SDL_Renderer *render;
	SDL_Texture *texture;
} sdl2_video_data;


bool sdl2_init_video(emu_state *state)
{
	sdl2_video_data *video;

	info(state, "Initalising the SDL video frontend!");

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		error(state, "Failed to initalise video frontend during video init: %s", SDL_GetError());
		return false;
	}

	video = calloc(1, sizeof(sdl2_video_data));
	state->front.video.data = video;

	video->window = SDL_CreateWindow("SuperGameHerm",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			WID*2, LEN*2, SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN);
	if(!(video->window))
	{
		error(state, "Failed to initalise video frontend during window creation: %s", SDL_GetError());
		free(video);
		return false;
	}

	video->render = SDL_CreateRenderer(video->window, -1, 0);
	if(!(video->render))
	{
		error(state, "Failed to initalise video frontend during render creation: %s", SDL_GetError());
		SDL_DestroyWindow(video->window);
		free(video);
		return false;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	SDL_RenderSetLogicalSize(video->render, WID, LEN);

	video->texture = SDL_CreateTexture(video->render,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			WID, LEN);
	if(!(video->texture))
	{
		error(state, "Failed to initalise video frontend during texture creation: %s", SDL_GetError());
		SDL_DestroyRenderer(video->render);
		SDL_DestroyWindow(video->window);
		free(video);
		return false;
	}

	SDL_SetRenderDrawColor(video->render, 255, 255, 255, 255);
	SDL_RenderClear(video->render);
	SDL_RenderPresent(video->render);

	return true;
}

void sdl2_finish_video(emu_state *state)
{
	sdl2_video_data *video = state->front.video.data;

	SDL_DestroyTexture(video->texture);
	SDL_DestroyRenderer(video->render);
	SDL_DestroyWindow(video->window);

	free(video);
	state->front.video.data = NULL;

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	info(state, "SDL video frontend has left the building!");
}

void sdl2_blit_canvas(emu_state *state)
{
	sdl2_video_data *video = state->front.video.data;

	SDL_UpdateTexture(video->texture, NULL, state->lcdc.out,
			PITCH);

	SDL_RenderCopy(video->render, video->texture, NULL, NULL);
	SDL_RenderPresent(video->render);
}


const frontend_video sdl2_frontend_video =
{
	&sdl2_init_video,
	&sdl2_finish_video,
	&sdl2_blit_canvas,
	NULL,
};
