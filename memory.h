#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h> /* (u)int*_t */

uint8_t mem_read8(uint16_t location);
uint16_t mem_read16(uint16_t location);
void mem_write8(uint16_t location, uint8_t data);
void mem_write16(uint16_t location, uint16_t data);

/* 8-bit address space */
#define MEM_SIZE	0x10000

enum offsets
{
	/* TODO GBC constants */

	/* restart and interrupt vectors */
	vec_begin = 0x0000,
	vec_end = 0x00FF,

	/* cartridge data */
	cart_begin = 0x0100,

	/* 4 bytes */
	start_instr_begin = 0x0100,
	start_instr_end = 0x0103,

	/* 48 bytes */
	graphic_begin = 0x0104,
	graphic_end = 0x0133,

	/* 10 bytes */
	title_begin = 0x0134,
	title_end = 0x013E,

	/* 4 bytes */
	publisher_begin = 0x013F,
	publisher_end = 0x0142,

	/* 1 byte */
	color_compat = 0x0143,

	/* 2 bytes */
	licensee_begin = 0x0144,
	licensee_end = 0x0145,

	/* 1 byte */
	sgb_compat = 0x0146,

	cart_type = 0x0147,

	cart_rom_size = 0x0148,

	cart_ram_size = 0x0149,

	cart_dest_code = 0x014A, /* Japanese or not */

	old_licensee = 0x014B, /* Mostly unused */

	mask_rom_version = 0x014C,

	header_checksum = 0x014D,

	/* 2 bytes */
	cart_checksum_begin = 0x014E,
	cart_checksum_end = 0x014F,

	cart_end = 0x014F,

	/* Fixed bank */
	rom0_begin = 0x0150,
	rom0_end = 0x3FFF,

	/* Switchable bank */
	rom_bank_begin = 0x4000,
	rom_bank_end = 0x7FFF,

	tile_ram_begin = 0x8000,
	tile_ram_end = 0x97FF,

	bg0_begin = 0x9800,
	bg0_end = 0x9BFF,

	bg1_begin = 0x9C00,
	bg1_end = 0x9FFF,

	/* Not present on all carts! */
	cart_ram_begin = 0xA000,
	cart_ram_end = 0xBFFF,

	ram_begin = 0xC000,
	ram_end = 0xDFFF,

	shadow_begin = 0xE000,
	shadow_end = 0xFDFF,

	oam_begin = 0xFE00,
	oam_end = 0xFE9F,

	/* Contains control registers and other doodads */
	mmio_begin = 0xFF00,
	/* TODO I/O constants */
	mmio_end = 0xFF7F,

	high_ram_begin = 0xFF80,
	high_ram_end = 0xFFFE,

	/* Interrupts locked out */
	int_flag = 0xFFFF,
};


typedef struct _cartridge_header
{
	/* NB: initial instructions excluded */
	uint8_t graphic[48];	/* 0x104-0x133 */
	union
	{
		/* new-style GBC header */
		struct
		{
			char title[11];		/* 0x134-0x143 */
			char publisher[4];	/* 0x13F-0x142 */
			uint8_t compat;		/* 0x143 CGB flag */
			char licensee_code[2];	/* 0x144-0x145 */
			uint8_t sgb;
		} gbc_title;
		struct
		{
			char title[18];
			uint8_t sgb;
		} sgb_title;
		char old_title[19];
	};

	uint8_t cart_type;	/* 0x147 */
	uint8_t rom_size;	/* 0x148 */
	uint8_t ram_size;	/* 0x149 Cartridge RAM */
	uint8_t dest_code;	/* 0x14A Non-Japan if true */
	char old_licensee;	/* 0x14B set to 0x33 if new code used */
	uint8_t mask_rom_version;	/* 0x14C almost always 0 */
	uint8_t header_checksum; 	/* 0x14D Enforced! */
	uint16_t cartridge_checksum;	/* 0x14E-0x14F Unenforced */
} cartridge_header;

#endif /*!__MEMORY_H_*/
