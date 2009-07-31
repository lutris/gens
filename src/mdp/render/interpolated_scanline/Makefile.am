# MDP Render Plugin: Interpolated Scanline renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_interpolated_scanline.la

INCLUDES = -I@top_srcdir@/src/
mdp_render_interpolated_scanline_la_CCASFLAGS	= $(INCLUDES)

mdp_render_interpolated_scanline_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_interpolated_scanline_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_interpolated_scanline_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_interpolated_scanline_la_SOURCES = \
		mdp_render_interpolated_scanline_plugin.c \
		mdp_render_interpolated_scanline.cpp

noinst_HEADERS = \
		mdp_render_interpolated_scanline_plugin.h \
		mdp_render_interpolated_scanline_icon.h \
		mdp_render_interpolated_scanline.hpp

if GENS_X86_ASM
mdp_render_interpolated_scanline_la_SOURCES += \
		mdp_render_interpolated_scanline_16_x86.S
noinst_HEADERS += \
		mdp_render_interpolated_scanline_x86.h
endif # GENS_X86_ASM
