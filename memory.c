#include <stdint.h>	// uint[XX]_t

#include "print.h"	// fatal


unsigned char memory[0x10000];

uint8_t mem_read8(uint16_t location)
{
	/* ROM bank #0 */
	if(location < 0x4000)
	{
		return memory[location];
	}
	/* switchable ROM bank */
	if(location < 0x8000)
	{
		fatal("bank switching not yet implemented");
	}
	/* video memory */
	if(location < 0xA000)
	{
		fatal("video RAM not yet implemented");
	}
	/* switchable RAM bank */
	if(location < 0xC000)
	{
		fatal("RAM bank switching not yet implemented");
	}
	/* internal RAM */
	if(location < 0xE000)
	{
		return memory[location];
	}
	/* who knows?  the SHADOW knows! */
	if(location < 0xFE00)
	{
		return memory[location - 0x2000];
	}
	/* OAM */
	if(location < 0xFEA0)
	{
		fatal("OAM not yet implemented");
	}
	/* invalid */
	if(location < 0xFF00)
	{
		fatal("invalid memory read at %04X", location);
	}
	/* I/O / hardware stuff */
	fatal("still not sure how I want to implement I/O access");
}

uint16_t mem_read16(uint16_t location)
{
	return (mem_read8(location+1) << 8) | mem_read8(location);
}

void mem_write8(uint16_t location, uint8_t data)
{
	fatal("invalid memory write at %04X (%02X)", location, data);
}

void mem_write16(uint16_t location, uint16_t data)
{
	fatal("invalid memory write at %04X (%04X)", location, data);
}