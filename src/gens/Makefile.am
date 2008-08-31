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
		gens_core/gfx/blit_hq2x_16.o
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
		gens_core/gfx/blit_hq2x_16.asm
gens_x86_asm_h =
else
gens_x86_asm_o =
gens_x86_asm_src =
gens_x86_asm_h =
endif

ASMFILES = \
		$(gens_x86_asm_o) \
		gens_core/gfx/gfx_cd.o \
		gens_core/mem/mem_m68k.o \
		gens_core/mem/mem_s68k.o \
		gens_core/mem/mem_sh2.o \
		gens_core/mem/mem_z80.o \
		gens_core/misc/misc.o \
		gens_core/io/io_old.o \
		gens_core/sound/pwm.o \
		gens_core/cpu/sh2/sh2a.o \
		gens_core/vdp/vdp_32x.o \
		gens_core/vdp/vdp_io.o \
		gens_core/vdp/vdp_rend.o \
		gens_core/cpu/z80/z80.o

NASM = \
nasm -O1 -D __GCC2 -f elf \
-I@srcdir@/gens_core/ -I@srcdir@/gens_core/io/ -I@srcdir@/gens_core/misc/ \
-I@srcdir@/gens_core/vdp/ -I@srcdir@/gens_core/sound/ -I@srcdir@/gens_core/gfx/ \
-I@srcdir@/gens_core/cpu/sh2/ -I@srcdir@/gens_core/cpu/z80/ -I@srcdir@/gens_core/mem/

INCLUDES = -I@srcdir@ -I@srcdir@/../ -I@srcdir@/ui/ -I@srcdir@/ui/gtk/

%.o: %.asm
	mkdir -p gens_core/gfx/ gens_core/io/ gens_core/mem/ gens_core/misc/ ;
	mkdir -p gens_core/sound/ gens_core/cpu/sh2/ gens_core/vdp/ gens_core/cpu/z80/ ;
	$(NASM) $< -o $@

bin_PROGRAMS = gens
gens_SOURCES = \
		emulator/gens.c \
		emulator/g_main.c \
		emulator/g_main_linux.c \
		emulator/g_input.c \
		emulator/g_md.c \
		emulator/g_mcd.c \
		emulator/g_32x.c \
		emulator/g_32x_32bit.c \
		emulator/g_palette.c \
		emulator/g_update.c \
		emulator/parse.c \
		emulator/ui_proxy.c \
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
		$(gens_x86_asm_src) \
		gens_core/gfx/blit1x.c \
		gens_core/gfx/blit2x.c \
		gens_core/gfx/scale2x.c \
		gens_core/gfx/hq2x_funcs.c \
		gens_core/gfx/renderers.c \
		gens_core/gfx/gfx_cd.asm \
		gens_core/io/io.c \
		gens_core/io/io.asm \
		gens_core/vdp/vdp_io.asm \
		gens_core/vdp/vdp_rend.asm \
		gens_core/vdp/vdp_32x.asm \
		port/port.c \
		port/timer.c \
		sdllayer/g_sdldraw.c \
		sdllayer/g_sdlinput.c \
		sdllayer/g_sdlsound.c \
		sdllayer/g_effects.c \
		segacd/cd_file.c \
		segacd/cd_sys.c \
		segacd/cdda_mp3.c \
		segacd/lc89510.c \
		ui/ui-common.c \
		util/file/chd.c \
		util/file/config_file.c \
		util/file/rom.cpp \
		util/file/save.c \
		util/file/ggenie.c \
		util/file/unzip.c \
		util/file/compress/compressor.cpp \
		util/file/compress/subcompressor.cpp \
		util/file/compress/gzip.cpp \
		util/file/compress/Zip.cpp \
		util/file/compress/7z.cpp \
		util/gfx/scrshot.c \
		util/sound/gym.c \
		util/sound/wave.c
##		netplay/ccnet.c 
##		netplay/ccnet.h

if GENS_DEBUGGER
gens_SOURCES += \
		debugger/debugger.c \
		debugger/m68kd.c \
		debugger/sh2d.c \
		debugger/z80dis.c
