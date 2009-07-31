# MDP Render Plugin: Scale3x renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_scale3x.la

INCLUDES = -I@top_srcdir@/src/

mdp_render_scale3x_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_scale3x_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_scale3x_la_SOURCES = \
		mdp_render_scale3x_plugin.c \
		mdp_render_scale3x.c \
		scale3x.c \
		scalebit_3x.c

noinst_HEADERS = \
		mdp_render_scale3x_plugin.h \
		mdp_render_scale3x_icon.h \
		mdp_render_scale3x.h \
		scale3x.h \
		scalebit_3x.h
