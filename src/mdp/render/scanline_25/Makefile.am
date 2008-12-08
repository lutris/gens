# MDP Render Plugin: 25% Scanline renderer

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_scanline_25_16_x86.o \
		mdp_render_scanline_25_32_x86.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdp_render_scanline_25.a

libmdp_render_scanline_25_a_SOURCES = \
		mdp_render_scanline_25_plugin.c \
		mdp_render_scanline_25.cpp \
		mdp_render_scanline_25_16_x86.asm \
		mdp_render_scanline_25_32_x86.asm

noinst_HEADERS = \
		mdp_render_scanline_25_plugin.h \
		mdp_render_scanline_25_icon.h \
		mdp_render_scanline_25.hpp \
		mdp_render_scanline_25_x86.h

libmdp_render_scanline_25_a_LIBADD = \
		$(ASMFILES)

libmdp_render_scanline_25_a_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