endif

if GENS_OPENGL
gens_SOURCES += \
		sdllayer/g_opengl.c
endif

if GENS_CDROM
gens_SOURCES += \
		segacd/cd_aspi.c
endif

if LINUX
gens_SOURCES += \
		ui/gtk/ui-common_gtk.c \
		ui/gtk/gtk-misc.c \
		ui/gtk/gens/gens_window.c \
		ui/gtk/gens/gens_window_callbacks.c \
		ui/gtk/gens/gens_window_sync.c \
		ui/gtk/game_genie/game_genie_window.c \
		ui/gtk/game_genie/game_genie_window_callbacks.c \
		ui/gtk/game_genie/game_genie_window_misc.c \
		ui/gtk/controller_config/controller_config_window.c \
		ui/gtk/controller_config/controller_config_window_callbacks.c \
		ui/gtk/controller_config/controller_config_window_misc.c \
		ui/gtk/bios_misc_files/bios_misc_files_window.c \
		ui/gtk/bios_misc_files/bios_misc_files_window_callbacks.c \
		ui/gtk/bios_misc_files/bios_misc_files_window_misc.c \
		ui/gtk/directory_config/directory_config_window.c \
		ui/gtk/directory_config/directory_config_window_callbacks.c \
		ui/gtk/directory_config/directory_config_window_misc.c \
		ui/gtk/general_options/general_options_window.c \
		ui/gtk/general_options/general_options_window_callbacks.c \
		ui/gtk/general_options/general_options_window_misc.c \
		ui/gtk/color_adjust/color_adjust_window.c \
		ui/gtk/color_adjust/color_adjust_window_callbacks.c \
		ui/gtk/color_adjust/color_adjust_window_misc.c \
		ui/gtk/country_code/country_code_window.c \
		ui/gtk/country_code/country_code_window_callbacks.c \
		ui/gtk/country_code/country_code_window_misc.c \
		ui/gtk/zip_select/zip_select_dialog.c \
		ui/gtk/zip_select/zip_select_dialog_misc.cpp \
		ui/gtk/about/about_window.c

if GENS_OPENGL
gens_SOURCES += \
		ui/gtk/opengl_resolution/opengl_resolution_window.c \
		ui/gtk/opengl_resolution/opengl_resolution_window_callbacks.c \
		ui/gtk/opengl_resolution/opengl_resolution_window_misc.c
endif

if GENS_CDROM
gens_SOURCES += \
		ui/gtk/select_cdrom/select_cdrom_window.c \
		ui/gtk/select_cdrom/select_cdrom_window_callbacks.c \
		ui/gtk/select_cdrom/select_cdrom_window_misc.c
endif

endif

noinst_HEADERS = \
		emulator/gens.h \
		emulator/g_main.h \
		emulator/g_main_linux.h \
		emulator/g_input.h \
		emulator/g_input_linux.h \
		emulator/g_md.h \
		emulator/g_mcd.h \
		emulator/g_32x.h \
		emulator/g_32x_32bit.h \
		emulator/g_palette.h \
		emulator/g_update.h \
		emulator/gens.h \
		emulator/parse.h \
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
		$(gens_x86_asm_h) \
		gens_core/gfx/blit.h \
		gens_core/gfx/gfx_cd.h \
		gens_core/gfx/renderers.h \
		gens_core/io/io.h \
		gens_core/io/io_old.h \
		gens_core/vdp/vdp_32x.h \
		gens_core/vdp/vdp_io.h \
		gens_core/vdp/vdp_rend.h \
		port/port.h \
		port/timer.h \
		sdllayer/g_sdldraw.h \
		sdllayer/g_sdlinput.h \
		sdllayer/g_sdlsound.h \
		sdllayer/g_effects.h \
		segacd/cd_file.h \
		segacd/cd_sys.h \
		segacd/cdda_mp3.h \
		segacd/lc89510.h \
		ui/ui-common.h \
		util/file/chd.h \
		util/file/config_file.h \
		util/file/rom.hpp \
		util/file/save.h \
		util/file/ggenie.h \
		util/file/unzip.h \
		util/file/compress/compressor.hpp \
		util/file/compress/subcompressor.hpp \
		util/file/compress/gzip.hpp \
		util/file/compress/Zip.hpp \
		util/file/compress/7z.hpp \
		util/gfx/scrshot.h \
		util/sound/gym.c \
		util/sound/wave.c

