#ifndef __MMAP_H__
#define __MMAP_H__

#include "config.h"	// config
#include "sgherm.h"	// emu_state

void * memmap_open(emu_state *restrict, const char *, size_t, memmap_state **);
void * memmap_resize(emu_state *restrict, void *, size_t, memmap_state **);
void memmap_close(emu_state *restrict, void *, memmap_state **);
void memmap_sync(emu_state *restrict, void *, memmap_state **);


#endif /*!__MMAP_H__*/
