#ifndef __FRONTENDS_W32_FRONTEND_H__
#define __FRONTENDS_W32_FRONTEND_H__

extern const frontend_video w32_frontend_video;
extern const frontend_audio w32_frontend_audio;
int w32_event_loop(emu_state *restrict);


#define WIN32_AUDIO &w32_frontend_audio
#define WIN32_VIDEO &w32_frontend_video
#define WIN32_LOOP &w32_event_loop

#endif
