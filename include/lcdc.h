#ifndef __LCDC_H_
#define __LCDC_H_

#include "config.h"	// macros, uint*_t
#include "typedefs.h"	// typedefs


struct oam_t
{
	uint8_t y;
	uint8_t x;
	uint8_t chr;			//! CHR code
	uint8_t priority;		//! Priority to obj/bg
	uint8_t vflip;			//! Vertical flip flag
	uint8_t hflip;			//! Horizontal flip
	uint8_t pal_dmg;		//! Palette selection (DMG only)
	uint8_t char_bank;		//! Character bank (CGB only)
	uint8_t pal_cgb;		//! Palette selection (CGB only)
};

struct cps_t
{
#ifdef LITTLE_ENDIAN
	unsigned pal_sel:1;		//! Palette changes on next write
	unsigned notused:1;		//! Not used
	unsigned pal_num:3;		//! Select palette number
	unsigned pal_data_num:2;	//! Select palette data number
	unsigned hl:1;			//! Specify H/L
#else
	unsigned hl:1;			//! Specify H/L
	unsigned pal_data_num:2;	//! Select palette data number
	unsigned pal_num:3;		//! Select palette number
	unsigned notused:1;		//! Not used
	unsigned pal_sel:1;		//! Palette changes on next write
#endif
};

struct lcdc_state_t
{
	uint_fast16_t curr_clk;		//! current clock
	uint_fast8_t curr_m3_clks;	//! number of mode 03 clocks for this LY
	uint_fast8_t curr_h_blk;	//! last H block to be written

	bool initial;			//! if the LCD has just turned on

	uint_fast8_t vram_bank;		//! Present VRAM bank
	uint8_t vram[0x2][0x2000];	//! VRAM banks (DMG only uses 1)

	uint8_t oam_ram[160];

	//! LCD control register
	uint8_t lcd_control;

	//! LCD stat
	uint8_t stat;

	//! CGB only (FIXME palette RAM)
	struct
	{
		uint8_t bcpi;
		uint8_t bcpd[64];	//! BG data write
		uint32_t bcpal[32];

		uint8_t ocpi;
		uint8_t ocpd[64];	//! OBJ data write
		uint32_t ocpal[32];

		uint16_t hsrc, hdst;
		uint8_t hlen;
	};

	uint_fast8_t scroll_y;	//! Y position in scrolling map
	uint_fast8_t scroll_x;	//! X position in scrolling map

	uint_fast8_t window_y;	//! Window Y coordinate (0 <= windowy <= 143)
	uint_fast8_t window_x;	//! Window X coordinate (7 <= windowx <= 166)

	uint_fast8_t ly;	//! Present line being transferred (144-153 = V-Blank)
	uint_fast8_t lyc;	//! LY comparison (set stat.lyc_state when == ly)

	bool throt_trigger; //! Trigger to allow throttling of vblank

	uint8_t bg_pal;		//! Background palette
	uint8_t obj_pal[2];	//! OAM palettes

	uint32_t out[144][160];	//! Simulated LCD screen buffer
};

#define LCDC_DMG_BG(state) ((state)->lcdc.lcd_control & 0x1)
#define LCDC_OBJ(state) ((state)->lcdc.lcd_control & 0x2)
#define LCDC_OBJ_SIZE(state) ((state)->lcdc.lcd_control & 0x4)
#define LCDC_BG_CODE_SEL(state) ((state)->lcdc.lcd_control & 0x8)
#define LCDC_BG_CHAR_SEL(state) ((state)->lcdc.lcd_control & 0x10)
#define LCDC_WIN(state) ((state)->lcdc.lcd_control & 0x20)
#define LCDC_WIN_CODE_SEL(state) ((state)->lcdc.lcd_control & 0x40)
#define LCDC_ENABLE(state) ((state)->lcdc.lcd_control & 0x80)

#define LCDC_STAT_MODE_FLAG(state) ((state)->lcdc.stat & 0x3)
#define LCDC_STAT_LYC_STATE(state) ((state)->lcdc.stat & 0x4)
#define LCDC_STAT_MODE0(state) ((state)->lcdc.stat & 0x8)
#define LCDC_STAT_MODE1(state) ((state)->lcdc.stat & 0x10)
#define LCDC_STAT_MODE2(state) ((state)->lcdc.stat & 0x20)
#define LCDC_STAT_LYC(state) ((state)->lcdc.stat & 0x40)



void init_lcdc(emu_state *restrict);
void lcdc_tick(emu_state *restrict, int count);

uint8_t lcdc_read(emu_state *restrict, uint16_t);
uint8_t vram_read(emu_state *restrict, uint16_t);
uint8_t lcdc_control_read(emu_state *restrict, uint16_t);
uint8_t lcdc_stat_read(emu_state *restrict, uint16_t);
uint8_t lcdc_scroll_read(emu_state *restrict, uint16_t);
uint8_t lcdc_ly_read(emu_state *restrict, uint16_t);
uint8_t lcdc_lyc_read(emu_state *restrict, uint16_t);
uint8_t lcdc_bgp_read(emu_state *restrict, uint16_t);
uint8_t lcdc_objp_read(emu_state *restrict, uint16_t);
uint8_t lcdc_window_read(emu_state *restrict, uint16_t);
uint8_t bg_pal_ind_read(emu_state *restrict, uint16_t);
uint8_t bg_pal_data_read(emu_state *restrict, uint16_t);
uint8_t sprite_pal_ind_read(emu_state *restrict, uint16_t);
uint8_t sprite_pal_data_read(emu_state *restrict, uint16_t);

void lcdc_write(emu_state *restrict, uint16_t, uint8_t);
void vram_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_control_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_stat_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_scroll_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_ly_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_lyc_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_bgp_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_objp_write(emu_state *restrict, uint16_t, uint8_t);
void lcdc_window_write(emu_state *restrict, uint16_t, uint8_t);
void bg_pal_ind_write(emu_state *restrict, uint16_t, uint8_t);
void bg_pal_data_write(emu_state *restrict, uint16_t, uint8_t);
void sprite_pal_ind_write(emu_state *restrict, uint16_t, uint8_t);
void sprite_pal_data_write(emu_state *restrict, uint16_t, uint8_t);
void hdma_reg_write(emu_state *restrict, uint16_t, uint8_t);
uint8_t hdma_status_read(emu_state *restrict state, uint16_t reg);

#endif /*!__LCDC_H_*/
