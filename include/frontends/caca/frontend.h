#ifndef __FRONTEND_CACA_FRONTEND_H__
#define __FRONTEND_CACA_FRONTEND_H__

#include <stdio.h>	// FILE
#include <caca.h>	// caca_*


extern const frontend_video libcaca_frontend_video;
int libcaca_event_loop(emu_state *);


typedef struct libcaca_video_data_t
{
	caca_canvas_t *canvas;
	caca_display_t *display;
	caca_dither_t *dither;

	FILE *stdout_new;
	FILE *stderr_new;
} libcaca_video_data;

#endif /*__FRONTEND_CACA_FRONTEND_H__*/
