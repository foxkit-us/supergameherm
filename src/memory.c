#include "config.h"	// macros, uint[XX]_t
#include <assert.h>	// assert
#include <string.h>	// memmove

#include "sgherm.h"	// emu_state
#include "memory.h"	// Constants and what have you
#include "mmio.h"	// hw_*
#include "print.h"	// fatal
#include "util.h"	// likely/unlikely


/*!
 * @brief	Read a byte (8 bits) out of memory.
 * @param	state		The emulator state to use when reading.
 * @param	location	The location in memory to read.
 * @returns	The value of the location in memory.
 * @result	Emulation will terminate if the memory cannot be read.
 */
uint8_t mem_read8(emu_state *restrict state, uint16_t location)
{
	if(state->dma_wait && location <= 0xFE80 && location >= 0xFFFE)
	{
		// XXX check into it and see how this is handled on real hardware
		fatal(state, "Prohibited read during DMA transfer");
		return 0xFF;
	}

	if(unlikely(state->in_bootrom && location < state->bootrom_size))
	{
		// Boot ROM overlays normal memory map
		return state->bootrom_data[location];
	}

	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		// switchable ROM bank - 0x4000..0x7FFF (depends on MBC)
	case 0xA:
	case 0xB:
		// switchable RAM bank - 0xA000-0xBFFF (depends on MBC)
		return MBC_READ(state, location);
	case 0x8:
	case 0x9:
		// video memory - 0x8000..0x9FFF
		// TODO - hooks for debug on bad read
		return state->lcdc.vram[state->lcdc.vram_bank][location - 0x8000];
	case 0xC:
		// Work RAM - 0xC000..0xCFFF
		return state->wram[0][location - 0xC000];
	case 0xD:
		// Work RAM banks 1-7 - 0xD000.0xDFFF
		if(state->system == SYSTEM_CGB)
		{
			return state->wram[state->wram_bank][location - 0xD000];
		}
		else
		{
			return state->wram[1][location - 0xD000];
		}
	case 0xE:
	case 0xF:
		if(unlikely(location <= 0xFDFF))
		{
			// Echo RAM - 0xE000..0xFDFF
			return mem_read8(state, location - 0x2000);
		}
		else if(location <= 0xFE9F)
		{
			// OAM RAM - 0xFE00..0xFE9F
			return state->lcdc.oam_ram[location - 0xFE00];
		}
		else if(unlikely(location <= 0xFEFF))
		{
			// Unusable RAM - 0xFEA0..0xFEFF
			return 0xFF;
		}
		else if(location <= 0xFF7F)
		{
			// Hardware - 0xFF00..0xFF7F
			return hw_read(state, location);
		}
		else if(location <= 0xFFFE)
		{
			// High RAM - 0xFF80..FFFE
			return state->hram[location - 0xFF80];
		}
		else
		{
			// Interrupt mask flag - 0xFFFF
			return state->interrupts.mask;
		}
	}

	warning(state, "Memory read outside of real RAM at %04X",
		location);
	return 0xFF;
}

/*!
 * @brief	Read two bytes (16 bits) out of memory.
 * @param	state		The emulator state to use when reading.
 * @param	location	The location in memory to read.
 * @returns	The value of the two bytes at the location, LSB lower.
 * @result	Emulation will terminate if the memory cannot be read.
 */
uint16_t mem_read16(emu_state *restrict state, uint16_t location)
{
	return (mem_read8(state, location + 1) << 8) | mem_read8(state, location);
}

/*!
 * @brief	Write a byte (8 bits) to memory.
 * @param	state		The emulator state to use while writing.
 * @param	location	The location in memory to write.
 * @param	data		The data to write.
 * @result	The data is written to the specified location.
 * 		Emulation will terminate if the memory cannot be written to.
 */
void mem_write8(emu_state *restrict state, uint16_t location, uint8_t data)
{
	if(state->dma_wait && location <= 0xFE80 && location >= 0xFFFE)
	{
		fatal(state, "Prohibited write during DMA transfer");
		return;
	}

	switch(location >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		// MBC controller regs (varies based on MBC)
	case 0xA:
	case 0xB:
		// switched RAM bank (depends on MBC)
		MBC_WRITE(state, location, data);
		return;
	case 0x8:
	case 0x9:
		// VRAM
		// TODO - hook on bad writes outside vblank
		state->lcdc.vram[state->lcdc.vram_bank][location - 0x8000] = data;
		return;
	case 0xC:
		state->wram[0][location - 0xC000] = data;
		return;
	case 0xD:
		if(state->system == SYSTEM_CGB)
		{
			state->wram[state->wram_bank][location - 0xD000] = data;
		}
		else
		{
			state->wram[1][location - 0xD000] = data;
			return;
		}
		return;
	case 0xE:
	case 0xF:
		if(unlikely(location <= 0xFDFF))
		{
			// Echo RAM - 0xE000..0xFDFF
			mem_write8(state, location - 0x2000, data);
		}
		else if(location <= 0xFE9F)
		{
			// OAM RAM - 0xFE00..0xFE9F
			state->lcdc.oam_ram[location - 0xFE00] = data;
		}
		else if(unlikely(location <= 0xFEFF))
		{
			// Unusable RAM - 0xFEA0..0xFEFF
			return;
		}
		else if(location <= 0xFF7F)
		{
			// Hardware - 0xFF00..0xFF7F
			hw_write(state, location, data);
		}
		else if(location <= 0xFFFE)
		{
			// High RAM - 0xFF80..FFFE
			state->hram[location - 0xFF80] = data;
		}
		else
		{
			// Interrupt mask flag - 0xFFFF
			state->interrupts.mask = data;
			compute_irq(state);
		}
		return;
	}

	warning(state, "Memory write outside of real RAM at %04X",
		location);
}

void mem_write16(emu_state *restrict state, uint16_t location, uint16_t data)
{
        mem_write8(state, location++, (uint8_t)(data & 0xFF));
        mem_write8(state, location, (uint8_t)(data >> 8));
}
