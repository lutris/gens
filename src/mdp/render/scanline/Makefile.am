# MDP Render Plugin: Scanline renderer

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_scanline_16_x86.o \
		mdp_render_scanline_32_x86.o

%.o: %.asm
	$(NASM) $< -o $@

lib_LTLIBRARIES = libmdp_render_scanline.la

libmdp_render_scanline_la_LDFLAGS = -module

libmdp_render_scanline_la_SOURCES = \
		mdp_render_scanline_plugin.c \
		mdp_render_scanline.cpp \
		mdp_render_scanline_16_x86.asm \
		mdp_render_scanline_32_x86.asm

noinst_HEADERS = \
		mdp_render_scanline_plugin.h \
		mdp_render_scanline_icon.h \
		mdp_render_scanline.hpp \
		mdp_render_scanline_x86.h

libmdp_render_scanline_la_LIBADD = \
		$(ASMFILES)

libmdp_render_scanline_la_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
