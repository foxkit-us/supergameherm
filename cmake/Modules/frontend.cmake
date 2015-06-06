if(POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)
endif()

macro(libcaca_check)
	find_package(libcaca)
	if(libcaca_FOUND)
		option(LIBCACA_ENABLE "Enable libcaca frontend" on)
	endif()
	if(LIBCACA_ENABLE)
		set(HAVE_LIBCACA 1)
		include_directories(${libcaca_INCLUDE_DIR})
		
		file(GLOB LIBCACA_FRONTEND_SOURCES src/frontends/caca/*.c)
		add_executable("sgherm-caca" ${LIBCACA_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)
		target_link_libraries("sgherm-caca" ${libcaca_LIBRARY})

		set(HAVE_FRONTEND on)
	endif()
endmacro()

macro(qt_check)
	####
	# Finding the Qt packages.
	####
	find_package(Qt5Widgets)
	if(Qt5Widgets_FOUND)
		option(QT5_ENABLE "Enable Qt 5 frontend" on)
	endif()

	####
	# Qt5 specific
	####
	if(QT5_ENABLE)
		find_package(SDL2 REQUIRED)  # XXX should go away and use Phonon.
		set(HAVE_QT5 1)

		include_directories(${Qt5Widgets_INCLUDE_DIR})
		file(GLOB QT5_FRONTEND_SOURCES src/frontends/qt/*.cc src/frontends/sdl2/audio.c)
		file(GLOB QT5_FRONTEND_HEADERS include/frontends/qt/*.h)
		file(GLOB QT5_FRONTEND_RESOURCES src/frontends/qt/*.qrc)
		qt5_wrap_cpp(QT5_FRONTEND_MOC ${QT5_FRONTEND_HEADERS})
		qt5_add_resources(QT5_COMPILED_RESOURCES ${QT5_FRONTEND_RESOURCES})
		add_executable("sgherm-qt5" ${QT5_FRONTEND_SOURCES} ${QT5_FRONTEND_MOC} ${QT5_COMPILED_RESOURCES} $<TARGET_OBJECTS:sgherm-core>)
		target_link_libraries("sgherm-qt5" Qt5::Widgets ${SDL2_LIBRARY})
	endif()


	find_package(Qt4)
	if(Qt4_FOUND)
		option(QT4_ENABLE "Enable Qt 4 frontend" on)
	endif()

	####
	# Qt4 specific
	####
	if(QT4_ENABLE)
		find_package(SDL2 REQUIRED)  # XXX same as qt5
		set(HAVE_QT4 1)
		include(${QT_USE_FILE})

		file(GLOB QT4_FRONTEND_SOURCES src/frontends/qt/*.cc src/frontends/sdl2/audio.c)
		file(GLOB QT4_FRONTEND_HEADERS include/frontends/qt/*.h)
		file(GLOB QT4_FRONTEND_RESOURCES src/frontends/qt/*.qrc)
		qt4_wrap_cpp(QT4_FRONTEND_MOC ${QT4_FRONTEND_HEADERS})
		qt4_add_resources(QT4_COMPILED_RESOURCES ${QT4_FRONTEND_RESOURCES})
		add_executable("sgherm-qt4" ${QT4_FRONTEND_SOURCES} ${QT4_FRONTEND_MOC} ${QT4_COMPILED_RESOURCES} $<TARGET_OBJECTS:sgherm-core>)
		target_link_libraries("sgherm-qt4" ${QT_LIBRARIES} ${SDL2_LIBRARY})
	endif()
endmacro()

macro(sdl2_check)
	find_package(SDL2)
	if(SDL2_FOUND)
		option(SDL2_ENABLE "Enable SDL2 frontend" on)
	endif()

	if(SDL2_ENABLE)
		find_package(SDL2 REQUIRED)
		mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY SDL2MAIN_LIBRARY)
		set(HAVE_SDL2 1)
		include_directories(${SDL2_INCLUDE_DIR})
		
		file(GLOB SDL2_FRONTEND_SOURCES src/frontends/sdl2/*.c)
		add_executable("sgherm-sdl2" WIN32 ${SDL2_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)
		target_link_libraries("sgherm-sdl2" ${SDL2_LIBRARY})

		set(HAVE_FRONTEND on)
	endif()
endmacro()

macro(win32_check)
	if(HAVE_WINDOWS)
		option(WIN32_ENABLE "Enable the Win32 GDI frontend" on)
	endif()

	if(WIN32_ENABLE)
		set(HAVE_WIN32 1)

		file(GLOB W32_FRONTEND_SOURCES src/frontends/w32/*.c)
		add_executable("sgherm-gdi" WIN32 ${W32_FRONTEND_SOURCES} $<TARGET_OBJECTS:sgherm-core>)

		if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
			# necessary to work around 'stdbool' and related stuff
			file(GLOB_RECURSE ALL_SOURCES src/*.c)
			set_source_files_properties(ALL_SOURCES PROPERTIES LANGUAGE CXX)
			set_target_properties("sgherm-gdi" PROPERTIES LINKER_LANGUAGE CXX)
		endif()

		target_link_libraries("sgherm-gdi" winmm)

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
	qt_check()
	win32_check()
	null_check()
endmacro()
