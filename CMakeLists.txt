project(sgherm)

cmake_minimum_required(VERSION 2.8)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(TestBigEndian)

check_include_files(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
	check_symbol_exists(_POSIX_VERSION unistd.h HAVE_POSIX)
endif()

if(HAVE_POSIX)
	check_symbol_exists(clock_gettime time.h HAVE_CLOCK_GETTIME)
	if(NOT HAVE_CLOCK_GETTIME)
		# OS X check
		check_include_files("mach/mach.h;mach/clock.h" HAVE_MACH_CLOCK_H)
	endif()
endif()

test_big_endian(BIG_ENDIAN)
if(NOT BIG_ENDIAN)
	set(LITTLE_ENDIAN TRUE)
endif()

# Check for endianness related doodads
check_include_files(endian.h HAVE_ENDIAN_H)
check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
if(NOT(HAVE_ENDIAN_H OR HAVE_SYS_ENDIAN_H))
	# Check for __bswap_XX
	check_function_exists(__bswap_16 HAVE_BSWAP_16)
	check_function_exists(__bswap_32 HAVE_BSWAP_32)
endif()


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ggdb")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	message("idk what flags to use...")
else()
	message("I'm at a loss as to what to do, hope it all works out.")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)


add_executable(sgherm main.c ctl_unit.c input.c lcdc.c memory.c print.c
	rom_read.c serio.c sound.c timer.c debug.c signals.c)
