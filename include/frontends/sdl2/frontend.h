#ifndef __FRONTEND_SDL2_FRONTEND_H__
#define __FRONTEND_SDL2_FRONTEND_H__

extern const frontend_input sdl2_frontend_input;
extern const frontend_audio sdl2_frontend_audio;
extern const frontend_video sdl2_frontend_video;
int sdl2_event_loop(emu_state *);

#endif
