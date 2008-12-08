# MDP Render Plugin: Interpolated Scanline renderer

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_interpolated_scanline_16_x86.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdp_render_interpolated_scanline.a

libmdp_render_interpolated_scanline_a_SOURCES = \
		mdp_render_interpolated_scanline_plugin.c \
		mdp_render_interpolated_scanline.cpp \
		mdp_render_interpolated_scanline_16_x86.asm

noinst_HEADERS = \
		mdp_render_interpolated_scanline_plugin.h \
		mdp_render_interpolated_scanline_icon.h \
		mdp_render_interpolated_scanline.hpp \
		mdp_render_interpolated_scanline_x86.h

libmdp_render_interpolated_scanline_a_LIBADD = \
		$(ASMFILES)

libmdp_render_interpolated_scanline_a_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
