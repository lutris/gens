# mp3_dec: MP3 decoder.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libmp3_dec.la

libmp3_dec_la_CFLAGS	= $(AM_CFLAGS) -static
libmp3_dec_la_LDFLAGS	= $(AM_LDFLAGS) -static

libmp3_dec_la_SOURCES = \
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
