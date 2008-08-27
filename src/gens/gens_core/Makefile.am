## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		gfx/blit.o \
		gfx/hq2x16.o \
		gfx/gfx_cd.o \
		io/io.o \
		mem/mem_m68k.o \
		mem/mem_s68k.o \
		mem/mem_sh2.o \
		mem/mem_z80.o \
		misc/misc.o \
		sound/pwm.o \
		cpu/sh2/sh2a.o \
		vdp/vdp_32x.o \
		vdp/vdp_io.o \
		vdp/vdp_rend.o \
		cpu/z80/z80.o

NASM = \
nasm -O1 -D __GCC2 -f elf \
-I@srcdir@/gens_core/ -I@srcdir@/gens_core/io/ -I@srcdir@/gens_core/misc/ \
-I@srcdir@/gens_core/vdp/ -I@srcdir@/gens_core/sound/ -I@srcdir@/gens_core/gfx/ \
-I@srcdir@/gens_core/cpu/sh2/ -I@srcdir@/gens_core/cpu/z80/ -I@srcdir@/gens_core/mem/

%.o: %.asm
	mkdir -p gens_core/gfx gens_core/io gens_core/mem gens_core/misc gens_core/sound gens_core/cpu/sh2 gens_core/vdp gens_core/cpu/z80 ;
	$(NASM) $< -o $@

INCLUDES = \
-I@srcdir@/../gens_core/cpu/68k -I@srcdir@/../gens_core/cpu/sh2 -I@srcdir@/../gens_core/cpu/z80 \
-I@srcdir@/../gens_core/sound -I@srcdir@/../gens_core/mem -I@srcdir@/../gens_core/misc -I@srcdir@/../gens_core/gfx \
-I@srcdir@/../gens_core/io -I@srcdir@/../gens_core/vdp -I@srcdir@/../segacd -I@srcdir@/../mp3_dec -I@srcdir@/../sdllayer \
-I@srcdir@/../util -I@srcdir@/../util/gfx -I@srcdir@/../util/sound -I@srcdir@/../util/file \
-I@srcdir@/../port -I@srcdir@/../emulator -I@srcdir@/../debug -I@srcdir@/../netplay \
-I@srcdir@/../ui -I@srcdir@/../ui/gtk \
-I@srcdir@/../ -I@srcdir@

lib_LIBRARIES = libgens_core.a

libgens_core_a_SOURCES = \
		nasmhead.inc \
		cpu/68k/cpu_68k.c \
		cpu/sh2/cpu_sh2.c \
		cpu/sh2/sh2a.asm \
		cpu/sh2/sh2.c \
		cpu/sh2/sh2_io.inc \
		cpu/z80/z80.asm \
		cpu/z80/cpu_z80.c \
		sound/pcm.c \
		sound/psg.c \
		sound/ym2612.c \
		sound/pwm.asm \
		mem/mem_m68k.asm \
		mem/mem_s68k.asm \
		mem/mem_sh2.asm \
		mem/mem_z80.asm \
		mem/mem_m68k_32x.inc \
		mem/mem_m68k_cd.inc \
		misc/byteswap.c \
		misc/misc.asm \
		gfx/blit.asm \
		gfx/blit2.c \
		gfx/hqfilters.c \
		gfx/hq2x16.asm \
		gfx/gfx_cd.asm \
		io/io.asm \
		vdp/vdp_io.asm \
		vdp/vdp_rend.asm \
		vdp/vdp_32x.asm

noinst_HEADERS = \
		cpu/68k/cpu_68k.h \
		cpu/68k/star_68k.h \
		cpu/sh2/sh2.h \
		cpu/sh2/cpu_sh2.h \
		cpu/z80/cpu_z80.h \
		cpu/z80/z80.h \
		sound/pcm.h \
		sound/psg.h \
		sound/ym2612.h \
		sound/pwm.h \
		mem/mem_m68k.h \
		mem/mem_s68k.h \
		mem/mem_sh2.h \
		mem/mem_z80.h \
		misc/byteswap.h \
		misc/misc.h \
		gfx/blit.h \
		gfx/gfx_cd.h \
		io/io.h \
		vdp/vdp_32x.h \
		vdp/vdp_io.h \
		vdp/vdp_rend.h

libgens_core_a_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@

libgens_core_a_LIBADD = ${ASMFILES}
