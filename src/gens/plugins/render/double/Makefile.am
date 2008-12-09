# MDP Render Plugin: Double renderer

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = \
		mdp_render_2x_16_x86.o \
		mdp_render_2x_32_x86.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdp_render_2x.a

libmdp_render_2x_a_SOURCES = \
		mdp_render_2x_plugin.c \
		mdp_render_2x.cpp \
		mdp_render_2x_16_x86.asm \
		mdp_render_2x_32_x86.asm

noinst_HEADERS = \
		mdp_render_2x_plugin.h \
		mdp_render_2x_icon.h \
		mdp_render_2x.hpp \
		mdp_render_2x_x86.h

if GENS_X86_ASM
libmdp_render_2x_a_LIBADD = $(ASMFILES)
libmdp_render_2x_a_DEPENDENCIES = $(ASMFILES)
CLEANFILES = $(ASMFILES)
else # !GENS_X86_ASM
libmdp_render_2x_a_LIBADD =
libmdp_render_2x_a_DEPENDENCIES =
CLEANFILES =
endif # GENS_X86_ASM
