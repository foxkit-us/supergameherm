#include "config.h"	// macros

#include "timer.h"	// get_time
#include "sgherm.h"	// emu_state, cpu_freq
#include "print.h"	// debug
#include "ctl_unit.h"	// flags
#include "debug.h"	// Prototypes etc
#include "memory.h"	// mem_read8

const char * const mnemonics[0x100] =
{
	"NOP", "LD BC,d16", "LD (BC),A", "INC BC",		// 0x00
	"INC B", "DEC B", "LD B,d8", "RLCA",			// 0x04
	"LD (a16),SP", "ADD HL,BC", "LD A,(BC)", "DEC BC",	// 0x08
	"INC C", "DEC C", "LD C,d8", "RRCA",			// 0x0C
	"STOP 0", "LD DE,d16", "LD (DE),A", "INC DE",		// 0x10
	"INC D", "DEC D", "LD D,d8", "RLA",			// 0x14
	"JR r8", "ADD HL,DE", "LD A,(DE)", "DEC DE",		// 0x18
	"INC E", "DEC E", "LD E,d8", "RRA",			// 0x1C
	"JR NZ,r8", "LD HL,d16", "LD (HL+),A", "INC HL",	// 0x20
	"INC H", "DEC H", "LD H,d8", "DAA",			// 0x24
	"JR Z,r8", "ADD HL,HL", "LD A,(HL+)", "DEC HL",		// 0x28
	"INC L", "DEC L", "LD L,d8", "CPL",			// 0x2C
	"JR NC,r8", "LD SP,d16", "LD (HL-),A", "INC SP",	// 0x30
	"INC (HL)", "DEC (HL)", "LD (HL),d8", "SCF",		// 0x34
	"JR C,r8", "ADD HL,SP", "LD A,(HL-)", "DEC SP",		// 0x38
	"INC A", "DEC A", "LD A,d8", "CCF",			// 0x3C
	"LD B,B", "LD B,C", "LD B,D", "LD B,E",			// 0x40
	"LD B,H", "LD B,L", "LD B,(HL)", "LD B,A",		// 0x44
	"LD C,B", "LD C,C", "LD C,D", "LD C,E",			// 0x48
	"LD C,H", "LD C,L", "LD C,(HL)", "LD C,A",		// 0x4C
	"LD D,B", "LD D,C", "LD D,D", "LD D,E",			// 0x50
	"LD D,H", "LD D,L", "LD D,(HL)", "LD D,A",		// 0x54
	"LD E,B", "LD E,C", "LD E,D", "LD E,E",			// 0x58
	"LD E,H", "LD E,L", "LD E,(HL)", "LD E,A",		// 0x5C
	"LD H,B", "LD H,C", "LD H,D", "LD H,E",			// 0x60
	"LD H,H", "LD H,L", "LD H,(HL)", "LD H,A",		// 0x64
	"LD L,B", "LD L,C", "LD L,D", "LD L,E",			// 0x68
	"LD L,H", "LD L,L", "LD L,(HL)", "LD L,A",		// 0x6C
	"LD (HL),B", "LD (HL),C", "LD (HL),D", "LD (HL),E",	// 0x70
	"LD (HL),H", "LD (HL),L", "HALT", "LD (HL),A",		// 0x74
	"LD A,B", "LD A,C", "LD A,D", "LD A,E",			// 0x78
	"LD A,H", "LD A,L", "LD A,(HL)", "LD A,A",		// 0x7C
	"ADD A,B", "ADD A,C", "ADD A,D", "ADD A,E",		// 0x80
	"ADD A,H", "ADD A,L", "ADD A,(HL)", "ADD A,A",		// 0x84
	"ADC A,B", "ADC A,C", "ADC A,D", "ADC A,E",		// 0x88
	"ADC A,H", "ADC A,L", "ADC A,(HL)", "ADC A,A",		// 0x8C
	"SUB B", "SUB C", "SUB D", "SUB E",			// 0x90
	"SUB H", "SUB L", "SUB (HL)", "SUB A",			// 0x94
	"SBC A,B", "SBC A,C", "SBC A,D", "SBC A,E",		// 0x98
	"SBC A,H", "SBC A,L", "SBC A,(HL)", "SBC A,A",		// 0x9C
	"AND B", "AND C", "AND D", "AND E",			// 0xA0
	"AND H", "AND L", "AND (HL)", "AND A",			// 0xA4
	"XOR B", "XOR C", "XOR D", "XOR E",			// 0xA8
	"XOR H", "XOR L", "XOR (HL)", "XOR A",			// 0xAC
	"OR B", "OR C", "OR D", "OR E",				// 0xB0
	"OR H", "OR L", "OR (HL)", "OR A",			// 0xB4
	"CP B", "CP C", "CP D", "CP E",				// 0xB8
	"CP H", "CP L", "CP (HL)", "CP A",			// 0xBC
	"RET NZ", "POP BC", "JP NZ,a16", "JP a16",		// 0xC0
	"CALL NZ,a16", "PUSH BC", "ADD A,d8", "RST 00H",	// 0xC4
	"RET Z", "RET", "JP Z,a16", "PREFIX CB",		// 0xC8
	"CALL Z,a16", "CALL a16", "ADC A,d8", "RST 08H",	// 0xCC
	"RET NC", "POP DE", "JP NC,a16", "",			// 0xD0
	"CALL NC,a16", "PUSH DE", "SUB d8", "RST 10H",		// 0xD4
	"RET C", "RETI", "JP C,a16", "",			// 0xD8
	"CALL C,a16", "", "SBC A,d8", "RST 18H",		// 0xDC
	"LDH (a8),A", "POP HL", "LD (C),A", "",			// 0xE0
	"", "PUSH HL", "AND d8", "RST 20H",			// 0xE4
	"ADD SP,r8", "JP (HL)", "LD (a16),A", "",		// 0xE8
	"", "", "XOR d8", "RST 28H",				// 0xEC
	"LDH A,(a8)", "POP AF", "LD A,(C)", "DI",		// 0xF0
	"", "PUSH AF", "OR d8", "RST 30H",			// 0xF4
	"LD HL,SP+r8", "LD SP,HL", "LD A,(a16)", "EI",		// 0xF8
	"", "", "CP d8", "RST 38H",				// 0xFC
};

