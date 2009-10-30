## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = plugins fw

if GENS_X86_ASM
gens_x86_asm_o =
gens_x86_asm_src = \
		gens_core/gfx/fastblur_16_x86.S \
		gens_core/gfx/fastblur_32_x86.S \
		audio/audio_mmx.S
gens_x86_asm_h = \
		gens_core/gfx/fastblur_x86.h \
		audio/audio_mmx.h
else
gens_x86_asm_o =
gens_x86_asm_src =
gens_x86_asm_h =
endif

ASMFILES = \
		$(gens_x86_asm_o) \
		gens_core/gfx/gfx_cd.o \
		gens_core/mem/mem_m68k.o \
		gens_core/mem/mem_m68k_cd.o \
		gens_core/mem/mem_m68k_32x.o \
		gens_core/mem/mem_s68k.o \
		gens_core/mem/mem_sh2.o \
		gens_core/misc/misc.o \
		gens_core/cpu/sh2/sh2a.o \
		gens_core/cpu/sh2/sh2_io.o \
		gens_core/vdp/vdp_32x_x86.o \
		gens_core/vdp/vdp_io_x86.o \
		gens_core/vdp/vdp_rend_x86.o

NASM_INCLUDES = \
-I@srcdir@/gens_core/ -I@srcdir@/gens_core/io/ -I@srcdir@/gens_core/misc/ \
-I@srcdir@/gens_core/vdp/ -I@srcdir@/gens_core/sound/ -I@srcdir@/gens_core/gfx/ \
-I@srcdir@/gens_core/cpu/sh2/ -I@srcdir@/gens_core/cpu/z80/ -I@srcdir@/gens_core/mem/

NASM += $(NASM_INCLUDES)
INCLUDES = -I@srcdir@/ -I@top_srcdir@/src/ -I@top_srcdir@/src/extlib/ -I@srcdir@/ui/
gens_CCASFLAGS = $(INCLUDES)

if GENS_ZLIB_INTERNAL
INCLUDES += -I@top_srcdir@/src/extlib/zlib/
endif # GENS_ZLIB_INTERNAL

if GENS_PNG_INTERNAL
INCLUDES += -I@top_srcdir@/src/extlib/libpng/
endif # GENS_PNG_INTERNAL

# x86 assembly language (using gcc)
%.o: %.S
	$(CC) -g -O2 -c $< -o $@

# x86 assembly language (using nasm)
%.o: %.asm
	mkdir -p gens_core/gfx/ gens_core/io/ gens_core/mem/ gens_core/misc/ ;
	mkdir -p gens_core/sound/ gens_core/cpu/sh2/ gens_core/vdp/ gens_core/cpu/z80/ ;
	$(NASM) $< -o $@

# Win32 resources
.rc.o:
	$(RC) -I@srcdir@ -I$(top_builddir) $(DEFS) $< -o $@
%.o: %.rc
	$(RC) -I@srcdir@ -I$(top_builddir) $(DEFS) $< -o $@

bin_PROGRAMS = gens

