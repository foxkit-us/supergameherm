# TODO - use caca-config for hints

find_path(libcaca_INCLUDE_DIR caca.h)
find_library(libcaca_LIBRARY caca)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libcaca DEFAULT_MSG libcaca_LIBRARY
	libcaca_INCLUDE_DIR)

if(FOUND_libcaca)
	check_symbol_exists(CACA_API_VERSION_1 caca.h CACA_API_VERSION_1)
	if(not defined CACA_API_VERSION_1)
		message("Your libcaca is too new (nobody ships 1.0 yet :/)")
		unset(libcaca_INCLUDE_DIR)
		unset(libcaca_LIBRARY)
	endif()
endif()

mark_as_advanced(libcaca_INCLUDE_DIR libcaca_LIBRARY)
