project(sgherm)

cmake_minimum_required(VERSION 2.8)

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(TestBigEndian)

check_include_files(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
	check_symbol_exists(_POSIX_VERSION unistd.h HAVE_POSIX)
endif()

if(HAVE_POSIX)
	check_symbol_exists(clock_gettime time.h HAVE_CLOCK_GETTIME)
endif()

test_big_endian(BIG_ENDIAN)
if(NOT BIG_ENDIAN)
	set(LITTLE_ENDIAN TRUE)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ggdb")
	#set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg -ggdb")
	#set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -pg")
	#set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} -pg")
	#set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} -pg")
	#set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} -pg")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	message("idk what flags to use...")
else()
	message("I'm at a loss as to what to do, hope it all works out.")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)


add_executable(sgherm main.c ctl_unit.c input.c lcdc.c memory.c print.c
	rom_read.c serio.c timer.c debug.c)
