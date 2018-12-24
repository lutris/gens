# MDP Render Plugin: EPX renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_epx.la

INCLUDES = -I@top_srcdir@/src/

mdp_render_epx_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_epx_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_epx_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_epx_la_SOURCES = \
		mdp_render_epx_plugin.c \
		mdp_render_epx.cpp

noinst_HEADERS = \
		mdp_render_epx_plugin.h \
		mdp_render_epx.hpp
