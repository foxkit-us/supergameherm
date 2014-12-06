#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend

#include <caca.h>	// libcaca
#include <stdbool.h>	// bool
#include <stdlib.h>	// calloc


typedef struct libcaca_video_data_t
{
	caca_canvas_t *canvas;
	caca_display_t *display;
	caca_dither_t *dither;
} libcaca_video_data;


bool libcaca_init_video(emu_state *state)
{
	libcaca_video_data *video;

	info("Initalising the libcaca video frontend!");

	video = calloc(sizeof(libcaca_video_data), 1);
	state->front.video.data = (void *)video;

	video->canvas = caca_create_canvas(160, 144);
	video->display = caca_create_display(video->canvas);
	if(!(video->display))
	{
		warning("Failed to initalise the libcaca video frontend");
		caca_free_canvas(video->canvas);
		return false;
	}

	caca_set_display_title(video->display, "SuperGameHerm");

	video->dither = caca_create_dither(32, 160, 144, 4*160,
			0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	if(!(video->dither))
	{
		warning("Failed to initalise the libcaca video frontend");
		caca_free_display(video->display);
		caca_free_canvas(video->canvas);
		return false;
	}

	return true;
}

bool libcaca_init_input(emu_state *state unused)
{
	// stub
	return true;
}

void libcaca_finish_video(emu_state *state)
{
	libcaca_video_data *video = state->front.video.data;

	info("libcaca video frontend has left the building!");

	caca_free_display(video->display);
	caca_free_dither(video->dither);
	caca_free_canvas(video->canvas);
	free(video);

	state->front.video.data = NULL;
}

void libcaca_finish_input(emu_state *state unused)
{
	// stub
	return;
}

void libcaca_blit_canvas(emu_state *state)
{
	libcaca_video_data *video = state->front.video.data;

	caca_dither_bitmap(video->canvas, 0, 0, 159, 143, video->dither, state->lcdc.out);
	caca_refresh_display(video->display);
}

input_key libcaca_get_key(emu_state *state)
{
	libcaca_video_data *video = state->front.video.data;
	caca_event_t ev;
	int ret = caca_get_event(video->display, CACA_EVENT_KEY_PRESS, &ev, 0);

	if(!ret)
	{
		return INPUT_NONE;
	}

	switch(caca_get_event_key_ch(&ev))
	{
	case CACA_KEY_UP:
		return INPUT_UP;

	case CACA_KEY_DOWN:
		return INPUT_DOWN;

	case CACA_KEY_LEFT:
		return INPUT_LEFT;

	case CACA_KEY_RIGHT:
		return INPUT_RIGHT;

	case 'a':
	case 'A':
	case 'z':
	case 'Z':
		return INPUT_A;

	case 's':
	case 'S':
	case 'x':
	case 'X':
		return INPUT_B;

	case CACA_KEY_RETURN:
		return INPUT_START;

	case CACA_KEY_BACKSPACE:
		return INPUT_SELECT;

	case CACA_KEY_ESCAPE:
		do_exit = true;

	default:
		return INPUT_NONE;
	}
}

int libcaca_event_loop(emu_state *state)
{
	debug("Executing libcaca event loop");

	do
	{
		step_emulator(state);
		if(state->lcdc.stat.params.mode_flag == 3)
		{
			input_key key = GET_KEY(state);
			joypad_signal(state, key, (key != INPUT_NONE));
		}

	} while(!do_exit);

	return 0;
}


const frontend_video libcaca_frontend_video = {
	&libcaca_init_video,
	&libcaca_finish_video,
	&libcaca_blit_canvas,
	NULL,
};

const frontend_input libcaca_frontend_input = {
	&libcaca_init_input,
	&libcaca_finish_input,
	&libcaca_get_key,
	NULL,
};
