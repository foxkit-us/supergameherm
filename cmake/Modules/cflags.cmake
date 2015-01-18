if(POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)
endif()

include(CheckCCompilerFlag)

macro(test_c11)
	check_c_compiler_flag("-std=c11" HAVE_C11)
	if(HAVE_C11)
		set(SGHERM_C_STANDARD "-std=c11")
	else()
		check_c_compiler_flag("-fms-extensions" HAVE_MS_EXTENSIONS)
		if(HAVE_MS_EXTENSIONS)
			# this allows us to use unnamed unions and is REQUIRED
			# on NetBSD and OS X 10.4/10.5.
			set(SGHERM_C_STANDARD "-std=c99 -fms-extensions")
		else()
			set(SGHERM_C_STANDARD "-std=c99")
		endif()
		message(WARNING "Your compiler does not support C11.  Things may break.")
	endif()
endmacro()

macro(set_cflags)
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
		set(HAVE_COMPILER_CLANG "1")
		test_c11()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGHERM_C_STANDARD} -Wpedantic -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -g3")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g3")
		endif()
	elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
		set(HAVE_COMPILER_GCC "1")
		test_c11()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGHERM_C_STANDARD} -pedantic -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ggdb")
		endif()
	elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
		set(HAVE_COMPILER_INTEL "1")
		test_c11()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGHERM_C_STANDARD} -Wall -Wextra")
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set(HAVE_COMPILER_MSVC "1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /G5 /Gr /D _CRT_SECURE_NO_WARNINGS")
		# /G5 - Enable Pentium optimisations
		# /Gr - __fastcall as deafult
		# /D _CRT_NO_SECURE_WARNINGS - disable whining about fopen and friends
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /W4 /Ot /Oa /Gy")
		# /MTd - statically link to runtime library
		# /W4 - enable all warnings
		# /Ot, /Oa - still enable some performance improvements
		# /Gy - function-level linking (slightly faster)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zp4 /MT
		/W3 /Zi /Ox /Ot /Oa /Og /Ob2 /GF")
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
