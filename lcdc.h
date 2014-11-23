#ifndef __LCDC_H_
#define __LCDC_H_

#include "config.h"	// macros

#include <stdint.h>	// uint*_t


typedef struct _emu_state emu_state;

typedef struct _lcdc_state
{
	uint32_t curr_clk;		/* current clock */
} lcdc;


uint8_t lcdc_read(emu_state *, uint16_t);
void lcdc_write(emu_state *, uint16_t, uint8_t);
void lcdc_tick(emu_state *);

#endif /*!__LCDC_H_*/
