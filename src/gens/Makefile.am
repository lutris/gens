## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS = debug emulator gens_core mp3_dec port sdllayer segacd ui util

INCLUDES = \
-I@srcdir@/gens_core/cpu/68k -I@srcdir@/gens_core/cpu/sh2 -I@srcdir@/gens_core/cpu/z80 \
-I@srcdir@/gens_core/sound -I@srcdir@/gens_core/mem -I@srcdir@/gens_core/misc -I@srcdir@/gens_core/gfx \
-I@srcdir@/gens_core/io -I@srcdir@/gens_core/vdp -I@srcdir@/segacd -I@srcdir@/mp3_dec -I@srcdir@/sdllayer \
-I@srcdir@/util -I@srcdir@/util/gfx -I@srcdir@/util/sound -I@srcdir@/util/file \
-I@srcdir@/port -I@srcdir@/emulator -I@srcdir@/debug -I@srcdir@/netplay \
-I@srcdir@/ui -I@srcdir@/ui/gtk \
-I@srcdir@

bin_PROGRAMS = gens
gens_SOURCES = 
##		netplay/ccnet.c 
##		netplay/ccnet.h
		
gens_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@


gens_LDADD = \
		emulator/libgens_emulator.a \
		debug/libgens_debug.a \
		segacd/libgens_segacd.a \
		mp3_dec/libgens_mp3_dec.a \
		port/libgens_port.a \
		sdllayer/libgens_sdllayer.a \
		ui/libgens_ui.a \
		ui/gtk/libgens_ui_gtk.a \
		util/file/libgens_util_file.a \
		util/gfx/libgens_util_gfx.a \
		util/sound/libgens_util_sound.a \
		gens_core/libgens_core.a \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o \
		-lm -lz @SDL_LIBS@ @GTK_LIBS@
#		$(ASMFILES) \ (before main68k.o)

## ARGH!! Get me some C4 and the address of the programmer who made me 
## have to do this!!


gens_DEPENDENCIES = \
		$(ASMFILES) \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o

CLEANFILES = \
		$(ASMFILES) \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o
