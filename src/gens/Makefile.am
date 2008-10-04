## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

if GENS_X86_ASM
gens_x86_asm_o = \
		gens_core/gfx/blit1x.o \
		gens_core/gfx/blit2x.o \
		gens_core/gfx/blit2x_int.o \
		gens_core/gfx/blit2x_scanline.o \
		gens_core/gfx/blit2x_scanline_50.o \
		gens_core/gfx/blit2x_scanline_25.o \
		gens_core/gfx/blit2x_scanline_int.o \
		gens_core/gfx/blit2x_scanline_50_int.o \
		gens_core/gfx/blit2x_scanline_25_int.o \
		gens_core/gfx/blit_2xSAI.o \
		gens_core/gfx/blit_hq2x_16.o \
		gens_core/misc/fastblur_16.o \
		audio/audio_mmx.o
gens_x86_asm_src = \
		gens_core/gfx/blit1x.asm \
		gens_core/gfx/blit2x.asm \
		gens_core/gfx/blit2x_int.asm \
		gens_core/gfx/blit2x_scanline.asm \
		gens_core/gfx/blit2x_scanline_50.asm \
		gens_core/gfx/blit2x_scanline_25.asm \
		gens_core/gfx/blit2x_scanline_int.asm \
		gens_core/gfx/blit2x_scanline_50_int.asm \
		gens_core/gfx/blit2x_scanline_25_int.asm \
		gens_core/gfx/blit_2xSAI.asm \
		gens_core/gfx/blit_hq2x_16.asm \
		gens_core/misc/fastblur_16.asm \
		audio/audio_mmx.asm
gens_x86_asm_h = \
		gens_core/misc/fastblur_16.h \
		audio/audio_mmx.h
else
gens_x86_asm_o =
gens_x86_asm_src =
gens_x86_asm_h =
endif

ASMFILES = \
		$(gens_x86_asm_o) \
		gens_core/gfx/gfx_cd.o \
		gens_core/io/io_old.o \
		gens_core/mem/mem_m68k.o \
		gens_core/mem/mem_s68k.o \
		gens_core/mem/mem_sh2.o \
		gens_core/mem/mem_z80.o \
		gens_core/misc/misc.o \
		gens_core/sound/pwm.o \
		gens_core/cpu/sh2/sh2a.o \
		gens_core/vdp/vdp_32x.o \
		gens_core/vdp/vdp_io.o \
		gens_core/vdp/vdp_rend.o \
		gens_core/cpu/z80/z80.o

NASM_INCLUDES = \
-I@srcdir@/gens_core/ -I@srcdir@/gens_core/io/ -I@srcdir@/gens_core/misc/ \
-I@srcdir@/gens_core/vdp/ -I@srcdir@/gens_core/sound/ -I@srcdir@/gens_core/gfx/ \
-I@srcdir@/gens_core/cpu/sh2/ -I@srcdir@/gens_core/cpu/z80/ -I@srcdir@/gens_core/mem/

NASM = nasm -O1 -w-orphan-labels $(NASM_INCLUDES)
INCLUDES = -I@srcdir@ -I@srcdir@/../ -I@srcdir@/ui/

if GENS_ZLIB_INTERNAL
INCLUDES += -I@srcdir@/../zlib/
endif # GENS_ZLIB_INTERNAL

if GENS_PNG_INTERNAL
INCLUDES += -I@srcdir@/../libpng/
endif # GENS_PNG_INTERNAL

# x86 assembly language (using nasm)
%.o: %.asm
	mkdir -p gens_core/gfx/ gens_core/io/ gens_core/mem/ gens_core/misc/ ;
	mkdir -p gens_core/sound/ gens_core/cpu/sh2/ gens_core/vdp/ gens_core/cpu/z80/ ;
	$(NASM) $< -o $@

# Win32 resources
.rc.o:
	$(host_alias)-windres $< -o $@
%.o: %.rc
	$(host_alias)-windres $< -o $@

bin_PROGRAMS = gens

