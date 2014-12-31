# This is a toolchain file you can use to build the win64 version of
# supergameherm on Linux
#
# To use this file, do the following:
#
#    $ rm -rf CMakeCache.txt CMakeFiles
#    $ cmake -DCMAKE_TOOLCHAIN_FILE=doc/Toolchain-mingw-w64.cmake


# Set this to either i686 or x86_64
set(WIN_ARCH x86_64)

# Set this to where you want your local libs/headers
set(LOCAL_MINGW_PATH "/home/elizabeth/mingw")

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER "${WIN_ARCH}-w64-mingw32-gcc-win32")
SET(CMAKE_CXX_COMPILER "${WIN_ARCH}-w64-mingw32-g++-win32")
SET(CMAKE_RC_COMPILER "${WIN_ARCH}-w64-mingw32-windres")

# here is the target environment located
# To use things like SDL2 and such, set the /home/elizabeth/mingw portion to
# wherever you're keeping the windows headers/libs
SET(CMAKE_FIND_ROOT_PATH "/usr/${WIN_ARCH}-w64-mingw32/" ${LOCAL_MINGW_PATH})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
