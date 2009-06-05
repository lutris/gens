## libgsft: Common functions.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libgsft.la

libgsft_la_CFLAGS = $(AM_CFLAGS) -static -O3 -funroll-loops

# TODO: Make gsft_win32.c conditional on GENS_OS_WIN32.

libgsft_la_SOURCES = \
		gsft_win32.c

noinst_HEADERS = \
		gsft_fncall.h \
		gsft_win32.h
