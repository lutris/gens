# MDP Render Plugin: 25% Scanline renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_scanline_25.la

INCLUDES = -I@top_srcdir@/src/
mdp_render_scanline_25_la_CCASFLAGS	= $(INCLUDES)

mdp_render_scanline_25_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_scanline_25_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_scanline_25_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_scanline_25_la_SOURCES = \
		mdp_render_scanline_25_plugin.c \
		mdp_render_scanline_25.cpp

noinst_HEADERS = \
		mdp_render_scanline_25_plugin.h \
		mdp_render_scanline_25_icon.h \
		mdp_render_scanline_25.hpp

if GENS_X86_ASM
mdp_render_scanline_25_la_SOURCES += \
		mdp_render_scanline_25_16_x86.S \
		mdp_render_scanline_25_32_x86.S
noinst_HEADERS += \
		mdp_render_scanline_25_x86.h
endif # GENS_X86_ASM
