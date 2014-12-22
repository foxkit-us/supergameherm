#ifndef __MBC_H__
#define __MBC_H__

#include "config.h"	// bool, *int*_t
#include "sgherm.h"	// emu_state
#include "rom_read.h"	// OFF_CART_TYPE

typedef enum
{
	CART_ROM_ONLY = 0x00,
        CART_MBC1 = 0x01,
        CART_MBC1_RAM = 0x02,
        CART_MBC1_RAM_BATT = 0x03,
        CART_MBC2 = 0x05,
        CART_MBC2_BATT = 0x06,
        CART_RAM = 0x08,
        CART_RAM_BATT = 0x09,
        CART_MMM01 = 0x0B,
        CART_MMM01_SRAM = 0x0C,
        CART_MMM01_SRAM_BATT = 0x0D,
        CART_MBC3_TIMER_BATT = 0x0F,
        CART_MBC3_TIMER_RAM_BATT = 0x10,
        CART_MBC3 = 0x11,
        CART_MBC3_RAM = 0x12,
        CART_MBC3_RAM_BATT = 0x13,
        CART_MBC5 = 0x19,
        CART_MBC5_RAM = 0x1A,
        CART_MBC5_RAM_BATT = 0x1B,
        CART_MBC5_RUMBLE = 0x1C,
        CART_MBC5_RUMBLE_SRAM = 0x1D,
        CART_MBC5_RUMBLE_SRAM_BATT = 0x1E,
        CART_CAMERA = 0x1F
} cart_types;

typedef struct mbc_common_data_t
{
	uint8_t ram_enable;
	bool rom_select;	//! ROM/RAM select switch (MBC1 only)

	//! TODO various MBC hardware stuff
} mbc_common_data;

typedef struct mbc3_data_t
{
	uint8_t ram_rtc_enable;	//! Enable reads/writes to RAM/RTC (A000-BFFF)
	uint8_t rtc_select;	//! RTC/RAM select switch

	//! RTC registers
	struct
	{
		uint8_t seconds;
		uint8_t minutes;
		uint8_t hours;
		struct
		{
#ifdef LITTLE_ENDIAN
			unsigned int day_carry:1;
			unsigned int halt:1;
			unsigned int unused:5;
			unsigned int days:9;
#else
			unsigned int days:9;
			unsigned int unused:5;
			unsigned int halt:1;
			unsigned int day_carry:1;
#endif
		};
	} rtc[2];

	uint8_t latched;	//! If 0x1, copy time into rtc[1] and read from that
} mbc3_data;

struct mbc_func_t
{
	bool (*cart_init)(emu_state *restrict);
	uint8_t (*cart_read)(emu_state *restrict, uint16_t);
	void (*cart_write)(emu_state *restrict, uint16_t, uint8_t);
	void (*cart_finish)(emu_state *restrict);
};

#define MBC_INIT(state) (state)->mbc.func->cart_init(state)
#define MBC_READ(state, location) (state)->mbc.func->cart_read(state, location)
#define MBC_WRITE(state, location, data) (state)->mbc.func->cart_write(state, location, data)
#define MBC_FINISH(state) (state)->mbc.func->cart_finish(state)

struct mbc_state_t
{
	const mbc_func *func;	//! MBC controlling functions

	cart_types cart;	//! Cartridge in use

	//! Cartridge RAM
	uint8_t cart_ram[0xF][0x2000];

	uint_fast16_t ram_bank;	//! Current RAM bank
	uint_fast16_t rom_bank;	//! Current ROM bank

	union
	{
		mbc_common_data mbc_common;
		mbc3_data mbc3;
	};
};


bool mbc_select(emu_state *);

#endif //__MBC_H__