gens_SOURCES = \
		emulator/gens.cpp \
		emulator/g_main.cpp \
		emulator/g_md.cpp \
		emulator/g_mcd.cpp \
		emulator/g_32x.cpp \
		emulator/md_palette.cpp \
		emulator/g_update.cpp \
		emulator/parse.cpp \
		emulator/options.cpp \
		gens_core/nasmhead.inc \
		gens_core/cpu/68k/cpu_68k.c \
		gens_core/cpu/sh2/cpu_sh2.c \
		gens_core/cpu/sh2/sh2a.asm \
		gens_core/cpu/sh2/sh2_io.asm \
		gens_core/cpu/sh2/sh2.c \
		gens_core/cpu/z80/cpu_z80.c \
		gens_core/sound/pcm.c \
		gens_core/sound/psg.c \
		gens_core/sound/ym2612.cpp \
		gens_core/sound/pwm.c \
		gens_core/mem/mem_m68k.asm \
		gens_core/mem/mem_m68k_cd.asm \
		gens_core/mem/mem_m68k_32x.asm \
		gens_core/mem/mem_s68k.asm \
		gens_core/mem/mem_sh2.asm \
		gens_core/mem/mem_z80.c \
		gens_core/misc/misc.asm \
		gens_core/gfx/fastblur.cpp \
		gens_core/misc/cpuflags.c \
		$(gens_x86_asm_src) \
		gens_core/gfx/gfx_cd.asm \
		gens_core/io/io.c \
		gens_core/io/io_teamplayer.c \
		gens_core/vdp/vdp_io_x86.asm \
		gens_core/vdp/vdp_io.c \
		gens_core/vdp/vdp_rend_x86.asm \
		gens_core/vdp/vdp_rend.cpp \
		gens_core/vdp/vdp_32x_x86.asm \
		gens_core/vdp/vdp_32x.c \
		gens_core/vdp/vdp_32x_32bit.c \
		macros/log_msg.c \
		port/ini.cpp \
		segacd/cd_file.c \
		segacd/cd_sys.cpp \
		segacd/lc89510.c \
		ui/gens_ui.cpp \
		ui/common/gens/gens_menu.c \
		ui/common/gens/gens_menu_callbacks.cpp \
		ui/common/gens/gens_window_sync.c \
		ui/common/about_window_data.c \
		ui/common/dir_window_common.c \
		ui/common/pmgr_window_common.cpp \
		ui/common/bmf_window_common.c \
		util/file/config_file.cpp \
		util/file/rom.cpp \
		util/file/save.cpp \
		util/file/file.cpp \
		util/file/decompressor/decompressor.c \
		util/file/decompressor/dummy.c \
		util/file/decompressor/md_rar.cpp \
		util/file/decompressor/md_rar_t.c \
		util/gfx/imageutil.cpp \
		util/sound/gym.cpp \
		util/sound/wave.c \
		video/C64_charset.c \
		video/osd_charset.cpp \
		video/vdraw.c \
		video/vdraw_cpp.cpp \
		video/vdraw_text.cpp \
		video/v_effects.cpp \
		video/vdraw_RGB.c \
		input/input.c \
		input/input_update.c \
		audio/audio.c \
		plugins/pluginmgr.cpp \
		plugins/rendermgr.cpp \
		plugins/mdp_host_gens.c \
		plugins/mdp_host_gens_cpp.cpp \
		plugins/mdp_host_gens_menu.cpp \
		plugins/mdp_host_gens_event.cpp \
		plugins/mdp_host_gens_mem.c \
		plugins/mdp_host_gens_emuctrl.cpp \
		plugins/mdp_host_gens_z.c \
		plugins/mdp_host_gens_config.cpp \
		plugins/mdp_host_gens_dir.cpp \
		plugins/reg/reg_get.c \
		plugins/reg/reg_set.c \
		plugins/reg/reg_get_all.c \
		plugins/reg/reg_set_all.c \
		plugins/eventmgr.cpp \
		plugins/mdp_incompat.cpp

if !GENS_DEBUG
gens_SOURCES += \
		emulator/sighandler.c
endif

if GENS_DEBUGGER
gens_SOURCES += \
		debugger/debugger.cpp \
		debugger/m68kd.c \
		debugger/sh2d.c \
		debugger/z80dis.c \
		debugger/mcd_cell_dump.asm
ASMFILES += \
		debugger/mcd_cell_dump.o
endif

if GENS_MP3
gens_SOURCES += \
		segacd/cdda_mp3.c
endif

if GENS_ZLIB
gens_SOURCES += \
		util/file/decompressor/md_gzip.c \
		util/file/decompressor/md_zip.c
endif

if GENS_LZMA
gens_SOURCES += \
		util/file/decompressor/md_7z.c
endif

