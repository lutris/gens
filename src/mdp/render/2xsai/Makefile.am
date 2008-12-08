# MDP Render Plugin: 2xSaI renderer

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_2xsai_16_x86.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdp_render_2xsai.a

libmdp_render_2xsai_a_SOURCES = \
		mdp_render_2xsai_plugin.c \
		mdp_render_2xsai.c \
		mdp_render_2xsai_16_x86.asm

noinst_HEADERS = \
		mdp_render_2xsai_plugin.h \
		mdp_render_2xsai.h \
		mdp_render_2xsai_x86.h

libmdp_render_2xsai_a_LIBADD = \
		$(ASMFILES)

libmdp_render_2xsai_a_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
