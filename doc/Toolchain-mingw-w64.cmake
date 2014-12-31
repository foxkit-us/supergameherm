# This is a toolchain file you can use to build the win64 version of
# supergameherm on Linux
#
# To use this file, delete your CMakeCache.txt, and run the magic invocation:
#
#   $ cmake -DCMAKE_TOOLCHAIN_FILE=doc/Toolchain-mingw-w64.cmake

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-win32)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-win32)
SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# here is the target environment located
# To use things like SDL2 and such, set the /home/elizabeth/mingw portion to
# wherever you're keeping the windows headers/libs
SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32/ /home/elizabeth/mingw)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
