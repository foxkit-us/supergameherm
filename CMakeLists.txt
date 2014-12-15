project(sgherm)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(TestBigEndian)

macro(platform_checks)
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

		add_definitions(-D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -D_XOPEN_VERSION=700)
	else()
		# Check for windows
		check_include_files(windows.h HAVE_WINDOWS)
	endif()

	test_big_endian(BIG_ENDIAN)
	if(NOT BIG_ENDIAN)
		set(LITTLE_ENDIAN TRUE)
	endif()

	# MSVC stuff
	check_include_files(stdbool.h HAVE_STDBOOL_H)
	check_include_files(stdint.h HAVE_STDINT_H)

	# Check for endianness related doodads
	check_include_files(endian.h HAVE_ENDIAN_H)
	check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
	if(NOT(HAVE_ENDIAN_H OR HAVE_SYS_ENDIAN_H))
		# Check for __bswap_XX
		check_function_exists(__bswap_16 HAVE_BSWAP_16)
		check_function_exists(__bswap_32 HAVE_BSWAP_32)
		if(NOT(HAVE_BSWAP_16 OR HAVE_BSWAP_32))
			check_function_exists(_byteswap_ushort
				HAVE_BYTESWAP_USHORT)
			check_function_exists(_byteswap_ulong
				HAVE_BYTESWAP_ULONG)
		endif()
	elseif(HAVE_POSIX)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_BSD_SOURCE")
	endif()
endmacro()

macro(compiler_checks)
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(HAVE_COMPILER_CLANG "1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wpedantic -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -g3")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g3")
		endif()
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(HAVE_COMPILER_GCC "1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -pedantic -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ggdb")
		endif()
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
		set(HAVE_COMPILER_INTEL "1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -g")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g")
		endif()
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(HAVE_COMPILER_MSVC "1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /G5 /Gr /D _CRT_SECURE_NO_WARNINGS")
		# /G5 - Enable Pentium optimisations
		# /Gr - __fastcall as deafult
		# /D _CRT_NO_SECURE_WARNINGS - disable whining about fopen and friends
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /W4
		/Ot /Oa /Gy")
		# /MTd - statically link to runtime library
		# /W4 - enable all warnings
		# /Ot, /Oa - still enable some performance improvements
		# /Gy - function-level linking (slightly faster)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zp4
		/MT /W3 /Zi /Ox /Ot /Oa /Og /Ob2 /GF")
		# /Zp4 - 4-byte struct alignment (~150k cycle/sec speed up on
		# P5 microarchitecture)
		# /MT - statically link to runtime library (larger file size,
		# but higher compatibility)
		# /W3 - enable most warnings
		# /Zi - still build debug symbol files (easier testing)
		# /Ox - "fuller" optimisation
		# /Ot - favour fast code over small code (~125k cycle/sec P5)
		# /Oa - assume no aliasing (restrict keyword)
		# /Og - perform global optimisations
		# /Ob2 - force inlining of functions
		# /GF - combine duplicate const strings into one (smaller file
		# size, and less data cache misses)
	else()
		message("I'm at a loss as to what compiler you have, hope it all works out.")
		set(HAVE_COMPILER_UNKNOWN "1")
	endif()
endmacro()

macro(libcaca_check)
	option(LIBCACA_ENABLE "Enable libcaca frontend" on)
	if(LIBCACA_ENABLE)
		find_package(libcaca REQUIRED)
		set(HAVE_LIBCACA 1)
		include_directories(${LIBCACA_INCLUDE_DIR})
	endif()
endmacro()

macro(sdl2_check)
	option(SDL2_ENABLE "Enable SDL2 frontend" on)
	if(SDL2_ENABLE)
		find_package(SDL2 REQUIRED)
		set(HAVE_SDL2 1)
		include_directories(${SDL2_INCLUDE_DIR})
	endif()
endmacro()

macro(library_checks)
	libcaca_check()
	sdl2_check()
endmacro()


platform_checks()
compiler_checks()
library_checks()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h")

add_library("sgherm-core" OBJECT src/sgherm.c src/ctl_unit.c src/input.c src/lcdc.c
	src/memory.c src/print.c src/rom_read.c src/serio.c src/sound.c
	src/timer.c src/debug.c src/signals.c src/util.c src/frontend.c)

# Add the SDL2 backend
if(SDL2_ENABLE)
	file(GLOB SDL2_FRONTEND_SOURCES src/frontends/sdl2/*.c)
	add_executable("sgherm-sdl2" ${SDL2_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)
	target_link_libraries("sgherm-sdl2" ${SDL2_LIBRARY} )
endif()

# Add the caca backend
if(LIBCACA_ENABLE)
	file(GLOB LIBCACA_FRONTEND_SOURCES src/frontends/caca/*.c)
	add_executable("sgherm-caca" ${LIBCACA_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)
	target_link_libraries("sgherm-caca" ${libcaca_LIBRARY})
endif()

# Add the Win32 backend
# XXX FIXME need a WIN32_ENABLE knob
if(HAVE_WINDOWS)
	file(GLOB W32_FRONTEND_SOURCES src/frontends/w32/*.c)
	add_executable("sgherm-w32" ${W32_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		# necessary to work around 'stdbool' and related stuff
		set_source_files_properties(src/*.c PROPERTIES LANGUAGE CXX)
		set_target_properties("sgherm-w32" PROPERTIES LINKER_LANGUAGE CXX)
	endif()
endif()

# Add the null backend
option(NULL_ENABLE "Enable a frontendless sgherm build (testing)" off)
if(NULL_ENABLE)
	file(GLOB NULL_FRONTEND_SOURCES src/frontends/null/*.c)
	add_executable("sgherm-null" ${BASE_SOURCES} ${NULL_FRONTEND_SOURCES})
endif()
