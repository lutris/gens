# MDP Render Plugin: 25% Scanline renderer

# libtool/nasm stuff from http://osdir.com/ml/sysutils.automake.general/2002-08/msg00134.html
# strip_fPIC.sh from http://networkmultimedia.org/

AUTOMAKE_OPTIONS = subdir-objects

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

lib_LTLIBRARIES = mdp_render_scanline_25.la

mdp_render_scanline_25_la_LDFLAGS = -module -no-undefined -avoid-version

mdp_render_scanline_25_la_SOURCES = \
		mdp_render_scanline_25_plugin.c \
		mdp_render_scanline_25.cpp \
		mdp_render_scanline_25_16_x86.asm \
		mdp_render_scanline_25_32_x86.asm

noinst_HEADERS = \
		mdp_render_scanline_25_plugin.h \
		mdp_render_scanline_25_icon.h \
		mdp_render_scanline_25.hpp \
		mdp_render_scanline_25_x86.h