/*! Bit length of given instructions */
const int instr_len[0x100] =
{
	1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,		// 0x00
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,		// 0x10
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,		// 0x20
	2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,		// 0x30
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x40
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x50
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x60
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x70
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x80
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0x90
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0xA0
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 0xB0
	1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,		// 0xC0
	1, 1, 3,-1, 3, 1, 2, 1, 1, 1, 3,-1, 3,-1, 2, 1,		// 0xD0
	2, 1, 2,-1,-1, 1, 2, 1, 2, 1, 3,-1,-1,-1, 2, 1,		// 0xE0
	2, 1, 2, 1,-1, 1, 2, 1, 2, 1, 3, 1,-1,-1, 2, 1,		// 0xF0
};

const char * const flags_expect[0x100] =
{
	"----", "----", "----", "----",	// 0x0
	"Z0H-", "Z1H-", "----", "000C",	// 0x4
	"----", "-0HC", "----", "----",	// 0x8
	"Z0H-", "Z1H-", "----", "000C",	// 0xC
	"----", "----", "----", "----",	// 0x10
	"Z0H-", "Z1H-", "----", "000C",	// 0x14
	"----", "-0HC", "----", "----",	// 0x18
	"Z0H-", "Z1H-", "----", "000C",	// 0x1C
	"----", "----", "----", "----",	// 0x20
	"Z0H-", "Z1H-", "----", "Z-0C",	// 0x24
	"----", "-0HC", "----", "----",	// 0x28
	"Z0H-", "Z1H-", "----", "-11-",	// 0x2C
	"----", "----", "----", "----",	// 0x30
	"Z0H-", "Z1H-", "----", "-001",	// 0x34
	"----", "-0HC", "----", "----",	// 0x38
	"Z0H-", "Z1H-", "----", "-00C",	// 0x3C
	"----", "----", "----", "----",	// 0x40
	"----", "----", "----", "----",	// 0x44
	"----", "----", "----", "----",	// 0x48
	"----", "----", "----", "----",	// 0x4C
	"----", "----", "----", "----",	// 0x50
	"----", "----", "----", "----",	// 0x54
	"----", "----", "----", "----",	// 0x58
	"----", "----", "----", "----",	// 0x5C
	"----", "----", "----", "----",	// 0x60
	"----", "----", "----", "----",	// 0x64
	"----", "----", "----", "----",	// 0x68
	"----", "----", "----", "----",	// 0x6C
	"----", "----", "----", "----",	// 0x70
	"----", "----", "----", "----",	// 0x74
	"----", "----", "----", "----",	// 0x78
	"----", "----", "----", "----",	// 0x7C
	"Z0HC", "Z0HC", "Z0HC", "Z0HC",	// 0x80
	"Z0HC", "Z0HC", "Z0HC", "Z0HC",	// 0x84
	"Z0HC", "Z0HC", "Z0HC", "Z0HC",	// 0x88
	"Z0HC", "Z0HC", "Z0HC", "Z0HC",	// 0x8C
	"Z1HC", "Z1HC", "Z1HC", "Z1HC",	// 0x90
	"Z1HC", "Z1HC", "Z1HC", "Z1HC",	// 0x94
	"Z1HC", "Z1HC", "Z1HC", "Z1HC",	// 0x98
	"Z1HC", "Z1HC", "Z1HC", "Z1HC",	// 0x9C
	"Z010", "Z010", "Z010", "Z010",	// 0xA0
	"Z010", "Z010", "Z010", "Z010",	// 0xA4
	"Z000", "Z000", "Z000", "Z000",	// 0xA8
	"Z000", "Z000", "Z000", "Z000",	// 0xAC
	"Z000", "Z000", "Z000", "Z000",	// 0xB0
	"Z000", "Z000", "Z000", "Z000",	// 0xB4
	"Z1HC", "Z1HC", "Z1HC", "Z1HC",	// 0xB8
	"Z1HC", "Z1HC", "Z1HC", "Z1HC",	// 0xBC
	"----", "----", "----", "----",	// 0xC0
	"----", "----", "Z0HC", "----",	// 0xC4
	"----", "----", "----", "----",	// 0xC8
	"----", "----", "Z0HC", "----",	// 0xCC
	"----", "----", "----", "NULL",	// 0xD0
	"----", "----", "Z1HC", "----",	// 0xD4
	"----", "----", "----", "NULL",	// 0xD8
	"----", "NULL", "Z1HC", "----",	// 0xDC
	"----", "----", "----", "NULL",	// 0xE0
	"NULL", "----", "Z010", "----",	// 0xE4
	"00HC", "----", "----", "NULL",	// 0xE8
	"NULL", "NULL", "Z000", "----",	// 0xEC
	"----", "ZNHC", "----", "----",	// 0xF0
	"NULL", "----", "Z000", "----",	// 0xF4
	"00HC", "----", "----", "----",	// 0xF8
	"NULL", "NULL", "Z1HC", "----",	// 0xFC
};

