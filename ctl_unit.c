#include <stdbool.h>	// bool
#include <stdint.h>	// integer types
#include <stdlib.h>	// NULL

#include "print.h"	// fatal


/*! usable memory */
char memory[0x10000];

/*! actual loaded ROM data */
extern char *data;

/*! registers */
uint16_t af, bc, de, hl, sp, pc;

char *a = ((char *)&af) + 1;
char *f = (char *)&af;
char *b = ((char *)&bc) + 1;
char *c = (char *)&bc;
char *d = ((char *)&de) + 1;
char *e = (char *)&de;
char *h = ((char *)&hl) + 1;
char *l = (char *)&hl;

char flag_reg;

/*! Zero Flag */
#define FLAG_Z 0x80
/*! Subtract Flag */
#define FLAG_N 0x40
/*! Half-Carry Flag */
#define FLAG_H 0x20
/*! Carry Flag */
#define FLAG_C 0x10

typedef void (*opcode_t)(void);

opcode_t handlers[0x100] = {
	/* 0x00 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x08 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x10 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x18 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x20 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x28 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x30 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x38 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x40 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x48 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x50 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x58 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x60 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x68 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x70 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x78 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x80 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x88 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x90 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x98 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xA0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xA8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xB0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xB8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xC0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xC8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xD0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xD8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xE0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xE8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xF0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0xF8 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};



/*! the emulated CU for the 'z80-ish' CPU */
bool execute(void)
{
	char opcode = memory[pc];
	
	if(handlers[opcode] == NULL)
	{
		fatal("invalid opcode %d", opcode);
	}
	
	return false;
}