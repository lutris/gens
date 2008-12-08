# MDP Render Plugin: hq2x, hq3x, and hq4x renderers

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_hq2x_16_x86.o \
		mdp_render_hq3x_16_x86.o \
		mdp_render_hq4x_16_x86.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdp_render_hq2x.a

libmdp_render_hq2x_a_SOURCES = \
		mdp_render_hq2x_plugin.c \
		mdp_render_hq2x.c \
		mdp_render_hq2x_lut.c \
		mdp_render_hq2x_16_x86.asm \
		mdp_render_hq3x_plugin.c \
		mdp_render_hq3x.c \
		mdp_render_hq3x_16_x86.asm \
		mdp_render_hq4x_plugin.c \
		mdp_render_hq4x.c \
		mdp_render_hq4x_16_x86.asm

noinst_HEADERS = \
		mdp_render_hq2x_plugin.h \
		mdp_render_hq2x.h \
		mdp_render_hq2x_lut.h \
		mdp_render_hq2x_x86.h \
		mdp_render_hq3x_plugin.h \
		mdp_render_hq3x.h \
		mdp_render_hq3x_x86.h \
		mdp_render_hq4x_plugin.h \
		mdp_render_hq4x.h \
		mdp_render_hq4x_x86.h

libmdp_render_hq2x_a_LIBADD = \
		$(ASMFILES)

libmdp_render_hq2x_a_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
