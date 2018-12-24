# MDP Render Plugin: 2xSaI renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_2xsai.la

INCLUDES = -I@top_srcdir@/src/
mdp_render_2xsai_la_CCASFLAGS	= $(INCLUDES)

mdp_render_2xsai_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_2xsai_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

NASM += -I@srcdir@/../

mdp_render_2xsai_la_SOURCES = \
		mdp_render_2xsai_plugin.c \
		mdp_render_2xsai.c \
		mdp_render_2xsai_16_x86.S

noinst_HEADERS = \
		mdp_render_2xsai_plugin.h \
		mdp_render_2xsai.h \
		mdp_render_2xsai_x86.h
