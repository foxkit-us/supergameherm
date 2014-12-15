#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend

#include <caca.h>	// libcaca
#include <stdbool.h>	// bool
#include <stdlib.h>	// calloc
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define LEN 160
#define WID 144
#define BPP 32
#define PITCH 4

#ifdef LITTLE_ENDIAN
#define ALPHA	0xff000000
#define RED	0x00ff0000
#define GREEN	0x0000ff00
#define BLUE	0x000000ff
#else
#define ALPHA	0x000000ff
#define RED	0x0000ff00
#define GREEN	0x00ff0000
#define BLUE	0xff000000
#endif


typedef struct libcaca_video_data_t
{
	caca_canvas_t *canvas;
	caca_display_t *display;
	caca_dither_t *dither;

	FILE *stdout_new;
	FILE *stderr_new;
} libcaca_video_data;


bool libcaca_init_video(emu_state *state)
{
	libcaca_video_data *video;

	info(state, "Initalising the libcaca video frontend!");

	video = calloc(sizeof(libcaca_video_data), 1);
	state->front.video.data = (void *)video;

	// Redirect stdout and stderr
	video->stdout_new = fopen("stdout.log", "w");
	video->stderr_new = fopen("stderr.log", "w");

	if(!(video->stdout_new && video->stderr_new))
	{
		perror("fopen");
		free(video);
		return false;
	}

	to_stdout = video->stdout_new;
	to_stderr = video->stderr_new;

	video->display = caca_create_display(NULL);
	if(!(video->display))
	{
		warning(state, "Failed to initalise the libcaca video frontend");
		free(video);
		return false;
	}

	video->canvas = caca_get_canvas(video->display);

	caca_set_display_title(video->display, "SuperGameHerm");

	video->dither = caca_create_dither(BPP, LEN, WID, WID*PITCH,
			RED, GREEN, BLUE, 0);
	if(!(video->dither))
	{
		warning(state, "Failed to initalise the libcaca video frontend");
		caca_free_display(video->display);
		free(video);
		return false;
	}

	return true;
}

bool libcaca_init_input(emu_state *state UNUSED)
{
	// stub
	return true;
}

void libcaca_finish_video(emu_state *state)
{
	libcaca_video_data *video = state->front.video.data;

	caca_free_display(video->display);
	caca_free_dither(video->dither);

	fclose(video->stdout_new);
	fclose(video->stderr_new);

	to_stdout = stdout;
	to_stderr = stderr;

	free(video);
	state->front.video.data = NULL;

	info(state, "libcaca video frontend has left the building!");
}

void libcaca_finish_input(emu_state *state UNUSED)
{
	// stub
	return;
}

void libcaca_blit_canvas(emu_state *state)
{
	libcaca_video_data *video = state->front.video.data;
	int wid = caca_get_canvas_width(video->canvas);
	int height = caca_get_canvas_height(video->canvas);

	caca_dither_bitmap(video->canvas, 0, 0, wid, height, video->dither,
			state->lcdc.out);
	caca_refresh_display(video->display);
}

void libcaca_get_key(emu_state *state, frontend_input_return *ret)
{
	libcaca_video_data *video = state->front.video.data;
	caca_event_t ev;
	int ev_type;

	if(!caca_get_event(video->display, CACA_EVENT_KEY_PRESS |
		CACA_EVENT_KEY_RELEASE | CACA_EVENT_QUIT, &ev, 0))
	{
		ret->key = 0;
		return;
	}

	ev_type = caca_get_event_type(&ev);

	if(ev_type & (CACA_EVENT_KEY_PRESS | CACA_EVENT_KEY_RELEASE))
	{
		ret->press = ev_type & CACA_EVENT_KEY_PRESS ? true : false;

		switch(caca_get_event_key_ch(&ev))
		{
		case CACA_KEY_UP:
			ret->key = INPUT_UP;
			break;

		case CACA_KEY_DOWN:
			ret->key = INPUT_DOWN;
			break;

		case CACA_KEY_LEFT:
			ret->key = INPUT_LEFT;
			break;

		case CACA_KEY_RIGHT:
			ret->key = INPUT_RIGHT;
			break;

		case 'a':
		case 'A':
		case 'z':
		case 'Z':
			ret->key = INPUT_A;
			break;

		case 's':
		case 'S':
		case 'x':
		case 'X':
			ret->key = INPUT_B;
			break;

		case CACA_KEY_RETURN:
			ret->key = INPUT_START;
			break;

		case CACA_KEY_BACKSPACE:
			ret->key = INPUT_SELECT;
			break;

		case CACA_KEY_ESCAPE:
			do_exit = true;

		default:
			ret->key = 0;
			break;
		}
	}
	else if(ev_type & CACA_EVENT_QUIT)
	{
		do_exit = true;
		ret->key = 0;
	}
}

int libcaca_event_loop(emu_state *state)
{
	debug(state, "Executing libcaca event loop");

	do
	{
		frontend_input_return ret;
		uint8_t mode = state->lcdc.stat.params.mode_flag;
		uint_fast16_t clock = state->lcdc.curr_clk;

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


const frontend_video libcaca_frontend_video =
{
	&libcaca_init_video,
	&libcaca_finish_video,
	&libcaca_blit_canvas,
	NULL,
};

const frontend_input libcaca_frontend_input =
{
	&libcaca_init_input,
	&libcaca_finish_input,
	&libcaca_get_key,
	NULL,
};
