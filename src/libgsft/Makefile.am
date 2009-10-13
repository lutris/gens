# libgsft: Common functions.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libgsft.la

libgsft_la_CFLAGS	= $(AM_CFLAGS) -static -O3 -funroll-loops
libgsft_la_CXXFLAGS	= $(AM_CXXFLAGS) -static -O3 -funroll-loops
libgsft_la_LDFLAGS	= $(AM_LDFLAGS) -static

libgsft_la_SOURCES = \
		gsft_byteswap.c \
		gsft_file.c

noinst_HEADERS = \
		gsft_fncall.h \
		gsft_hashtable.hpp \
		gsft_byteswap.h \
		gsft_bool.h \
		gsft_fastcall.h \
		gsft_malloc_align.h \
		gsft_unused.h \
		gsft_file.h \
		gsft_szprintf.h \
		gsft_strlcpy.h

if !HAVE_STRDUP
libgsft_la_SOURCES	+= gsft_strdup.c
noinst_HEADERS		+= gsft_strdup.h
endif

if !HAVE_STRSEP
libgsft_la_SOURCES	+= gsft_strsep.c
noinst_HEADERS		+= gsft_strsep.h
endif

if !HAVE_STRLCPY
libgsft_la_SOURCES	+= strlcpy.c
endif

if !HAVE_STRLCAT
libgsft_la_SOURCES	+= strlcat.c
endif

# Win32-specific code.
if GENS_OS_WIN32

noinst_LTLIBRARIES += libgsft_win32.la libgsft_win32_gdi.la

# General Win32 stuff.
libgsft_win32_la_CFLAGS		= $(libgsft_la_CFLAGS)
libgsft_win32_la_CXXFLAGS	= $(libgsft_la_CXXFLAGS)
libgsft_win32_la_LDFLAGS	= $(libgsft_la_LDFLAGS)

libgsft_win32_la_SOURCES = \
		gsft_win32.c
noinst_HEADERS += \
		gsft_win32.h

# Win32 GDI stuff.
libgsft_win32_gdi_la_CFLAGS	= $(libgsft_win32_la_CFLAGS)
libgsft_win32_gdi_la_CXXFLAGS	= $(libgsft_win32_la_CXXFLAGS)
libgsft_win32_gdi_la_LDFLAGS	= $(libgsft_win32_la_LDFLAGS)

libgsft_win32_gdi_la_SOURCES = \
		gsft_win32_gdi.c
noinst_HEADERS += \
		gsft_win32_gdi.h

endif