const char * const flags_cb_expect[0x100] =
{
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x0
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x4
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x8
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0xC
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x10
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x14
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x18
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x1C
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x20
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x24
	"Z000", "Z000", "Z000", "Z000",	// 0x28
	"Z000", "Z000", "Z000", "Z000",	// 0x2C
	"Z000", "Z000", "Z000", "Z000",	// 0x30
	"Z000", "Z000", "Z000", "Z000",	// 0x34
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x38
	"Z00C", "Z00C", "Z00C", "Z00C",	// 0x3C
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x40
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x44
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x48
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x4C
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x50
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x54
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x58
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x5C
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x60
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x64
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x68
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x6C
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x70
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x74
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x78
	"Z01-", "Z01-", "Z01-", "Z01-",	// 0x7C
	"----", "----", "----", "----",	// 0x80
	"----", "----", "----", "----",	// 0x84
	"----", "----", "----", "----",	// 0x88
	"----", "----", "----", "----",	// 0x8C
	"----", "----", "----", "----",	// 0x90
	"----", "----", "----", "----",	// 0x94
	"----", "----", "----", "----",	// 0x98
	"----", "----", "----", "----",	// 0x9C
	"----", "----", "----", "----",	// 0xA0
	"----", "----", "----", "----",	// 0xA4
	"----", "----", "----", "----",	// 0xA8
	"----", "----", "----", "----",	// 0xAC
	"----", "----", "----", "----",	// 0xB0
	"----", "----", "----", "----",	// 0xB4
	"----", "----", "----", "----",	// 0xB8
	"----", "----", "----", "----",	// 0xBC
	"----", "----", "----", "----",	// 0xC0
	"----", "----", "----", "----",	// 0xC4
	"----", "----", "----", "----",	// 0xC8
	"----", "----", "----", "----",	// 0xCC
	"----", "----", "----", "----",	// 0xD0
	"----", "----", "----", "----",	// 0xD4
	"----", "----", "----", "----",	// 0xD8
	"----", "----", "----", "----",	// 0xDC
	"----", "----", "----", "----",	// 0xE0
	"----", "----", "----", "----",	// 0xE4
	"----", "----", "----", "----",	// 0xE8
	"----", "----", "----", "----",	// 0xEC
	"----", "----", "----", "----",	// 0xF0
	"----", "----", "----", "----",	// 0xF4
	"----", "----", "----", "----",	// 0xF8
	"----", "----", "----", "----",	// 0xFC
};

