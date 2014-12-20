#######################
Platform-specific notes
#######################

This documentation outlines any platform-specific notes gathered by the
developers of SuperGameHerm as they were aware at the time of its release.

Note that this documentation may be out of date by the time you read this,
and suggestions given are simply that; notes here do NOT imply that a
specific platform is supported by SuperGameHerm nor does it imply any form
of guarantee the steps outlined will be a 'magic cure'.

*******
Windows
*******

This chapter outlines notes about building and running SuperGameHerm on
computers running Windows®.

General
=======

The native Windows® API frontend (sgherm-w32.exe) uses GDI calls.  These
calls are accelerated under Windows Vista™ and later versions (including
7, 8, and 10).

The SDL 2 frontend (sgherm-sdl2.exe) will use OpenGL where available to
accelerate graphics calls.  This can result in a speed up over GDI if you
are running an older version of Windows that does not support accelerated
GDI calls, but does support hardware-accelerated OpenGL and SDL 2.

Windows 95/98/Me/NT4 (Visual Studio 6.0/98)
===========================================

Visual C++ 6.0 needs to be updated to at least SP4 to compile the code.
You will need to change the print_cycles function to use signed math as
this compiler does not support conversion of unsigned 64-bit integers to
double-precision floating point representation.

The Windows 95/98/Me kernels don't allow much throughput, even on fast
CPUs (such as the Pentium 4), so the emulator may be so slow as to be
unplayable.  This is a design issue with that version of Windows and it
cannot be worked around.

On Windows NT 4 it should be playable assuming your CPU is fast enough.

Windows 2000 (Visual Studio .NET 7.1/2003)
==========================================

No special build steps should be required.

The emulator should run without issue.

Windows XP (Visual Studio 2005)
===============================

SDL 2 is untested on Windows XP and Visual Studio 2005.

Windows Vista™ (Visual Studio 2008)
===================================

No special build steps are required.

The emulator should run without issue.

Windows 7 (Visual Studio 2010)
==============================

No special build steps are required.  This compiler supports enough C99 to
build SuperGameHerm in C mode, instead of C++.

The emulator should run without issue with both SDL 2 and GDI.

Windows 8 (Visual Studio 2012)
==============================

Untested.

Windows 10 Build 9879 (Visual Studio 2015 CTP)
==============================================

This compiler supports all C99 and C11 features in use by SuperGameHerm.
No special build steps are required.

The emulator should run without issue with both SDL 2 and GDI.


*****
Linux
*****

Most system/architecture/frontend combinations should run without error,
as long as the compiler in use is Clang.

GCC 4.8's optimiser has bugs that we attempt to work around, but we cannot
vouch for its correctness regarding code generation.  GCC 4.6 and 4.7
should also work, but do not support some C11 features.  We attempt to
work around this too, but it starts to become rather un-worthwhile to
continue writing code against buggy compilers (it's a miracle the code
works at all in Visual C++, even).

ICC has not been tested, but as it has its own optimisation engine, it
should work well.  Please report your findings, either way, to us.


***
BSD
***

FreeBSD
=======

The libcaca and SDL 2 frontends should work fine on at least amd64 and
i386.

NetBSD
======

The libcaca frontend works.  The SDL 2 frontend does not work due to
an incompatibility with NetBSD's X.Org distribution.

OpenBSD
=======

The libcaca and SDL 2 frontends work on amd64.  Other architectures have
not been tested.


*****
Other
*****

Testing has not been done on any system not specified above.  If you are
able to run SuperGameHerm on one, or are having issues building, let us
know - however, we cannot and will not support every system and every
architecture ever made.
