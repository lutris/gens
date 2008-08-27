## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = \
-I@srcdir@/../gens_core/cpu/68k -I@srcdir@/../gens_core/cpu/sh2 -I@srcdir@/../gens_core/cpu/z80 \
-I@srcdir@/../gens_core/sound -I@srcdir@/../gens_core/mem -I@srcdir@/../gens_core/misc -I@srcdir@/../gens_core/gfx \
-I@srcdir@/../gens_core/io -I@srcdir@/../gens_core/vdp -I@srcdir@/../segacd -I@srcdir@/../mp3_dec -I@srcdir@/../sdllayer \
-I@srcdir@/../util -I@srcdir@/../util/gfx -I@srcdir@/../util/sound -I@srcdir@/../util/file \
-I@srcdir@/../port -I@srcdir@/../emulator -I@srcdir@/../debug -I@srcdir@/../netplay \
-I@srcdir@/../ui -I@srcdir@/../ui/gtk \
-I@srcdir@/../ -I@srcdir@

lib_LIBRARIES = libgens_sdllayer.a

libgens_sdllayer_a_SOURCES = \
		g_sdldraw.c \
		g_sdlinput.c \
		g_sdlsound.c

noinst_HEADERS = \
		g_sdldraw.h \
		g_sdlinput.h \
		g_sdlsound.h
		
libgens_sdllayer_a_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@
