#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend

#include <caca.h>	// libcaca
#include <stdbool.h>	// bool

bool libcaca_init_input(emu_state *state UNUSED)
{
	// stub
	return true;
}
void libcaca_finish_input(emu_state *state UNUSED)
{
	// stub
	return;
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
	else if(ev_type & CACA_EVENT_RESIZE)
	{
		const int wid = caca_get_event_resize_width(&ev);
		const int height = caca_get_event_resize_height(&ev);
		libcaca_video_data *video = state->front.video.data;

		caca_set_canvas_size(video->canvas, wid, height);

		// Replot bitmap
		caca_dither_bitmap(video->canvas, 0, 0, wid, height, video->dither,
			state->lcdc.out);
		caca_refresh_display(video->display);
	}
}


const frontend_input libcaca_frontend_input =
{
	&libcaca_init_input,
	&libcaca_finish_input,
	&libcaca_get_key,
	NULL,
};
