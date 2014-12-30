#include "sgherm.h"	// emu_state,
#include "print.h"	// debug
#include "signals.h"	// do_exit
#include "frontend.h"	// frontend

#include <caca.h>	// libcaca
#include <stdbool.h>	// bool
#include <stdlib.h>	// calloc
#include <string.h>	// strerror
#include <errno.h>	// errno


#define LEN 160
#define WID 144
#define BPP 32
#define PITCH 4

#define RED     0x000000ff
#define GREEN   0x0000ff00
#define BLUE    0x00ff0000
#define ALPHA   0xff000000


bool libcaca_init_video(emu_state *state)
{
	libcaca_video_data *video;

	info(state, "Initalising the libcaca video frontend!");

	video = calloc(sizeof(libcaca_video_data), 1);
	state->front.video.data = (void *)video;

	// Redirect stdout and stderr
	if(!(video->stdout_new = fopen("stdout.log", "w")))
	{
		fatal(state, "Could not open stdout.log: %s", strerror(errno));
		free(video);
		return false;
	}

	if(!(video->stderr_new = fopen("stderr.log", "w")))
	{
		fatal(state, "Could not open stderr.log: %s", strerror(errno));
		fclose(video->stdout_new);
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

	video->dither = caca_create_dither(BPP, LEN, WID, LEN*PITCH,
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

void libcaca_blit_canvas(emu_state *state)
{
	libcaca_video_data *video = state->front.video.data;
	const int wid = caca_get_canvas_width(video->canvas);
	const int height = caca_get_canvas_height(video->canvas);

	caca_dither_bitmap(video->canvas, 0, 0, wid, height, video->dither,
			state->lcdc.out);
	caca_refresh_display(video->display);
}


const frontend_video libcaca_frontend_video =
{
	&libcaca_init_video,
	&libcaca_finish_video,
	&libcaca_blit_canvas,
	NULL,
};
