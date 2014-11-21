#ifndef __INPUT_H_
#define __INPUT_H_

#include "config.h"	// macros

#include <stdint.h>	// uint[XX]_t


uint8_t joypad_read(emu_state *restrict, uint16_t);
void joypad_write(emu_state *restrict, uint16_t, uint8_t);

#endif /*!__INPUT_H_*/
