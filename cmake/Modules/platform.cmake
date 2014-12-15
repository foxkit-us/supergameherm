include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(TestBigEndian)

macro(posix_check)
	check_include_files(unistd.h HAVE_UNISTD_H)
	if(HAVE_UNISTD_H)
		check_symbol_exists(_POSIX_VERSION unistd.h HAVE_POSIX)
	endif()
endmacro()

macro(windows_check)
	check_include_files(windows.h HAVE_WINDOWS)
endmacro()

macro(stdc_check)
	check_include_files(stdbool.h HAVE_STDBOOL_H)
	check_include_files(stdint.h HAVE_STDINT_H)
endmacro()

macro(swap_check)
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
	else()
		add_definitions(-D_BSD_SOURCE)
	endif()
endmacro()

macro(clock_check)
	if(HAVE_POSIX)
		add_definitions(-D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -D_XOPEN_VERSION=700)
		check_symbol_exists(clock_gettime time.h HAVE_CLOCK_GETTIME)

		if(NOT HAVE_CLOCK_GETTIME)
			# OS X check
			check_include_files("mach/mach.h;mach/clock.h" HAVE_MACH_CLOCK_H)
		endif()
	endif()
endmacro()

macro(platform_checks)
	posix_check()
	if(NOT HAVE_POSIX)
		windows_check()
	endif()

	test_big_endian(BIG_ENDIAN)
	if(NOT BIG_ENDIAN)
		set(LITTLE_ENDIAN 1)
	endif()

	stdc_check()
	swap_check()
	clock_check()
endmacro()
