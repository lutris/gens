# minizip: ZIP file handling library.

AUTOMAKE_OPTIONS = foreign subdir-objects

noinst_LTLIBRARIES = libminizip.la

libminizip_la_CFLAGS	= $(AM_CFLAGS) -static -O3 -funroll-loops
libminizip_la_LDFLAGS	= $(AM_LDFLAGS) -static

if GENS_ZLIB_INTERNAL
INCLUDES = -I@srcdir@/../zlib/
endif # GENS_ZLIB_INTERNAL

libminizip_la_SOURCES	= unzip.c
noinst_HEADERS		= unzip.h ioapi.h crypt.h

if !GENS_OS_WIN32
libminizip_la_SOURCES	+= ioapi.c
else
libminizip_la_SOURCES	+= iowin32.c
noinst_HEADERS		+= iowin32.h
libminizip_la_LIBADD	= -lkernel32
endif
