# minizip: ZIP file handling library.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libminizip.la

libminizip_la_CFLAGS	= $(AM_CFLAGS) -static -O3 -funroll-loops
libminizip_la_LDFLAGS	= $(AM_LDFLAGS) -static

if GENS_ZLIB_INTERNAL
INCLUDES = -I@srcdir@/../zlib/
endif # GENS_ZLIB_INTERNAL

libminizip_la_SOURCES = \
		ioapi.c \
		unzip.c

noinst_HEADERS = \
		ioapi.h \
		unzip.h \
		crypt.h
