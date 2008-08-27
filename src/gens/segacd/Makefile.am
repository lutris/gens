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

lib_LIBRARIES = libgens_segacd.a

libgens_segacd_a_SOURCES = \
		cd_aspi.c \
		cd_file.c \
		cd_sys.c \
		cdda_mp3.c \
		lc89510.c

noinst_HEADERS = \
		cd_aspi.h \
		cd_file.h \
		cd_sys.h \
		cdda_mp3.h \
		lc89510.h
		
libgens_segacd_a_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@
