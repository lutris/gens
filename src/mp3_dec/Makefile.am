## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libmp3_dec.a

libmp3_dec_a_CFLAGS = $(AM_CFLAGS)

libmp3_dec_a_SOURCES = \
		common.c \
		dct64_i386.c \
		decode_i386.c \
		interface.c \
		layer3.c \
		tabinit.c

noinst_HEADERS = \
		huffman.h \
		mpg123.h \
		mpglib.h
