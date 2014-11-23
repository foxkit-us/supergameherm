#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "config.h"	// macros

#include <stdint.h>	// (u)int*_t
#include <stdbool.h>	// bool
#include <stdio.h>	// FILE


typedef struct _emu_state emu_state;

uint8_t mem_read8(emu_state *restrict, uint16_t);
uint16_t mem_read16(emu_state *restrict, uint16_t);
void mem_write8(emu_state *restrict, uint16_t, uint8_t);
void mem_write16(emu_state *restrict, uint16_t, uint16_t);

#endif /*!__MEMORY_H_*/
