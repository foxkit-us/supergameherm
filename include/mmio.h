#ifndef __MMIO_H__
#define __MMIO_H__

#include "sgherm.h"

uint8_t hw_read(emu_state *restrict, uint16_t);
void hw_write(emu_state *restrict, uint16_t, uint8_t);

#endif //__MMIO_H__
