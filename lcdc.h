#ifndef __LCDC_H_
#define __LCDC_H_

#include "config.h"	// macros

#include "sgherm.h"	// emu_state

uint8_t lcdc_read(emu_state *, uint16_t);
void lcdc_write(emu_state *, uint16_t, uint8_t);
void lcdc_tick(emu_state *);

#endif /*!__LCDC_H_*/
