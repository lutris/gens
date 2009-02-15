## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

if GENS_ZLIB_INTERNAL
INCLUDES = -I@srcdir@/../zlib/
endif # GENS_ZLIB_INTERNAL

CFLAGS = @CFLAGS@ -O3 -funroll-loops \
	-Wno-old-style-definition \
	-Wno-unused \
	-Wno-strict-prototypes \
	-Wno-parentheses \
	-Wno-switch-default \
	-Wno-uninitialized

noinst_LIBRARIES = libminizip.a
libminizip_a_SOURCES = \
		ioapi.c \
		unzip.c

noinst_HEADERS = \
		ioapi.h \
		unzip.h \
		crypt.h
