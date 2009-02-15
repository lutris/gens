## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

CFLAGS = @CFLAGS@ -O3 -funroll-loops \
	-Wno-old-style-definition \
	-Wno-cast-qual \
	-Wno-unreachable-code \
	-Wno-unsafe-loop-optimizations \
	-Wno-switch-default \
	-Wno-switch-enum

noinst_LIBRARIES = libz.a
libz_a_SOURCES = \
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
