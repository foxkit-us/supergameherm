#include "config.h"	// bool etc.

#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend
#include "frontends/caca/frontend.h"

static inline input_key get_key(caca_event_t *ev)
{
	switch(caca_get_event_key_ch(ev))
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
		return 0;
	}
}

int libcaca_event_loop(emu_state *state)
{
	debug(state, "Executing libcaca event loop");

	do
	{
		libcaca_video_data *video = state->front.video.data;
		caca_event_t ev;
		uint8_t mode = LCDC_STAT_MODE_FLAG(state);
		uint_fast16_t clock = state->lcdc.curr_clk;
		int events;

		step_emulator(state);

		if(unlikely(mode == 1 && clock == 1 && state->input.col))
		{
			if(!caca_get_event(video->display, CACA_EVENT_KEY_PRESS |
				CACA_EVENT_KEY_RELEASE | CACA_EVENT_QUIT, &ev, 0))
			{
				continue;
			}
		}
		else
		{
			continue;
		}

		events = caca_get_event_type(&ev);

		if(unlikely(events & CACA_EVENT_QUIT))
		{
			do_exit = true;
		}
		else if(events & CACA_EVENT_RESIZE)
		{
			// XXX check if libcaca video backend
			int wid = caca_get_event_resize_width(&ev);
			int height = caca_get_event_resize_height(&ev);
			libcaca_video_data *video = state->front.video.data;

			caca_set_canvas_size(video->canvas, wid, height);

			// Replot bitmap
			caca_dither_bitmap(video->canvas, 0, 0, wid, height, video->dither,
				state->lcdc.out);
			caca_refresh_display(video->display);
		}
		else if(events & (CACA_EVENT_KEY_PRESS | CACA_EVENT_KEY_RELEASE))
		{
			bool pressed = (events & CACA_EVENT_KEY_PRESS) != 0;;
			input_key key = get_key(&ev);

			if(!key)
			{
				continue;
			}

			joypad_signal(state, key, pressed);
		}
	} while(!do_exit);

	return 0;
}
