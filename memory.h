#ifndef __MEMORY_H_
#define __MEMORY_H_

uint8_t mem_read8(uint16_t location);
uint16_t mem_read16(uint16_t location);
void mem_write8(uint16_t location, uint8_t data);
void mem_write16(uint16_t location, uint16_t data);

/* 8-bit address space */
#define MEM_SIZE	0x10000

typedef enum
{
	/* TODO GBC constants */

	/* restart and interrupt vectors */
	vec_begin = 0x0000;
	vec_end = 0x00FF;

	/* cartridge data */
	cart_begin = 0x0100;

	/* 4 bytes */
	start_instr_begin = 0x0100;
	start_instr_end = 0x0103;

	/* 10 bytes */
	game_title_begin = 0x0134;
	game_title_end = 0x013E;

	/* 4 bytes */
	game_desig_begin = 0x013F;
	game_desig_end = 0x0142;

	/* 1 byte */
	color_compat = 0x0143;

	/* 2 bytes */
	licensee_begin = 0x0144;
	licensee_end = 0x0145;

	/* 1 byte */
	sgb_compat = 0x0146;

	cart_type = 0x0147;

	cart_rom_size = 0x0148;

	cart_ram_size = 0x0149;

	cart_dest_code = 0x014A; /* Japanese or not */

	old_licensee = 0x014B; /* Mostly unused */

	mask_rom_ver = 0x014C;

	header_checksum = 0x014D;

	/* 2 bytes */
	cart_checksum_begin = 0x014E;
	cart_checksum_end = 0x014F;

	cart_end = 0x014F;

	/* Fixed bank */
	rom0_begin = 0x0150;
	rom0_end = 0x3FFF;

	/* Switchable bank */
	rom_bank_begin = 0x4000;
	rom_bank_end = 0x7FFF;

	tile_ram_begin = 0x8000;
	tile_ram_end = 0x97FF;

	bg0_begin = 0x9800;
	bg0_end = 0x9BFF;

	bg1_begin = 0x9C00;
	bg1_end = 0x9FFF;

	/* Not present on all carts! */
	cart_ram_begin = 0xA000;
	cart_ram_end = 0xBFFF;

	ram_begin = 0xC000;
	ram_end = 0xDFFF;

	shadow_begin = 0xE000;
	shadow_end = 0xFDFF;

	oam_begin = 0xFE00;
	oam_end = 0xFE9F;

	/* Contains control registers and other doodads */
	mmio_begin = 0xFF00;
	/* TODO I/O constants */
	mmio_end = 0xFF7F;

	high_ram_begin = 0xFF80;
	high_ram_end = 0xFFFE;

	/* Interrupts locked out */
	int_flag = 0xFFFF;
} offsets;

#endif /*!__MEMORY_H_*/
