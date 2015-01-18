static const uint8_t cgb_ramp[32] = {
	// TODO: get actual ramping
	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
	0x78, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0,
	0xB8, 0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0,
	0xF2, 0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE, 0xFF,
};

static inline uint8_t lcdc_read(emu_state *restrict state, uint16_t reg)
{
	error(state, "lcdc: unknown register %04X (R)", reg);
	return 0xFF;
}

static inline uint8_t lcdc_control_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.lcd_control;
}

static inline uint8_t lcdc_stat_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.stat;
}

static inline uint8_t lcdc_scroll_read(emu_state *restrict state, uint16_t reg)
{
	if(reg == 0xFF42)
	{
		return state->lcdc.scroll_y;
	}
	else if(reg == 0xFF43)
	{
		return state->lcdc.scroll_x;
	}
	else
	{
		fatal(state, "BUG: attempt to read scroll stuff from non-scroll reg");
		return 0xFF;
	}
}

static inline uint8_t lcdc_ly_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.ly;
}

static inline uint8_t lcdc_lyc_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.lyc;
}

static inline uint8_t lcdc_bgp_read(emu_state *restrict state, uint16_t reg UNUSED)
{
	return state->lcdc.bg_pal;
}

static inline uint8_t lcdc_objp_read(emu_state *restrict state, uint16_t reg)
{
	return state->lcdc.obj_pal[reg & 0xB7];
}

static inline uint8_t lcdc_window_read(emu_state *restrict state, uint16_t reg)
{
	if(reg == 0xFF4A)
	{
		return state->lcdc.window_y;
	}
	else if(reg == 0xFF4B)
	{
		return state->lcdc.window_x;
	}
	else
	{
		fatal(state, "BUG: Attempt to read window stuff from non-window reg");
		return 0xFF;
	}
}

static inline uint8_t bg_pal_ind_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hw_read(state, reg);
	}

	// TODO
	return 0xFF;
}

static inline uint8_t bg_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hw_read(state, reg);
	}

	// TODO
	return 0xFF;
}

static inline uint8_t sprite_pal_ind_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hw_read(state, reg);
	}

	// TODO
	return 0xFF;
}

static inline uint8_t sprite_pal_data_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hw_read(state, reg);
	}

	// TODO
	return 0xFF;
}

static inline void lcdc_write(emu_state *restrict state, uint16_t reg, uint8_t data UNUSED)
{
	error(state, "lcdc: unknown register %04X (W)", reg);
}

static inline void lcdc_control_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	bool is_off = (LCDC_ENABLE(state)) == 0;

	state->lcdc.lcd_control = data;

	if(is_off && LCDC_ENABLE(state))
	{
		// Restart LY clock
		state->lcdc.ly = 0;
		state->lcdc.curr_clk = 0;
		state->lcdc.initial = true;
		lcdc_mode_change(state, 0);
	}
	else if(!is_off && !LCDC_ENABLE(state))
	{
		// hardware sets mode to 1 when disabling
		// source: http://www.codeslinger.co.uk/pages/projects/gameboy/lcd.html
		// source: https://code.google.com/p/megaboy/wiki/Interrupts
		state->lcdc.ly = 0;
		state->lcdc.stat = (state->lcdc.stat & ~0x3) | 1;
	}
}

static inline void lcdc_stat_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	/* don't overwrite mode bits */
	state->lcdc.stat = (data & 0x78) | LCDC_STAT_MODE_FLAG(state);
}

static inline void lcdc_scroll_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg == 0xFF42)
	{
		state->lcdc.scroll_y = data;
	}
	else if(reg == 0xFF43)
	{
		state->lcdc.scroll_x = data;
	}
	else
	{
		fatal(state, "BUG: attempt to write scroll stuff to non-scroll reg");
	}
}

static inline void lcdc_ly_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data UNUSED)
{
#ifndef NDEBUG
	fatal(state, "write to LY (FF44); you can't just vsync yourself!");
#else
	error(state, "write to LY (FF44) is being ignored");
#endif //!NDEBUG
}

static inline void lcdc_lyc_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->lcdc.lyc = data;
}

static inline void lcdc_bgp_write(emu_state *restrict state, uint16_t reg UNUSED, uint8_t data)
{
	state->lcdc.bg_pal = data;
}

static inline void lcdc_objp_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg == 0xFF48)
	{
		state->lcdc.obj_pal[0] = data;
	}
	else if(reg == 0xFF49)
	{
		state->lcdc.obj_pal[1] = data;
	}
	else
	{
		fatal(state, "BUG: Attempt to write OBP data to non-OBP register");
	}
}

static inline void lcdc_window_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(reg == 0xFF4A)
	{
		state->lcdc.window_y = data;
	}
	else if(reg == 0xFF4B)
	{
		state->lcdc.window_x = data;
	}
	else
	{
		fatal(state, "BUG: Attempt to write window data to non-window register");
	}
}

static inline void bg_pal_ind_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		no_hw_write(state, reg, data);
		return;
	}

	state->lcdc.bcpi = data & (0x80|0x3F);
}

