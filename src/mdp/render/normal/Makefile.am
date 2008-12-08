# MDP Render Plugin: Normal renderer

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_1x_16_x86.o \
		mdp_render_1x_32_x86.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdp_render_1x.a

libmdp_render_1x_a_SOURCES = \
		mdp_render_1x_plugin.c \
		mdp_render_1x.cpp \
		mdp_render_1x_16_x86.asm \
		mdp_render_1x_32_x86.asm

noinst_HEADERS = \
		mdp_render_1x_plugin.h \
		mdp_render_1x_icon.h \
		mdp_render_1x.hpp \
		mdp_render_1x_x86.h

libmdp_render_1x_a_LIBADD = \
		$(ASMFILES)

libmdp_render_1x_a_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
