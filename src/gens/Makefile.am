## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		gens_core/gfx/blit.o \
		gens_core/gfx/hq2x16.o \
		gens_core/gfx/gfx_cd.o \
		gens_core/io/io.o \
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


NASM = \
nasm -O1 -D __GCC2 -f elf \
-I@srcdir@/gens_core/ -I@srcdir@/gens_core/io/ -I@srcdir@/gens_core/misc/ \
-I@srcdir@/gens_core/vdp/ -I@srcdir@/gens_core/sound/ -I@srcdir@/gens_core/gfx/ \
-I@srcdir@/gens_core/cpu/sh2/ -I@srcdir@/gens_core/cpu/z80/ -I@srcdir@/gens_core/mem/

%.o: %.asm
	mkdir -p gens_core/gfx gens_core/io gens_core/mem gens_core/misc gens_core/sound gens_core/cpu/sh2 gens_core/vdp gens_core/cpu/z80 ;
	$(NASM) $< -o $@

INCLUDES = \
-I@srcdir@/gens_core/cpu/68k -I@srcdir@/gens_core/cpu/sh2 -I@srcdir@/gens_core/cpu/z80 \
-I@srcdir@/gens_core/sound -I@srcdir@/gens_core/mem -I@srcdir@/gens_core/misc -I@srcdir@/gens_core/gfx \
-I@srcdir@/gens_core/io -I@srcdir@/gens_core/vdp -I@srcdir@/segacd -I@srcdir@/mp3_dec -I@srcdir@/sdllayer \
-I@srcdir@/util -I@srcdir@/util/gfx -I@srcdir@/util/sound -I@srcdir@/util/file \
-I@srcdir@/port -I@srcdir@/emulator -I@srcdir@/debug -I@srcdir@/netplay \
-I@srcdir@/ui -I@srcdir@/ui/gtk \
-I@srcdir@

