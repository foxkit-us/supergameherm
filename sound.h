#ifndef __SOUND_H_
#define __SOUND_H_

#include "config.h"	// Various macros

#include <stdint.h>	// uint[XX]_t

#include "sgherm.h"	// emu_state


uint8_t sound_read(emu_state *restrict, uint16_t);
void sound_write(emu_state *restrict, uint16_t, uint8_t);
void sound_tick(emu_state *restrict);

#endif /*!__SOUND_H_*/