static void bg_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		no_hw_write(state, reg, data);
		return;
	}

	state->lcdc.bcpd[state->lcdc.bcpi&0x3F] = data;

	int idx = (state->lcdc.bcpi>>1) & 0x1F;
	uint8_t c0 = state->lcdc.bcpd[(idx<<1)+0];
	uint8_t c1 = state->lcdc.bcpd[(idx<<1)+1];
	uint32_t r = c0 & 0x1F;
	uint32_t g = ((c0>>5) | (c1<<3)) & 0x1F;
	uint32_t b = (c1>>2) & 0x1F;
	r = cgb_ramp[r];
	g = cgb_ramp[g];
	b = cgb_ramp[b];
	r <<= 16;
	g <<= 8;
	state->lcdc.bcpal[idx] = (r|g|b);
	//printf("pal %08X %08X\n", idx, (r|g|b));

	state->lcdc.bcpi += (state->lcdc.bcpi>>7);
	state->lcdc.bcpi &= (0x80|0x3F);
}

static inline void sprite_pal_ind_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		no_hw_write(state, reg, data);
		return;
	}

	state->lcdc.ocpi = data & (0x80|0x3F);
}

static inline void sprite_pal_data_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		no_hw_write(state, reg, data);
		return;
	}

	state->lcdc.ocpd[state->lcdc.ocpi&0x3F] = data;

	int idx = (state->lcdc.ocpi>>1) & 0x1F;
	uint8_t c0 = state->lcdc.ocpd[(idx<<1)+0];
	uint8_t c1 = state->lcdc.ocpd[(idx<<1)+1];
	uint32_t r = c0 & 0x1F;
	uint32_t g = ((c0>>5) | (c1<<3)) & 0x1F;
	uint32_t b = (c1>>2) & 0x1F;
	r = cgb_ramp[r];
	g = cgb_ramp[g];
	b = cgb_ramp[b];
	r <<= 16;
	g <<= 8;
	state->lcdc.ocpal[idx] = (r|g|b);
	//printf("pal %08X %08X\n", idx, (r|g|b));

	state->lcdc.ocpi += (state->lcdc.ocpi>>7);
	state->lcdc.ocpi &= (0x80|0x3F);
}

static inline void hdma_reg_write(emu_state *restrict state, uint16_t reg, uint8_t data)
{
	int i;

	reg &= 0xFF;

	switch(reg)
	{
		case 0x51: // Source High
			state->lcdc.hsrc = (state->lcdc.hsrc & 0x00F0)
				| (((uint16_t)data)<<8);
			break;

		case 0x52: // Source Low
			state->lcdc.hsrc = (state->lcdc.hsrc & 0xFF00)
				| (((uint16_t)data));
			break;

		case 0x53: // Dest High
			state->lcdc.hdst = (state->lcdc.hdst & 0x00F0)
				| (((uint16_t)data)<<8);
			break;

		case 0x54: // Dest Low
			state->lcdc.hdst = (state->lcdc.hdst & 0xFF00)
				| (((uint16_t)data));
			break;

		case 0x55: // Start transfer
			// TODO: do this properly!
			// TODO: clamp/verify addresses correctly
			for(i = 0; i < 16*((data&0x7F)+1); i++)
			{
				if(true
					&& ((state->lcdc.hsrc >= 0x0000 && state->lcdc.hsrc <= 0x7FFF)
					|| (state->lcdc.hsrc >= 0xA000 && state->lcdc.hsrc <= 0xDFFF))
					&& (state->lcdc.hdst >= 0x8000 && state->lcdc.hdst <= 0x9FFF))
				{
					mem_write8(state, state->lcdc.hdst,
						mem_read8(state, state->lcdc.hsrc));
				}

				state->lcdc.hsrc++;
				state->lcdc.hdst++;
			}

			break;

		default:
			error(state, "[%4X] EDOOFUS: register %02X is not an HDMA register!\n",
				REG_PC(state), reg);
			break;
	}
}

static inline uint8_t hdma_status_read(emu_state *restrict state, uint16_t reg)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hw_read(state, reg);
	}

	// TODO
	return 0xFF;
}

static inline uint8_t vram_bank_switch_read(emu_state *restrict state, uint16_t location UNUSED)
{
	if(state->system != SYSTEM_CGB)
	{
		return no_hw_read(state, location);
	}

	return state->lcdc.vram_bank;
}

static inline void vram_bank_switch_write(emu_state *restrict state, uint16_t location, uint8_t data)
{
	if(state->system != SYSTEM_CGB)
	{
		no_hw_write(state, location, data);
		return;
	}

	state->lcdc.vram_bank = data & 1;
}

static inline void dma_write(emu_state *restrict state, uint16_t location UNUSED, uint8_t data)
{
	// TODO FIXME XXX OMG HAX
	/* this is 'correct' but horribly inaccurate:
	 * this transfer should take 160 µs (640 clocks), and during the
	 * transfer, the CPU locks all memory reads except FE80-FFFE.
	 *
	 * NOTE
	 * The above may not be true anymore, but I'm not sure I implemented
	 * this correctly. --Elizabeth
	 */
	uint16_t addr = data << 8;
	int curr = 0;

	for (; curr < 160; curr++, addr++)
	{
		state->lcdc.oam_ram[curr] = mem_read8(state, addr);
	}

	state->dma_wait = 640;
}
