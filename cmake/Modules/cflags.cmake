if(POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)
endif()

macro(set_cflags)
	if(${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
		set(HAVE_COMPILER_CLANG "1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wpedantic -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -g3")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g3")
		endif()
	elseif(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
		set(HAVE_COMPILER_GCC "1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -pedantic -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ggdb")
		endif()
	elseif(${CMAKE_C_COMPILER_ID} STREQUAL "Intel")
		set(HAVE_COMPILER_INTEL "1")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra")
		if(HAVE_POSIX)
			set(CMAKE_CXX_FLAGS_DEBUG, "${CMAKE_CXX_FLAGS_DEBUG} -g")
			set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g")
		endif()
	elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
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