gens_SOURCES = \
		emulator/gens.cpp \
		emulator/g_main.cpp \
		emulator/g_input.cpp \
		emulator/g_md.cpp \
		emulator/g_mcd.cpp \
		emulator/g_32x.cpp \
		emulator/g_32x_32bit.c \
		emulator/g_palette.c \
		emulator/g_update.cpp \
		emulator/parse.cpp \
		emulator/ui_proxy.cpp \
		gens_core/nasmhead.inc \
		gens_core/cpu/68k/cpu_68k.c \
		gens_core/cpu/sh2/cpu_sh2.c \
		gens_core/cpu/sh2/sh2a.asm \
		gens_core/cpu/sh2/sh2.c \
		gens_core/cpu/sh2/sh2_io.inc \
		gens_core/cpu/z80/z80.asm \
		gens_core/cpu/z80/cpu_z80.c \
		gens_core/sound/pcm.c \
		gens_core/sound/psg.c \
		gens_core/sound/ym2612.c \
		gens_core/sound/pwm.asm \
		gens_core/mem/mem_m68k.asm \
		gens_core/mem/mem_s68k.asm \
		gens_core/mem/mem_sh2.asm \
		gens_core/mem/mem_z80.asm \
		gens_core/mem/mem_m68k_32x.inc \
		gens_core/mem/mem_m68k_cd.inc \
		gens_core/misc/byteswap.c \
		gens_core/misc/misc.asm \
		gens_core/misc/fastblur.c \
		$(gens_x86_asm_src) \
		gens_core/gfx/blit1x.cpp \
		gens_core/gfx/blit2x.cpp \
		gens_core/gfx/blit2x_scanline.cpp \
		gens_core/gfx/scale2x.cpp \
		gens_core/gfx/hq2x_funcs.c \
		gens_core/gfx/renderers.c \
		gens_core/gfx/gfx_cd.asm \
		gens_core/io/io.c \
		gens_core/io/io_old.asm \
		gens_core/vdp/vdp_io.asm \
		gens_core/vdp/vdp_rend.asm \
		gens_core/vdp/vdp_32x.asm \
		port/ini.cpp \
		port/timer.c \
		segacd/cd_file.c \
		segacd/cd_sys.cpp \
		segacd/cdda_mp3.c \
		segacd/lc89510.c \
		ui/gens_ui.cpp \
		ui/charset.cpp \
		util/file/config_file.cpp \
		util/file/rom.cpp \
		util/file/save.cpp \
		util/file/ggenie.c \
		util/file/compress/compressor.cpp \
		util/file/compress/subcompressor.cpp \
		util/file/compress/7z.cpp \
		util/gfx/scrshot.c \
		util/sound/gym.cpp \
		util/sound/wave.c \
		video/C64_charset.c \
		video/v_draw.cpp \
		video/v_effects.cpp \
		input/input.cpp \
		audio/audio.cpp

if GENS_DEBUGGER
gens_SOURCES += \
		debugger/debugger.cpp \
		debugger/m68kd.c \
		debugger/sh2d.c \
		debugger/z80dis.c
endif

if GENS_CDROM
gens_SOURCES += \
		segacd/cd_aspi.cpp
endif

if GENS_ZLIB
gens_SOURCES += \
		util/file/compress/gzip.cpp \
		util/file/compress/Zip.cpp
endif

