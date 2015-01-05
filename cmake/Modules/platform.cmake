include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(TestBigEndian)

function(posix_check)
	check_include_files(unistd.h HAVE_UNISTD_H)
	if(HAVE_UNISTD_H)
		check_symbol_exists(_POSIX_VERSION unistd.h HAVE_POSIX)
	endif()

	set(HAVE_POSIX ${HAVE_POSIX}
		CACHE INTERNAL "Operating system is POSIX-compatible")
	set(HAVE_UNISTD_H ${HAVE_UNISTD_H}
		CACHE INTERNAL "Have unistd.h header")
endfunction(posix_check)

function(windows_check)
	check_include_files(windows.h HAVE_WINDOWS)

	set(HAVE_WINDOWS ${HAVE_WINDOWS}
		CACHE INTERNAL "Operating system is Windows")
endfunction(windows_check)

function(stdc_check)
	check_include_files(stdbool.h HAVE_STDBOOL_H)
	check_include_files(stdint.h HAVE_STDINT_H)

	set(HAVE_STDBOOL_H ${HAVE_STDBOOL_H}
		CACHE INTERNAL "Have stdbool.h (C99 and later)")
	set(HAVE_STDINT_H ${HAVE_STDINT_H}
		CACHE INTERNAL "Have stdint.h (C99 and later)")
endfunction(stdc_check)

function(swap_check)
	check_include_files(endian.h HAVE_ENDIAN_H)
	check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)

	if(HAVE_ENDIAN_H)
		# This is sufficient
		LIST(APPEND CMAKE_REQUIRED_DEFINITIONS "-D_BSD_SOURCE")
		check_symbol_exists(be16toh endian.h HAVE_BSD_ENDIAN)
	elseif(HAVE_SYS_ENDIAN_H)
		LIST(APPEND CMAKE_REQUIRED_DEFINITIONS "-D_BSD_SOURCE")
		check_symbol_exists(be16toh sys/endian.h HAVE_BSD_ENDIAN)
	endif()

	if(NOT HAVE_BSD_ENDIAN)
		unset(HAVE_BSD_ENDIAN)
		unset(HAVE_ENDIAN_H)
		unset(HAVE_SYS_ENDIAN_H)

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

	set(HAVE_ENDIAN_H ${HAVE_ENDIAN_H}
		CACHE INTERNAL "Platform has endian.h swap builtins")
	set(HAVE_SYS_ENDIAN_H ${HAVE_SYS_ENDIAN_H}
		CACHE INTERNAL "Platform has sys/endian.h swap builtins")
	set(HAVE_BSWAP_16 ${HAVE_BSWAP_16}
		CACHE INTERNAL "Platform has __bswap_16 swap builtin")
	set(HAVE_BSWAP_32  ${HAVE_BSWAP_32}
		CACHE INTERNAL "Platform has __bswap_32 swap builtin")
	set(HAVE_BYTESWAP_USHORT ${HAVE_BYTESWAP_USHORT}
		CACHE INTERNAL "Platform has _byteswap_ushort swap builtin")
	set(HAVE_BYTESWAP_ULONG ${HAVE_BYTESWAP_ULONG}
		CACHE INTERNAL "Platform has _byteswap_uling swap builtin")
endfunction(swap_check)

function(clock_check)
	if(HAVE_POSIX)
		check_symbol_exists(clock_gettime time.h HAVE_CLOCK_GETTIME)

		if(NOT(HAVE_CLOCK_GETTIME))
			# OS X check
			check_include_files("mach/mach.h;mach/clock.h" HAVE_MACH_CLOCK_H)
		endif()

		check_symbol_exists(nanosleep time.h HAVE_NANOSLEEP)
		check_symbol_exists(clock_nanosleep time.h HAVE_CLOCK_NANOSLEEP)
	endif()

	set(HAVE_NANOSLEEP ${HAVE_NANOSLEEP}
		CACHE INTERNAL "Platform has nanosleep")
	set(HAVE_CLOCK_NANOSLEEP ${HAVE_CLOCK_NANOSLEEP}
		CACHE INTERNAL "Platform has clock_nanosleep")
endfunction(clock_check)

function(mmap_check)
	if(HAVE_POSIX)
		check_symbol_exists(mmap sys/mman.h HAVE_MMAP)
		if(HAVE_MMAP)
			check_symbol_exists(mremap sys/mman.h HAVE_MREMAP)
		endif()
	endif()

	set(HAVE_MMAP ${HAVE_MMAP}
		CACHE INTERNAL "Platform has mmap")
	set(HAVE_MREMAP ${HAVE_MREMAP}
		CACHE INTERNAL "Platform has mremap")
endfunction(mmap_check)

function(endian_check)
	test_big_endian(BIG_ENDIAN)
	if(NOT BIG_ENDIAN)
		set(LITTLE_ENDIAN 1)
	endif()

	set(BIG_ENDIAN ${BIG_ENDIAN}
		CACHE INTERNAL "Platform is big-endian")
	set(LITTLE_ENDIAN ${LITTLE_ENDIAN}
		CACHE INTERNAL "Platform is little-endian")
endfunction(endian_check)

macro(platform_checks)
	posix_check()
	if(NOT HAVE_POSIX)
		windows_check()
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		set(GLIBC_FEATURE "-D_XOPEN_SOURCE=700")
		set(CMAKE_REQUIRED_DEFINITIONS ${GLIBC_FEATURE})
		add_definitions(${GLIBC_FEATURE})
	endif()

	endian_check()
	stdc_check()
	swap_check()
	clock_check()
	mmap_check()
endmacro(platform_checks)
