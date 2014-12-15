macro(libcaca_check)
	option(LIBCACA_ENABLE "Enable libcaca frontend" on)
	if(LIBCACA_ENABLE)
		find_package(libcaca REQUIRED)
		set(HAVE_LIBCACA 1)
		include_directories(${LIBCACA_INCLUDE_DIR})
		
		file(GLOB LIBCACA_FRONTEND_SOURCES src/frontends/caca/*.c)
		add_executable("sgherm-caca" ${LIBCACA_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)
		target_link_libraries("sgherm-caca" ${libcaca_LIBRARY})

		set(HAVE_FRONTEND on)
	endif()
endmacro()

macro(sdl2_check)
	option(SDL2_ENABLE "Enable SDL2 frontend" on)
	if(SDL2_ENABLE)
		find_package(SDL2 REQUIRED)
		mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY SDL2MAIN_LIBRARY)
		set(HAVE_SDL2 1)
		include_directories(${SDL2_INCLUDE_DIR})
		
		file(GLOB SDL2_FRONTEND_SOURCES src/frontends/sdl2/*.c)
		add_executable("sgherm-sdl2" ${SDL2_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)
		target_link_libraries("sgherm-sdl2" ${SDL2_LIBRARY})

		set(HAVE_FRONTEND on)
	endif()
endmacro()

macro(win32_check)
	if(HAVE_WINDOWS)
		option(WIN32_ENABLE "Enable the Win32 frontend" on)
	endif()

	if(WIN32_ENABLE)
		set(HAVE_WIN32 1)

		file(GLOB W32_FRONTEND_SOURCES src/frontends/w32/*.c)
		add_executable("sgherm-w32" ${W32_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)

		if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
			# necessary to work around 'stdbool' and related stuff
			file(GLOB_RECURSE ALL_SOURCES src/*.c)
			set_source_files_properties(ALL_SOURCES PROPERTIES LANGUAGE CXX)
			set_target_properties("sgherm-w32" PROPERTIES LINKER_LANGUAGE CXX)
		endif()

		set(HAVE_FRONTEND on)
	endif()
endmacro()

macro(null_check)
	# Set the default value depending on if we have a frontend
	if(HAVE_FRONTEND)
		set(NULL_DEFAULT off)
	else()
		set(NULL_DEFAULT on)
	endif()

	# Add the null backend
	option(NULL_ENABLE "Enable a frontendless sgherm build (testing)" ${NULL_DEFAULT})
	if(NULL_ENABLE)
		file(GLOB NULL_FRONTEND_SOURCES src/frontends/null/*.c)
		add_executable("sgherm-null" ${NULL_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)
	endif()
endmacro()

macro(frontend_checks)
	sdl2_check()
	libcaca_check()
	win32_check()
	null_check()
endmacro()
