## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = \
-I@srcdir@/../../gens_core/cpu/68k -I@srcdir@/../../gens_core/cpu/sh2 -I@srcdir@/../../gens_core/cpu/z80 \
-I@srcdir@/../../gens_core/sound -I@srcdir@/../../gens_core/mem -I@srcdir@/../../gens_core/misc -I@srcdir@/../../gens_core/gfx \
-I@srcdir@/../../gens_core/io -I@srcdir@/../../gens_core/vdp -I@srcdir@/../../segacd -I@srcdir@/../../mp3_dec -I@srcdir@/../../sdllayer \
-I@srcdir@/../../util -I@srcdir@/../../util/gfx -I@srcdir@/../../util/sound -I@srcdir@/../../util/file \
-I@srcdir@/../../port -I@srcdir@/../../emulator -I@srcdir@/../../debug -I@srcdir@/../../netplay \
-I@srcdir@/../../ui -I@srcdir@/../../ui/gtk \
-I@srcdir@/../../ -I@srcdir@/../ -I@srcdir@

lib_LIBRARIES = libgens_ui_gtk.a

libgens_ui_gtk_a_SOURCES = \
		ui-common_gtk.c \
		gtk-misc.c \
		gens/gens_window.c \
		gens/gens_window_callbacks.c \
		gens/gens_window_sync.c \
		game_genie/game_genie_window.c \
		game_genie/game_genie_window_callbacks.c \
		game_genie/game_genie_window_misc.c \
		controller_config/controller_config_window.c \
		controller_config/controller_config_window_callbacks.c \
		controller_config/controller_config_window_misc.c \
		bios_misc_files/bios_misc_files_window.c \
		bios_misc_files/bios_misc_files_window_callbacks.c \
		bios_misc_files/bios_misc_files_window_misc.c \
		directory_config/directory_config_window.c \
		directory_config/directory_config_window_callbacks.c \
		directory_config/directory_config_window_misc.c \
		general_options/general_options_window.c \
		general_options/general_options_window_callbacks.c \
		general_options/general_options_window_misc.c \
		color_adjust/color_adjust_window.c \
		color_adjust/color_adjust_window_callbacks.c \
		color_adjust/color_adjust_window_misc.c \
		opengl_resolution/opengl_resolution_window.c \
		opengl_resolution/opengl_resolution_window_callbacks.c \
		opengl_resolution/opengl_resolution_window_misc.c \
		select_cdrom/select_cdrom_window.c \
		select_cdrom/select_cdrom_window_callbacks.c \
		select_cdrom/select_cdrom_window_misc.c \
		country_code/country_code_window.c \
		country_code/country_code_window_callbacks.c \
		country_code/country_code_window_misc.c \
		about/about_window.c

noinst_HEADERS = \
		gtk-misc.h \
		gens/gens_window.h \
		gens/gens_window_callbacks.h \
		gens/gens_window_sync.h \
		game_genie/game_genie_window.h \
		game_genie/game_genie_window_callbacks.h \
		game_genie/game_genie_window_misc.h \
		controller_config/controller_config_window.h \
		controller_config/controller_config_window_callbacks.h \
		controller_config/controller_config_window_misc.h \
		bios_misc_files/bios_misc_files_window.h \
		bios_misc_files/bios_misc_files_window_callbacks.h \
		bios_misc_files/bios_misc_files_window_misc.h \
		directory_config/directory_config_window.h \
		directory_config/directory_config_window_callbacks.h \
		directory_config/directory_config_window_misc.h \
		general_options/general_options_window.h \
		general_options/general_options_window_callbacks.h \
		general_options/general_options_window_misc.h \
		color_adjust/color_adjust_window.h \
		color_adjust/color_adjust_window_callbacks.h \
		color_adjust/color_adjust_window_misc.h \
		opengl_resolution/opengl_resolution_window.h \
		opengl_resolution/opengl_resolution_window_callbacks.h \
		opengl_resolution/opengl_resolution_window_misc.h \
		select_cdrom/select_cdrom_window.h \
		select_cdrom/select_cdrom_window_callbacks.h \
		select_cdrom/select_cdrom_window_misc.h \
		country_code/country_code_window.h \
		country_code/country_code_window_callbacks.h \
		country_code/country_code_window_misc.h \
		about/about_window.h

libgens_ui_gtk_a_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@