bin_PROGRAMS = gens
gens_SOURCES = \
		gens_core/nasmhead.inc \
		gens_core/cpu/68k/cpu_68k.c \
		gens_core/cpu/68k/cpu_68k.h \
		gens_core/cpu/68k/star_68k.h \
		gens_core/cpu/sh2/cpu_sh2.c \
		gens_core/cpu/sh2/sh2a.asm \
		gens_core/cpu/sh2/sh2.c \
		gens_core/cpu/sh2/sh2.h \
		gens_core/cpu/sh2/cpu_sh2.h \
		gens_core/cpu/sh2/sh2_io.inc \
		gens_core/cpu/z80/z80.asm \
		gens_core/cpu/z80/cpu_z80.c \
		gens_core/cpu/z80/cpu_z80.h \
		gens_core/sound/pcm.c \
		gens_core/sound/pcm.h \
		gens_core/sound/psg.c \
		gens_core/sound/psg.h \
		gens_core/sound/ym2612.c \
		gens_core/sound/ym2612.h \
		gens_core/sound/pwm.asm \
		gens_core/sound/pwm.h \
		gens_core/sound/z80.h \
		gens_core/mem/mem_m68k.asm \
		gens_core/mem/mem_m68k.h \
		gens_core/mem/mem_s68k.asm \
		gens_core/mem/mem_s68k.h \
		gens_core/mem/mem_sh2.asm \
		gens_core/mem/mem_sh2.h \
		gens_core/mem/mem_z80.asm \
		gens_core/mem/mem_z80.h \
		gens_core/mem/mem_m68k_32x.inc \
		gens_core/mem/mem_m68k_cd.inc \
		gens_core/misc/byteswap.c \
		gens_core/misc/byteswap.h \
		gens_core/misc/misc.asm \
		gens_core/misc/misc.h \
		gens_core/gfx/blit.asm \
		gens_core/gfx/blit.h \
		gens_core/gfx/blit2.c \
		gens_core/gfx/blit2.h \
		gens_core/gfx/hqfilters.c \
		gens_core/gfx/hq2x16.asm \
		gens_core/gfx/gfx_cd.asm \
		gens_core/gfx/gfx_cd.h \
		gens_core/io/io.asm \
		gens_core/io/io.h \
		gens_core/vdp/vdp_32x.h \
		gens_core/vdp/vdp_io.asm \
		gens_core/vdp/vdp_io.h \
		gens_core/vdp/vdp_rend.asm \
		gens_core/vdp/vdp_rend.h \
		gens_core/vdp/vdp_32x.asm \
		segacd/cd_aspi.c \
		segacd/cd_aspi.h \
		segacd/cd_file.c \
		segacd/cd_file.h \
		segacd/cd_sys.c \
		segacd/cd_sys.h \
		segacd/cdda_mp3.c \
		segacd/cdda_mp3.h \
		segacd/lc89510.c \
		segacd/lc89510.h \
		mp3_dec/common.c \
		mp3_dec/dct64_i386.c \
		mp3_dec/decode_i386.c \
		mp3_dec/interface.c \
		mp3_dec/layer3.c \
		mp3_dec/tabinit.c \
		mp3_dec/huffman.h \
		mp3_dec/mpg123.h \
		mp3_dec/mpglib.h \
		sdllayer/g_sdldraw.c \
		sdllayer/g_ddraw.h \
		sdllayer/g_sdlinput.c \
		sdllayer/g_input.h \
		sdllayer/g_sdlsound.c \
		sdllayer/g_dsound.h \
		util/file/chd.c \
		util/file/chd.h \
		util/file/config_file.c \
		util/file/config_file.h \
		util/file/rom.c \
		util/file/rom.h \
		util/file/save.c \
		util/file/save.h \
		util/file/ggenie.c \
		util/file/ggenie.h \
		util/file/compress/compress.c \
		util/file/compress/compress.h \
		util/file/compress/gzip.c \
		util/file/compress/gzip.h \
		util/file/compress/dummy.c \
		util/file/compress/dummy.h \
		util/file/compress/Zip.c \
		util/file/compress/Zip.h \
		util/sound/gym.c \
		util/sound/gym.h \
		util/gfx/scrshot.c \
		util/gfx/scrshot.h \
		util/file/unzip.c \
		util/file/unzip.h \
		util/sound/wave.c \
		util/sound/wave.h \
		port/port.c \
		port/port.h \
		port/timer.c \
		port/timer.h \
		emulator/ui_proxy.c \
		emulator/ui_proxy.h \
		emulator/g_main.c \
		emulator/g_main.h \
		emulator/g_main_linux.c \
		emulator/g_main_linux.h \
		emulator/g_input.c \
		emulator/g_input.h \
		emulator/g_input_linux.h \
		emulator/g_md.c \
		emulator/g_md.h \
		emulator/g_mcd.c \
		emulator/g_mcd.h \
		emulator/g_32x.c \
		emulator/g_32x.h \
		emulator/g_palette.c \
		emulator/g_palette.h \
		emulator/gens.c \
		emulator/gens.h \
		debug/debug.c \
		debug/debug.h \
		debug/m68kd.c \
		debug/m68kd.h \
		debug/sh2d.c \
		debug/sh2d.h \
		debug/z80dis.c \
		debug/z80dis.h \
		ui/ui-common.c \
		ui/ui-common.h \
		ui/gtk/ui-common_gtk.c \
		ui/gtk/ui-common_gtk.h \
		ui/gtk/gtk-misc.c \
		ui/gtk/gtk-misc.h \
		ui/gtk/gens/gens_window.c \
		ui/gtk/gens/gens_window.h \
		ui/gtk/gens/gens_window_callbacks.c \
		ui/gtk/gens/gens_window_callbacks.h \
		ui/gtk/gens/gens_window_sync.c \
		ui/gtk/gens/gens_window_sync.h \
		ui/gtk/game_genie/game_genie_window.c \
		ui/gtk/game_genie/game_genie_window.h \
		ui/gtk/game_genie/game_genie_window_callbacks.c \
		ui/gtk/game_genie/game_genie_window_callbacks.h \
		ui/gtk/game_genie/game_genie_window_misc.c \
		ui/gtk/game_genie/game_genie_window_misc.h \
		ui/gtk/controller_config/controller_config_window.c \
		ui/gtk/controller_config/controller_config_window.h \
		ui/gtk/controller_config/controller_config_window_callbacks.c \
		ui/gtk/controller_config/controller_config_window_callbacks.h \
		ui/gtk/controller_config/controller_config_window_misc.c \
		ui/gtk/controller_config/controller_config_window_misc.h \
		ui/gtk/bios_misc_files/bios_misc_files_window.c \
		ui/gtk/bios_misc_files/bios_misc_files_window.h \
		ui/gtk/bios_misc_files/bios_misc_files_window_callbacks.c \
		ui/gtk/bios_misc_files/bios_misc_files_window.callbacks.h \
		ui/gtk/bios_misc_files/bios_misc_files_window_misc.c \
		ui/gtk/bios_misc_files/bios_misc_files_window_misc.h \
		ui/gtk/directory_config/directory_config_window.c \
		ui/gtk/directory_config/directory_config_window.h \
		ui/gtk/directory_config/directory_config_window_callbacks.c \
		ui/gtk/directory_config/directory_config_window.callbacks.h \
		ui/gtk/directory_config/directory_config_window_misc.c \
		ui/gtk/directory_config/directory_config_window_misc.h \
		ui/gtk/general_options/general_options_window.c \
		ui/gtk/general_options/general_options_window.h \
		ui/gtk/general_options/general_options_window_callbacks.c \
		ui/gtk/general_options/general_options_window_callbacks.h \
		ui/gtk/general_options/general_options_window_misc.c \
		ui/gtk/general_options/general_options_window_misc.h \
		ui/gtk/color_adjust/color_adjust_window.c \
		ui/gtk/color_adjust/color_adjust_window.h \
		ui/gtk/color_adjust/color_adjust_window_callbacks.c \
		ui/gtk/color_adjust/color_adjust_window_callbacks.h \
		ui/gtk/color_adjust/color_adjust_window_misc.c \
		ui/gtk/color_adjust/color_adjust_window_misc.h \
		ui/gtk/opengl_resolution/opengl_resolution_window.c \
		ui/gtk/opengl_resolution/opengl_resolution_window.h \
		ui/gtk/opengl_resolution/opengl_resolution_window_callbacks.c \
		ui/gtk/opengl_resolution/opengl_resolution_window_callbacks.h \
		ui/gtk/opengl_resolution/opengl_resolution_window_misc.c \
		ui/gtk/opengl_resolution/opengl_resolution_window_misc.h \
		ui/gtk/select_cdrom/select_cdrom_window.c \
		ui/gtk/select_cdrom/select_cdrom_window.h \
		ui/gtk/select_cdrom/select_cdrom_window_callbacks.c \
		ui/gtk/select_cdrom/select_cdrom_window_callbacks.h \
		ui/gtk/select_cdrom/select_cdrom_window_misc.c \
		ui/gtk/select_cdrom/select_cdrom_window_misc.h \
		ui/gtk/country_code/country_code_window.c \
		ui/gtk/country_code/country_code_window.h \
		ui/gtk/country_code/country_code_window_callbacks.c \
		ui/gtk/country_code/country_code_window_callbacks.h \
		ui/gtk/country_code/country_code_window_misc.c \
		ui/gtk/country_code/country_code_window_misc.h \
		ui/gtk/about/about_window.c \
		ui/gtk/about/about_window.h \
		emulator/parse.c \
		emulator/parse.h
##		netplay/ccnet.c \
##		netplay/ccnet.h
		
gens_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@


gens_LDADD = \
		$(ASMFILES) \
		../starscream/main68k/main68k.o \
		../starscream/sub68k/sub68k.o \
		-lm -lz @SDL_LIBS@ @GTK_LIBS@

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
