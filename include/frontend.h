#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include "config.h"	// bool

typedef struct _emu_state emu_state;

typedef struct _frontend
{
	// Initialisation
	bool (*init_video)(emu_state *);	/*! Initalise the video output */
	bool (*init_audio)(emu_state *);	/*! Initalise the audio output */
	bool (*init_input)(emu_state *);	/*! Initalise the keyboard input */

	// Finalisation
	void (*finish_video)(emu_state *);	/*! Deinitalise the video output */
	void (*finish_audio)(emu_state *);	/*! Deinitalise the audio output */
	void (*finish_input)(emu_state *);	/*! Deinitalise the keyboard input */

	// Video related functions
	void (*blit_canvas)(emu_state *);	/*! Blit the canvas */

	// Audio related functions
	void (*output_sample)(emu_state *);	/*! Output an audio sample */

	// Event loop (ingest external events here)
	int (*event_loop)(emu_state *);
} frontend;


/*! Frontend that does nothing */
extern frontend null_frontend;


#define CALL_FRONTEND(state, fn) ((*(state->front.fn))(state))


#endif /*__FRONTEND_H__*/
