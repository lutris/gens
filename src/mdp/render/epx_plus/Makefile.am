# MDP Render Plugin: EPX Plus renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_epx_plus.la

INCLUDES = -I@top_srcdir@/src/

mdp_render_epx_plus_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_epx_plus_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_epx_plus_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_epx_plus_la_SOURCES = \
		mdp_render_epx_plus_plugin.c \
		mdp_render_epx_plus.cpp

noinst_HEADERS = \
		mdp_render_epx_plus_plugin.h \
		mdp_render_epx_plus.hpp