noinst_HEADERS = \
		emulator/gens.hpp \
		emulator/g_main.hpp \
		emulator/g_md.hpp \
		emulator/g_mcd.hpp \
		emulator/g_32x.hpp \
		emulator/md_palette.hpp \
		emulator/g_update.hpp \
		emulator/parse.hpp \
		emulator/options.hpp \
		gens_core/cpu/68k/cpu_68k.h \
		gens_core/cpu/68k/star_68k.h \
		gens_core/cpu/sh2/sh2.h \
		gens_core/cpu/sh2/cpu_sh2.h \
		gens_core/cpu/z80/cpu_z80.h \
		gens_core/sound/pcm.h \
		gens_core/sound/psg.h \
		gens_core/sound/ym2612.hpp \
		gens_core/sound/pwm.h \
		gens_core/mem/mem_m68k.h \
		gens_core/mem/mem_m68k_cd.h \
		gens_core/mem/mem_m68k_32x.h \
		gens_core/mem/mem_s68k.h \
		gens_core/mem/mem_sh2.h \
		gens_core/mem/mem_z80.h \
		gens_core/misc/misc.h \
		gens_core/gfx/fastblur.hpp \
		gens_core/misc/cpuflags.h \
		$(gens_x86_asm_h) \
		gens_core/gfx/gfx_cd.h \
		gens_core/io/io.h \
		gens_core/io/io_teamplayer.h \
		gens_core/vdp/vdp_io.h \
		gens_core/vdp/vdp_rend.h \
		gens_core/vdp/vdp_32x.h \
		gens_core/vdp/vdp_32x_32bit.h \
		macros/git.h \
		macros/log_msg.h \
		port/ini.hpp \
		segacd/cd_file.h \
		segacd/cd_sys.hpp \
		segacd/lc89510.h \
		ui/gens_ui.hpp \
		ui/common/gens/gens_menu.h \
		ui/common/gens/gens_menu_callbacks.cpp \
		ui/common/gens/gens_window_sync.h \
		ui/common/about_window_data.h \
		ui/common/dir_window_common.h \
		ui/common/pmgr_window_common.hpp \
		ui/common/bmf_window_common.h \
		util/file/config_file.hpp \
		util/file/rom.hpp \
		util/file/save.hpp \
		util/file/file.hpp \
		util/file/gsx_struct.h \
		util/file/gsx_v6.h \
		util/file/gsx_v7.h \
		util/file/decompressor/decompressor.h \
		util/file/decompressor/popen_wrapper.h \
		util/file/decompressor/dummy.h \
		util/file/decompressor/md_rar.hpp \
		util/file/decompressor/md_rar_t.h \
		util/gfx/imageutil.hpp \
		util/sound/gym.hpp \
		util/sound/wave.h \
		video/C64_charset.h \
		video/osd_charset.hpp \
		video/vdraw.h \
		video/vdraw_cpp.hpp \
		video/vdraw_text.hpp \
		video/v_effects.hpp \
		video/vdraw_RGB.h \
		input/input.h \
		input/input_update.h \
		audio/audio.h \
		plugins/pluginmgr.hpp \
		plugins/rendermgr.hpp \
		plugins/mdp_host_gens.h \
		plugins/mdp_host_gens_cpp.hpp \
		plugins/mdp_host_gens_menu.hpp \
		plugins/mdp_host_gens_event.hpp \
		plugins/mdp_host_gens_mem.h \
		plugins/mdp_host_gens_emuctrl.hpp \
		plugins/mdp_host_gens_z.h \
		plugins/mdp_host_gens_config.hpp \
		plugins/mdp_host_gens_dir.hpp \
		plugins/reg/reg_get.h \
		plugins/reg/reg_set.h \
		plugins/reg/reg_get_all.h \
		plugins/reg/reg_set_all.h \
		plugins/eventmgr.hpp \
		plugins/mdp_incompat.hpp

if !GENS_DEBUG
noinst_HEADERS += \
		emulator/sighandler.h
endif

if GENS_DEBUGGER
noinst_HEADERS += \
		debugger/debugger.hpp \
		debugger/m68kd.h \
		debugger/sh2d.h \
		debugger/z80dis.h \
		debugger/mcd_cell_dump.h
endif

if GENS_CDROM
noinst_HEADERS += \
		segacd/cd_aspi.hpp
endif