noinst_HEADERS = \
		emulator/gens.hpp \
		emulator/g_main.hpp \
		emulator/g_input.hpp \
		emulator/g_md.hpp \
		emulator/g_mcd.hpp \
		emulator/g_32x.hpp \
		emulator/g_32x_32bit.h \
		emulator/g_palette.h \
		emulator/g_update.hpp \
		emulator/parse.hpp \
		emulator/ui_proxy.hpp \
		gens_core/cpu/68k/cpu_68k.h \
		gens_core/cpu/68k/star_68k.h \
		gens_core/cpu/sh2/sh2.h \
		gens_core/cpu/sh2/cpu_sh2.h \
		gens_core/cpu/z80/cpu_z80.h \
		gens_core/cpu/z80/z80.h \
		gens_core/sound/pcm.h \
		gens_core/sound/psg.h \
		gens_core/sound/ym2612.h \
		gens_core/sound/pwm.h \
		gens_core/mem/mem_m68k.h \
		gens_core/mem/mem_s68k.h \
		gens_core/mem/mem_sh2.h \
		gens_core/mem/mem_z80.h \
		gens_core/misc/byteswap.h \
		gens_core/misc/misc.h \
		gens_core/misc/fastblur.h \
		$(gens_x86_asm_h) \
		gens_core/gfx/blit.h \
		gens_core/gfx/gfx_cd.h \
		gens_core/gfx/renderers.h \
		gens_core/io/io.h \
		gens_core/io/io_old.h \
		gens_core/vdp/vdp_32x.h \
		gens_core/vdp/vdp_io.h \
		gens_core/vdp/vdp_rend.h \
		port/ini.hpp \
		port/timer.h \
		segacd/cd_file.h \
		segacd/cd_sys.hpp \
		segacd/cdda_mp3.h \
		segacd/lc89510.h \
		ui/gens_ui.hpp \
		ui/charset.hpp \
		ui/about_window_data.h \
		util/file/config_file.hpp \
		util/file/rom.hpp \
		util/file/save.hpp \
		util/file/ggenie.h \
		util/file/compress/compressor.hpp \
		util/file/compress/subcompressor.hpp \
		util/file/compress/7z.hpp \
		util/gfx/scrshot.h \
		util/sound/gym.hpp \
		util/sound/wave.h \
		video/C64_charset.h \
		video/v_draw.hpp \
		video/v_effects.hpp \
		input/input.hpp \
		audio/audio.hpp \
		audio/audio_mmx.h

if GENS_DEBUGGER
noinst_HEADERS += \
		debugger/debugger.hpp \
		debugger/m68kd.h \
		debugger/sh2d.h \
		debugger/z80dis.h
endif

if GENS_CDROM
noinst_HEADERS += \
		segacd/cd_aspi.hpp
endif

if GENS_ZLIB
noinst_HEADERS += \
		util/file/compress/gzip.hpp \
		util/file/compress/Zip.hpp
endif



gens_LDADD = \
		${ASMFILES} \
		../mp3_dec/libmp3_dec.a \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o \
		-lm

if GENS_ZLIB
gens_LDADD += ../minizip/libminizip.a
if GENS_ZLIB_INTERNAL
gens_LDADD += ../zlib/libz.a
else
gens_LDADD += -lz
endif # GENS_ZLIB_INTERNAL
endif # GENS_ZLIB

if GENS_PNG
if GENS_PNG_INTERNAL
gens_LDADD += ../libpng/libpng.a
else
gens_LDADD += -lpng
endif # GENS_PNG_INTERNAL
endif # GENS_PNG

## ARGH!! Get me some C4 and the address of the programmer who made me 
## have to do this!!

gens_DEPENDENCIES = \
		${ASMFILES} \
		../mp3_dec/libmp3_dec.a \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o

if GENS_ZLIB
gens_DEPENDENCIES += ../minizip/libminizip.a
if GENS_ZLIB_INTERNAL
gens_DEPENDENCIES += ../zlib/libz.a
endif # GENS_ZLIB_INTERNAL
endif # GENS_ZLIB

if GENS_PNG
if GENS_PNG_INTERNAL
gens_DEPENDENCIES += ../libpng/libpng.a
endif # GENS_PNG_INTERNAL
endif # GENS_PNG

CLEANFILES = \
		${ASMFILES} \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o

# Initialize CFLAGS and CXXFLAGS.
gens_CFLAGS =
gens_CXXFLAGS =

# OS-specific includes

if GENS_OS_LINUX
include $(srcdir)/Makefile.linux.am.inc
endif # GENS_OS_LINUX

if GENS_OS_WIN32
include $(srcdir)/Makefile.win32.am.inc
endif # GENS_OS_WIN32

# Add CFLAGS and CXXFLAGS to the end of the OS-specific flags.
gens_CFLAGS += @CFLAGS@
gens_CXXFLAGS += @CXXFLAGS@
