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

#include <SDL.h>	// SDL
#include <stdbool.h>	// bool
#include <stdlib.h>	// calloc


#define LEN 144
#define WID 160
#define BPP 32
#define PITCH 4

#ifdef LITTLE_ENDIAN
#define RED	0x000000ff
#define GREEN	0x0000ff00
#define BLUE	0x00ff0000
#define ALPHA	0xff000000
#else
#define RED	0xff000000
#define GREEN	0x00ff0000
#define BLUE	0x0000ff00
#define ALPHA	0x000000ff
#endif


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
		error(state, "Failed to initalise video frontend: %s", SDL_GetError());
		return false;
	}

	video = calloc(1, sizeof(sdl2_video_data));
	state->front.video.data = video;

	video->window = SDL_CreateWindow("SuperGameHerm",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			320, 288, SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN);
	if(!(video->window))
	{
		error(state, "Failed to initalise video frontend: %s", SDL_GetError());
		free(video);
		return false;
	}

	video->render = SDL_CreateRenderer(video->window, -1, 0);
	if(!(video->render))
	{
		error(state, "Failed to initalise video frontend: %s", SDL_GetError());
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
		error(state, "Failed to initalise video frontend: %s", SDL_GetError());
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

void sdl2_finish_input(emu_state *state UNUSED)
{
	info(state, "SDL input frontend is outta here!");

	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void sdl2_finish_audio(emu_state *state UNUSED)
{
	info(state, "SDL audio frontend finishing up");

	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void sdl2_blit_canvas(emu_state *state)
{
	sdl2_video_data *video = state->front.video.data;

	SDL_UpdateTexture(video->texture, NULL, state->lcdc.out,
			WID * sizeof(uint32_t));

	SDL_RenderCopy(video->render, video->texture, NULL, NULL);
	SDL_RenderPresent(video->render);
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
}

void sdl2_output_sample(emu_state *state UNUSED)
{
	// TODO
	return;
}

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
			if(ret.key > 0)
			{
				joypad_signal(state, ret.key, ret.press);
			}
		}
	} while(!do_exit);

	return 0;
}


const frontend_video sdl2_frontend_video =
{
	&sdl2_init_video,
	&sdl2_finish_video,
	&sdl2_blit_canvas,
	NULL,
};

const frontend_input sdl2_frontend_input =
{
	&sdl2_init_input,
	&sdl2_finish_input,
	&sdl2_get_key,
	NULL,
};

const frontend_audio sdl2_frontend_audio =
{
	&sdl2_init_audio,
	&sdl2_finish_audio,
	&sdl2_output_sample,
	NULL,
};
