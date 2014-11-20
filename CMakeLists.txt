project(sgherm)

set(CMAKE_C_FLAGS "-std=gnu99")

add_executable(sgherm main.c ctl_unit.c lcdc.c memory.c print.c rom_read.c
	serio.c timer.c)
