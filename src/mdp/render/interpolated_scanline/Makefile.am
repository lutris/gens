# MDP Render Plugin: Interpolated Scanline renderer.

# libtool/nasm stuff from http://osdir.com/ml/sysutils.automake.general/2002-08/msg00134.html
# strip_fPIC.sh from http://networkmultimedia.org/

AUTOMAKE_OPTIONS = subdir-objects

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

lib_LTLIBRARIES = mdp_render_interpolated_scanline.la

mdp_render_interpolated_scanline_la_LDFLAGS = -module -no-undefined -avoid-version

mdp_render_interpolated_scanline_la_SOURCES = \
		mdp_render_interpolated_scanline_plugin.c \
		mdp_render_interpolated_scanline.cpp

noinst_HEADERS = \
		mdp_render_interpolated_scanline_plugin.h \
		mdp_render_interpolated_scanline_icon.h \
		mdp_render_interpolated_scanline.hpp \
		mdp_render_interpolated_scanline_x86.h

if GENS_X86_ASM
mdp_render_interpolated_scanline_la_SOURCES += \
		mdp_render_interpolated_scanline_16_x86.asm
endif # GENS_X86_ASM
