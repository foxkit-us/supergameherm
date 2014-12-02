#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include "config.h"	// bool
#include "typedefs.h"	// typedefs


typedef enum
{
	KEY_NONE,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_A,
	KEY_B,
	KEY_SELECT,
	KEY_START,
} frontend_key;

struct frontend_input_t
{
	bool (*init)(emu_state *);	/*! Initalise the keyboard input */
	void (*finish)(emu_state *);	/*! Deinitalise the keyboard input */
	frontend_key (*get_key)(emu_state *);	/*! Get a key */
};

struct frontend_audio_t
{
	bool (*init)(emu_state *);	/*! Initalise the audio output */
	void (*finish)(emu_state *);	/*! Deinitalise the audio output */
	void (*output_sample)(emu_state *);	/*! Output an audio sample */
};

struct frontend_video_t
{
	bool (*init)(emu_state *);	/*! Initalise the video output */
	void (*finish)(emu_state *);	/*! Deinitalise the video output */
	void (*blit_canvas)(emu_state *);	/*! Blit the canvas */
};

struct frontend_t
{
	frontend_input input;
	frontend_audio audio;
	frontend_video video;

	int (*event_loop)(emu_state *);	/*! Event loop function (for use with toolkits */
};


/*! Frontends that do nothing */
extern frontend_input null_frontend_input;
extern frontend_audio null_frontend_audio;
extern frontend_video null_frontend_video;
int null_event_loop(emu_state *);

#define CALL_FRONTEND(state, type, fn) ((*(state->front.type.fn))(state))
#define EVENT_LOOP(state) ((*(state->front.event_loop))(state))

#define INIT_INPUT(state) CALL_FRONTEND(state, input, init)
#define INIT_AUDIO(state) CALL_FRONTEND(state, audio, init)
#define INIT_VIDEO(state) CALL_FRONTEND(state, video, init)
#define FINISH_INPUT(state) CALL_FRONTEND(state, input, finish)
#define FINISH_AUDIO(state) CALL_FRONTEND(state, audio, finish)
#define FINISH_VIDEO(state) CALL_FRONTEND(state, video, finish)

#define INIT_ALL(state) \
	{ \
		INIT_INPUT(state); \
		INIT_AUDIO(state); \
		INIT_VIDEO(state); \
	}

#define FINISH_ALL(state) \
	{ \
		CALL_FRONTEND(state, input, finish); \
		CALL_FRONTEND(state, audio, finish); \
		CALL_FRONTEND(state, video, finish); \
	}

#define BLIT_CANVAS(state) CALL_FRONTEND(state, video, blit_canvas)
#define OUTPUT_SAMPLE(state) CALL_FRONTEND(state, audio, output_sample)
#define GET_KEY(state) CALL_FRONTEND(state, input, get_key)

#endif /*__FRONTEND_H__*/
