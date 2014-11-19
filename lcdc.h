#ifndef __LCDC_H_
#define __LCDC_H_

#include "sgherm.h"	// emulator_state

uint8_t lcdc_read(emulator_state *, uint16_t);
void lcdc_write(emulator_state *, uint16_t, uint8_t);
void lcdc_tick(emulator_state *);

#endif /*!__LCDC_H_*/
