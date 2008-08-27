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

lib_LIBRARIES = libgens_emulator.a

libgens_emulator_a_SOURCES = \
		ui_proxy.c \
		g_main.c \
		g_main_linux.c \
		g_input.c \
		g_md.c \
		g_mcd.c \
		g_32x.c \
		g_palette.c \
		gens.c \
		parse.c

noinst_HEADERS = \
		ui_proxy.h \
		g_main.h \
		g_main_linux.h \
		g_input.h \
		g_input_linux.h \
		g_md.h \
		g_mcd.h \
		g_32x.h \
		g_palette.h \
		gens.h \
		parse.h

libgens_emulator_a_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@
