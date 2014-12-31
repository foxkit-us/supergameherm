# TODO - use caca-config for hints

find_path(libcaca_INCLUDE_DIR caca.h)
find_library(libcaca_LIBRARY caca)
check_symbol_exists(CACA_API_VERSION_1 caca.h CACA_API_VERSION_1)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libcaca FOUND_VAR libcaca_FOUND
	REQUIRED_VARS libcaca_LIBRARY libcaca_INCLUDE_DIR
	VERSION_VAR CACA_API_VERSION_1)

mark_as_advanced(libcaca_INCLUDE_DIR libcaca_LIBRARY)