if GENS_DEBUGGER
noinst_HEADERS += \
		debugger/debugger.h \
		debugger/m68kd.h \
		debugger/sh2d.h \
		debugger/z80dis.h
endif

if GENS_OPENGL
noinst_HEADERS += \
		sdllayer/g_opengl.h
endif

if GENS_CDROM
noinst_HEADERS += \
		segacd/cd_aspi.h
endif

if LINUX
noinst_HEADERS += \
		ui/gtk/gtk-misc.h \
		ui/gtk/gens/gens_window.h \
		ui/gtk/gens/gens_window_callbacks.h \
		ui/gtk/gens/gens_window_sync.h \
		ui/gtk/game_genie/game_genie_window.h \
		ui/gtk/game_genie/game_genie_window_callbacks.h \
		ui/gtk/game_genie/game_genie_window_misc.h \
		ui/gtk/controller_config/controller_config_window.h \
		ui/gtk/controller_config/controller_config_window_callbacks.h \
		ui/gtk/controller_config/controller_config_window_misc.h \
		ui/gtk/bios_misc_files/bios_misc_files_window.h \
		ui/gtk/bios_misc_files/bios_misc_files_window_callbacks.h \
		ui/gtk/bios_misc_files/bios_misc_files_window_misc.h \
		ui/gtk/directory_config/directory_config_window.h \
		ui/gtk/directory_config/directory_config_window_callbacks.h \
		ui/gtk/directory_config/directory_config_window_misc.h \
		ui/gtk/general_options/general_options_window.h \
		ui/gtk/general_options/general_options_window_callbacks.h \
		ui/gtk/general_options/general_options_window_misc.h \
		ui/gtk/color_adjust/color_adjust_window.h \
		ui/gtk/color_adjust/color_adjust_window_callbacks.h \
		ui/gtk/color_adjust/color_adjust_window_misc.h \
		ui/gtk/country_code/country_code_window.h \
		ui/gtk/country_code/country_code_window_callbacks.h \
		ui/gtk/country_code/country_code_window_misc.h \
		ui/gtk/zip_select/zip_select_dialog.h \
		ui/gtk/zip_select/zip_select_dialog_misc.hpp \
		ui/gtk/about/about_window.h

if GENS_OPENGL
noinst_HEADERS += \
		ui/gtk/opengl_resolution/opengl_resolution_window.h \
		ui/gtk/opengl_resolution/opengl_resolution_window_callbacks.h \
		ui/gtk/opengl_resolution/opengl_resolution_window_misc.h
endif

if GENS_CDROM
noinst_HEADERS += \
		ui/gtk/select_cdrom/select_cdrom_window.h \
		ui/gtk/select_cdrom/select_cdrom_window_callbacks.h \
		ui/gtk/select_cdrom/select_cdrom_window_misc.h
endif

endif

gens_CFLAGS = \
		-DDATADIR=\"@datadir@/@PACKAGE@\" \
		@SDL_CFLAGS@ \
		@GTK_CFLAGS@ \
		@CFLAGS@

gens_CXXFLAGS = \
		-DDATADIR=\"@datadir@/@PACKAGE@\" \
		@SDL_CFLAGS@ \
		@GTK_CFLAGS@ \
		@CFLAGS@ \
		@CXXFLAGS@


gens_LDADD = \
		${ASMFILES} \
		../mp3_dec/libmp3_dec.a \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o \
		-lm -lz @SDL_LIBS@ @GTK_LIBS@

## ARGH!! Get me some C4 and the address of the programmer who made me 
## have to do this!!


gens_DEPENDENCIES = \
		${ASMFILES} \
		../mp3_dec/libmp3_dec.a \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o

CLEANFILES = \
		${ASMFILES} \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o
