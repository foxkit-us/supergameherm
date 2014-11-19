project(sgherm)

add_executable(sgherm main.c ctl_unit.c lcdc.c memory.c print.c rom_read.c
	serio.c timer.c)
