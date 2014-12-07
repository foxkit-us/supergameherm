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

		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -D_XOPEN_VERSION=700")
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
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wpedantic -Wall -Wextra")
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
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /G5 /Gr")
		# /G5 - Enable Pentium optimisations
		# /Gr - __fastcall as deafult
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
		set(SOURCES_ADDITIONAL ${SOURCES_ADDITIONAL} src/winmain.c)
	else()
		message("I'm at a loss as to what compiler you have, hope it all works out.")
		set(HAVE_COMPILER_UNKNOWN "1")
	endif()
endmacro()

macro(libcaca_check)
	option(LIBCACA_ENABLE "Enable libcaca frontend" on)
	if(LIBCACA_ENABLE)
		find_package(libcaca)
		if(LIBCACA_FOUND)
			set(HAVE_LIBCACA 1)
			set(SOURCES_ADDITIONAL ${SOURCES_ADDITIONAL} "src/caca_frontend.c")
			include_directories(${libcaca_INCLUDE_DIR})
			set(LIBS_ADDITIONAL ${LIBS_ADDITIONAL} ${libcaca_LIBRARY})
		endif()
	endif()
endmacro()

macro(library_checks)
	libcaca_check()
endmacro()


platform_checks()
compiler_checks()
library_checks()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h")

add_executable("sgherm" src/main.c src/ctl_unit.c src/input.c src/lcdc.c
	src/memory.c src/print.c src/rom_read.c src/serio.c src/sound.c src/timer.c 
	src/debug.c src/signals.c src/util.c src/frontend.c src/null_frontend.c ${SOURCES_ADDITIONAL})
target_link_libraries(sgherm ${LIBS_ADDITIONAL})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# necessary to work around 'stdbool' and related stuff
	set_source_files_properties(src/*.c PROPERTIES LANGUAGE CXX)
	set_target_properties("sgherm" PROPERTIES LINKER_LANGUAGE CXX)
endif()
