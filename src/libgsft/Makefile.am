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
		gsft_strz.h

if !HAVE_STRDUP
libgsft_la_SOURCES	+= gsft_strdup.c
noinst_HEADERS		+= gsft_strdup.h
endif

if !HAVE_STRSEP
libgsft_la_SOURCES	+= gsft_strsep.c
noinst_HEADERS		+= gsft_strsep.h
endif

# Win32-specific headers.
if GENS_OS_WIN32
libgsft_la_SOURCES += \
		gsft_win32.c
noinst_HEADERS += \
		gsft_win32.h
endif
