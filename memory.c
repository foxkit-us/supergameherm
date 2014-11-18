#include <stdint.h>	// uint[XX]_t

#include "print.h"	// fatal
#include "memory.h"	// Constants and what have you */


/***********************************************************************
 * readers
 ***********************************************************************/
typedef uint8_t (*mem_read_fn)(emulator_state*, uint16_t);

uint8_t direct_read(emulator_state *state, uint16_t location)
{
	return state->memory[location];
}

uint8_t rom_bank_read(emulator_state *state, uint16_t location)
{
	fatal("bank switching not yet implemented");
	/* NOTREACHED */
}

uint8_t not_impl(emulator_state *state, uint16_t location)
{
	fatal("reading from %04X is not yet implemented", location);
	/* NOTREACHED */
}

uint8_t ram_bank_read(emulator_state *state, uint16_t location)
{
	fatal("RAM bank switching not yet implemented");
	/* NOTREACHED */
}

uint8_t shadow_read(emulator_state *state, uint16_t location)
{
	/* Shadow is offset */
	return state->memory[location - 0x2000];
}

uint8_t f_read(emulator_state *state, uint16_t location)
{
	/* who knows?  the SHADOW knows! */
	if(location < 0xFE00)
	{
		return shadow_read(state, location);
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
	if(location < 0xFF80)
	{
	fatal("still not sure how I want to implement I/O access at %04X",
		location);
	}
	return direct_read(state, location);
}

mem_read_fn readers[0x10] = {
	/* ROM bank #0 - 0x0000..0x3FFF */
	direct_read, direct_read, direct_read, direct_read,
	/* switchable bank - 0x4000..0x7FFF */
	rom_bank_read, rom_bank_read, rom_bank_read, rom_bank_read,
	/* video memory - 0x8000..0x9FFF */
	not_impl, not_impl,
	/* switchable RAM bank - 0xA000..0xBFFF */
	ram_bank_read, ram_bank_read,
	/* internal RAM - 0xC000..0xDFFF */
	direct_read, direct_read,
	/* who knows? the SHADOW knows! - 0xE000..0xFDFF */
	shadow_read,
	/* special stuff - 0xFE00-0xFFFF */
	f_read
};

uint8_t mem_read8(emulator_state *state, uint16_t location)
{
	char reader = location >> 12;

	if(location > 0xFFFF)
	{
		fatal("invalid memory access - out of bounds");
	}

	return readers[reader](state, location);
}

uint16_t mem_read16(emulator_state *state, uint16_t location)
{
	return (mem_read8(state, location+1) << 8) | mem_read8(state, location);
}




/***********************************************************************
 * writers
 ***********************************************************************/
void mem_write8(emulator_state *state, uint16_t location, uint8_t data)
{
	//if(location < 0xC000 || location >= 0xFE00)
	//	fatal("invalid memory write at %04X (%02X)", location, data);
	if(location < 0xE000)
	{
		debug("wrote %02X to %04X", data, location);
		state->memory[location] = data;
		return;
	}
	if(location < 0xFE00)
	{
		state->memory[location - 0x2000] = data;
		return;
	}
	if(location >= 0xFE80)
	{
		state->memory[location] = data;
		return;
	}
	debug("IGNORING write of %02X to %04X", data, location);
}

void mem_write16(emulator_state *state, uint16_t location, uint16_t data)
{
	/* TODO this needs to be filtered just like the readers */
	unsigned char *l = state->memory + location;
	uint16_t *hax = (uint16_t *)l;

	*l = data;
	//fatal("invalid memory write at %04X (%04X)", location, data);
}