void print_cpu_state(emu_state *restrict state)
{
	debug("[%X] (af bc de hl sp %X %X %X %X %X)", REG_PC(state),
		REG_AF(state), REG_BC(state), REG_DE(state),
		REG_HL(state), REG_SP(state));
}

void print_cycles(emu_state *restrict state)
{
	uint64_t finish = get_time();
	double taken = (finish - state->start_time) / 1e9;
	double cps = state->cycles / taken;
	const cpu_freq freq_dmg = CPU_FREQ_DMG, freq_cgb = CPU_FREQ_CGB;

	info("Time taken: %.3f seconds", taken);
	info("Cycle count: %ld", state->cycles);
	info("Cycles per second: %.3f (%.3fx GB, %.3fx GBC)", cps,
			cps / freq_dmg, cps / freq_cgb);
}

void print_flags(emu_state *restrict state)
{
	debug("flags = %s%s%s%s",
		IS_FLAG(state, FLAG_Z) ? "Z":"z",
		IS_FLAG(state, FLAG_N) ? "N":"n",
		IS_FLAG(state, FLAG_H) ? "H":"h",
		IS_FLAG(state, FLAG_C) ? "C":"c");
}

void dump_all_state(emu_state *restrict state)
{
	debug("pc=%04X\tsp=%04X\tbk=%04X\n",
		   REG_PC(state),
	REG_SP(state),
	state->bank);
	debug("af=%04X\tbc=%04X\tde=%04X\thl=%04X\n",
		   REG_AF(state),
	REG_BC(state),
	REG_DE(state),
	REG_HL(state));
	debug("interrupts are %s\n", (state->int_state.enabled ? "ENABLED" : "DISABLED"));
	print_flags(state);
	debug("bytes at pc: %02X %02X %02X %02X\n",
		mem_read8(state, REG_PC(state)),
		mem_read8(state, REG_PC(state)+1),
		mem_read8(state, REG_PC(state)+2),
		mem_read8(state, REG_PC(state)+3));
	debug("bytes at sp: %02X %02X %02X %02X\n",
		mem_read8(state, REG_SP(state)),
		mem_read8(state, REG_SP(state)+1),
		mem_read8(state, REG_SP(state)+2),
		mem_read8(state, REG_SP(state)+3));
}
