## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

libpng_a_CFLAGS =

if !GENS_X86_ASM
libpng_a_CFLAGS += -DPNG_NO_MMX_CODE
endif # !GENS_X86_ASM

libpng_a_CFLAGS += @CFLAGS@ -O3 -funroll-loops

if GENS_ZLIB_INTERNAL
INCLUDES = -I@srcdir@/../zlib/
endif # GENS_ZLIB_INTERNAL

noinst_LIBRARIES = libpng.a
libpng_a_SOURCES = \
		png.c \
		pngset.c \
		pngget.c \
		pngrutil.c \
		pngtrans.c \
		pngwutil.c \
		pngread.c \
		pngrio.c \
		pngwio.c \
		pngwrite.c \
		pngrtran.c \
		pngwtran.c \
		pngmem.c \
		pngerror.c \
		pngpread.c

noinst_HEADERS = \
		png.h \
		pngconf.h
