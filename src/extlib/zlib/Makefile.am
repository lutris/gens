# zlib: Deflate compression library.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libz.la

libz_la_CFLAGS		= $(AM_CFLAGS) -static -O3 -funroll-loops
libz_la_LDFLAGS		= $(AM_LDFLAGS) -static

libz_la_SOURCES = \
		adler32.c \
		compress.c \
		crc32.c \
		gzio.c \
		uncompr.c \
		deflate.c \
		trees.c \
		zutil.c \
		inflate.c \
		infback.c \
		inftrees.c \
		inffast.c

noinst_HEADERS = \
		crc32.h \
		deflate.h \
		inffast.h \
		inffixed.h \
		inflate.h \
		inftrees.h \
		trees.h \
		zlib.h \
		zutil.h
