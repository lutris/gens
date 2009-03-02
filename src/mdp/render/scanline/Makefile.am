# MDP Render Plugin: Scanline renderer.

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = mdp_render_scanline.la

mdp_render_scanline_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_scanline_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_scanline_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_scanline_la_SOURCES = \
		mdp_render_scanline_plugin.c \
		mdp_render_scanline.cpp

noinst_HEADERS = \
		mdp_render_scanline_plugin.h \
		mdp_render_scanline_icon.h \
		mdp_render_scanline.hpp

if GENS_X86_ASM
mdp_render_scanline_la_SOURCES += \
		mdp_render_scanline_16_x86.S \
		mdp_render_scanline_32_x86.S
noinst_HEADERS += \
		mdp_render_scanline_x86.h
endif # GENS_X86_ASM
