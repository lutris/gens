# MDP Render Plugin: Blargg's NTSC renderer.

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

lib_LTLIBRARIES = mdp_render_blargg_ntsc.la

mdp_render_blargg_ntsc_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_blargg_ntsc_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_blargg_ntsc_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_blargg_ntsc_la_SOURCES = \
		mdp_render_blargg_ntsc_plugin.c \
		mdp_render_blargg_ntsc.c \
		md_ntsc.cpp

noinst_HEADERS = \
		mdp_render_blargg_ntsc_plugin.h \
		mdp_render_blargg_ntsc.h \
		md_ntsc.hpp \
		md_ntsc_config.h \
		md_ntsc_impl.h
