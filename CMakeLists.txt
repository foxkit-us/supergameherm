project(sgherm)

set(CMAKE_C_FLAGS "-std=c99")

add_executable(sgherm main.c ctl_unit.c input.c lcdc.c memory.c print.c
	rom_read.c serio.c timer.c)
