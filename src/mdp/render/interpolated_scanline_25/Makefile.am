# MDP Render Plugin: Interpolated 25% Scanline renderer.

# libtool/nasm stuff from http://osdir.com/ml/sysutils.automake.general/2002-08/msg00134.html
# strip_fPIC.sh from http://networkmultimedia.org/

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_interpolated_scanline_25.la

mdp_render_interpolated_scanline_25_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_interpolated_scanline_25_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_interpolated_scanline_25_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_interpolated_scanline_25_la_SOURCES = \
		mdp_render_interpolated_scanline_25_plugin.c \
		mdp_render_interpolated_scanline_25.cpp

noinst_HEADERS = \
		mdp_render_interpolated_scanline_25_plugin.h \
		mdp_render_interpolated_scanline_25_icon.h \
		mdp_render_interpolated_scanline_25.hpp
