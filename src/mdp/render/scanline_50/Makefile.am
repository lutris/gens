# MDP Render Plugin: 50% Scanline renderer

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_scanline_50_16_x86.o \
		mdp_render_scanline_50_32_x86.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdp_render_scanline_50.a

libmdp_render_scanline_50_a_SOURCES = \
		mdp_render_scanline_50_plugin.c \
		mdp_render_scanline_50.cpp \
		mdp_render_scanline_50_16_x86.asm \
		mdp_render_scanline_50_32_x86.asm

noinst_HEADERS = \
		mdp_render_scanline_50_plugin.h \
		mdp_render_scanline_50_icon.h \
		mdp_render_scanline_50.hpp \
		mdp_render_scanline_50_x86.h

libmdp_render_scanline_50_a_LIBADD = \
		$(ASMFILES)

libmdp_render_scanline_50_a_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
