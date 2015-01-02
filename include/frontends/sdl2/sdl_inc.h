#ifndef __FRONTEND_SDL2_SDL_INC_H__
#define __FRONTEND_SDL2_SDL_INC_H__

// SDL 2 whines. *sigh*
#ifdef HAVE_STDINT_H
#       undef HAVE_STDINT_H
#endif
#ifdef HAVE_CLOCK_GETTIME
#       undef HAVE_CLOCK_GETTIME
#endif
#ifdef HAVE_NANOSLEEP
#       undef HAVE_NANOSLEEP
#endif

#include <SDL2/SDL.h>

#endif //__FRONTEND_SDL2_SDL_INC_H__
