#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h>	/* (u)int*_t */
#include <stdbool.h>	/* bool */
#include <stdio.h>	/* FILE * */

#include "sgherm.h"	/* emulator_state */
#include "params.h"	/* system_types */

typedef enum
{
	/*! A list of constants, mostly used for documentation purposes. */
	/* TODO GBC constants */

	/* restart and interrupt vectors */
	OFF_VEC_BEGIN = 0x0000,
	OFF_VEC_END = 0x00FF,

	/* cart data */
	OFF_CART_BEGIN = 0x0100,

	/* 4 bytes */
	OFF_START_INSTR_BEGIN = 0x0100,
	OFF_START_INSTR_END = 0x0103,

	/* 48 bytes */
	OFF_GRAPHIC_BEGIN = 0x0104,
	OFF_GRAPHIC_END = 0x0133,

	/* 10 bytes */
	OFF_TITLE_BEGIN = 0x0134,
	OFF_TITLE_END = 0x013E,

	/* 4 bytes */
	OFF_MANUFACTURER_BEGIN = 0x013F,
	OFF_MANUFACTURER_END = 0x0142,

	/* 1 byte */
	OFF_COLOR_COMPAT = 0x0143,

	/* 2 bytes */
	OFF_LICENSEE_BEGIN = 0x0144,
	OFF_LICENSEE_END = 0x0145,

	/* 1 byte */
	OFF_SGB_COMPAT = 0x0146,

	OFF_CART_TYPE = 0x0147,

	OFF_ROM_SIZE = 0x0148,

	OFF_RAM_SIZE = 0x0149,

	OFF_DEST_CODE = 0x014A, /* Japanese or not */

	OFF_OLD_LICENSEE = 0x014B, /* Mostly unused */

	OFF_MASK_ROM_VERSION = 0x014C,

	OFF_HEADER_CHECKSUM = 0x014D,

	/* 2 bytes */
	OFF_CART_CHECKSUM_BEGIN = 0x014E,
	OFF_CART_CHECKSUM_END = 0x014F,

	OFF_CART_END = 0x014F,

	/* Fixed bank */
	OFF_ROM0_BEGIN = 0x0150,
	OFF_ROM0_END = 0x3FFF,

	/* Switchable bank */
	OFF_ROM_BANK_BEGIN = 0x4000,
	OFF_ROM_BANK_END = 0x7FFF,

	OFF_TILE_RAM_BEGIN = 0x8000,
	OFF_TILE_RAM_END = 0x97FF,

	OFF_BG0_BEGIN = 0x9800,
	OFF_BG0_END = 0x9BFF,

	OFF_BG1_BEGIN = 0x9C00,
	OFF_BG1_END = 0x9FFF,

	/* Not present on all carts! */
	OFF_CART_RAM_BEGIN = 0xA000,
	OFF_CART_RAM_END = 0xBFFF,

	OFF_RAM_BEGIN = 0xC000,
	OFF_RAM_END = 0xDFFF,

	OFF_SHADOW_BEGIN = 0xE000,
	OFF_SHADOW_END = 0xFDFF,

	OFF_OAM_BEGIN = 0xFE00,
	OFF_OAM_END = 0xFE9F,

	/* Contains control registers and other doodads */
	OFF_GBC_MMIO_BEGIN = 0xFF00,
	/* TODO more I/O constants */

	OFF_GBC_SWITCH_VIDEO_BANK = 0xFF40,

	OFF_GBC_MMIO_END = 0xFF7F,

	OFF_HIGH_RAM_BEGIN = 0xFF80,
	OFF_HIGH_RAM_END = 0xFFFE,

	/* Interrupts locked out */
	OFF_INT_FLAG = 0xFFFF,
} offsets;

typedef struct _cart_header
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
			char title[16];
			char licensee_code[2];
			uint8_t sgb;
		} sgb_title;

		char gb_title[19];
	};

	uint8_t cart_type;	/* 0x147 */
	uint8_t rom_size;	/* 0x148 */
	uint8_t ram_size;	/* 0x149 Cartridge RAM */
	uint8_t dest_code;	/* 0x14A Non-Japan if true */
	char old_licensee;	/* 0x14B set to 0x33 if new code used */
	uint8_t mask_rom_version;	/* 0x14C almost always 0 */
	uint8_t header_checksum; 	/* 0x14D Enforced! */
	uint16_t cart_checksum;	/* 0x14E-0x14F Unenforced */
} cart_header;


uint8_t mem_read8(emulator_state *state, uint16_t location);
uint16_t mem_read16(emulator_state *state, uint16_t location);
void mem_write8(emulator_state *state, uint16_t location, uint8_t data);
void mem_write16(emulator_state *state, uint16_t location, uint16_t data);

bool read_rom_data(emulator_state *state, FILE *rom, cart_header **header,
		system_types *type);

#endif /*!__MEMORY_H_*/
