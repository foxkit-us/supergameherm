project(sgherm)

cmake_minimum_required(VERSION 2.8.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(platform)
include(cflags)
include(frontend)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_BINARY_DIR}")

option(THROTTLE_VBLANK "Enable throttling of vblank" OFF)

set_cflags()
platform_checks()

# Doodads for the core
set(CORE_FILES src/sgherm.c src/ctl_unit.c src/input.c src/lcdc.c src/memory.c
	src/mbc.c src/mmap.c src/print.c src/rom.c src/serio.c src/sound.c
	src/timer.c src/debug.c src/signals.c src/util.c src/frontend.c)
add_library("sgherm-core" OBJECT ${CORE_FILES})

# Do the frontend checks
frontend_checks()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in" "${CMAKE_BINARY_DIR}/config.h")

if(THROTTLE_VBLANK)
	add_definitions("-DTHROTTLE_VBLANK")
endif()