if GENS_MP3
noinst_HEADERS += \
		segacd/cdda_mp3.h
endif

if GENS_ZLIB
noinst_HEADERS += \
		util/file/decompressor/md_gzip.h \
		util/file/decompressor/md_zip.h
endif

if GENS_LZMA
noinst_HEADERS += \
		util/file/decompressor/md_7z.h
endif


# Statically-linked MDP plugins.
# TODO: Add built-in renderers automatically.
gens_mdp_static = \
		plugins/render/normal/libmdp_render_1x.la \
		plugins/render/double/libmdp_render_2x.la

gens_LDADD = \
		${ASMFILES} \
		${gens_mdp_static} \
		$(top_builddir)/src/starscream/libstarscream.la \
		$(top_builddir)/src/mdZ80/libmdZ80.la \
		$(top_builddir)/src/libgsft/libgsft.la \
		fw/libgens_fw.la \
		-lm @RT_LIBS@ @ICONV_LIBS@ @PNG_LIBS@

if GENS_MP3
gens_LDADD += $(top_builddir)/src/extlib/mp3_dec/libmp3_dec.la
endif 

if GENS_ZLIB
gens_LDADD += $(top_builddir)/src/extlib/minizip/libminizip.la
if GENS_ZLIB_INTERNAL
gens_LDADD += $(top_builddir)/src/extlib/zlib/libz.la
else
gens_LDADD += -lz
endif # GENS_ZLIB_INTERNAL
endif # GENS_ZLIB

if GENS_PNG
gens_LDADD += $(top_builddir)/src/libgsft/libgsft_png.la
if GENS_PNG_INTERNAL
gens_LDADD += $(top_builddir)/src/extlib/libpng/libpng.la
endif # GENS_PNG_INTERNAL
endif # GENS_PNG

if GENS_LZMA
gens_LDADD += $(top_builddir)/src/extlib/lzma/liblzma.la
endif # GENS_LZMA


## ARGH!! Get me some C4 and the address of the programmer who made me 
## have to do this!!

gens_DEPENDENCIES = \
		${ASMFILES} \
		${gens_mdp_static} \
		$(top_builddir)/src/starscream/libstarscream.la \
		$(top_builddir)/src/mdZ80/libmdZ80.la \
		$(top_builddir)/src/libgsft/libgsft.la \
		fw/libgens_fw.la

if GENS_MP3
gens_DEPENDENCIES += $(top_builddir)/src/extlib/mp3_dec/libmp3_dec.la
endif

if GENS_ZLIB
gens_DEPENDENCIES += $(top_builddir)/src/extlib/minizip/libminizip.la
if GENS_ZLIB_INTERNAL
gens_DEPENDENCIES += $(top_builddir)/src/extlib/zlib/libz.la
endif # GENS_ZLIB_INTERNAL
endif # GENS_ZLIB

if GENS_PNG
gens_DEPENDENCIES += $(top_builddir)/src/libgsft/libgsft_png.la
if GENS_PNG_INTERNAL
gens_DEPENDENCIES += $(top_builddir)/src/extlib/libpng/libpng.la
endif # GENS_PNG_INTERNAL
endif # GENS_PNG

if GENS_LZMA
gens_DEPENDENCIES += $(top_builddir)/src/extlib/lzma/liblzma.la
endif # GENS_LZMA

CLEANFILES = ${ASMFILES}

# Initialize CFLAGS, CXXFLAGS, and LDFLAGS.
gens_CFLAGS	= $(AM_CFLAGS)
gens_CXXFLAGS	= $(AM_CXXFLAGS)
gens_LDFLAGS	= $(AM_LDFLAGS)

# OS-specific includes

if GENS_OS_UNIX
include $(srcdir)/Makefile.unix.inc.am
endif # GENS_OS_UNIX

if GENS_OS_WIN32
include $(srcdir)/Makefile.win32.inc.am
endif # GENS_OS_WIN32

if GENS_OS_HAIKU
include $(srcdir)/Makefile.haiku.inc.am
endif # GENS_OS_HAIKU
