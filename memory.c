#include <stdint.h>	// uint[XX]_t

unsigned char memory[0x10000];

uint8_t mem_read8(uint16_t location)
{
	return memory[location];
}

uint16_t mem_read16(uint16_t location)
{
	return (memory[location+1] << 8) | memory[location];
}

void mem_write8(uint16_t location, uint8_t data)
{
}

void mem_write16(uint16_t location, uint8_t data)
{
}