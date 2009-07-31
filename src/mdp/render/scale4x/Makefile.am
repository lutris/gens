# MDP Render Plugin: Scale4x renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_scale4x.la

INCLUDES = -I@top_srcdir@/src/

mdp_render_scale4x_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_scale4x_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_scale4x_la_SOURCES = \
		mdp_render_scale4x_plugin.c \
		mdp_render_scale4x.c \
		scale2x.c \
		scalebit_4x.c

noinst_HEADERS = \
		mdp_render_scale4x_plugin.h \
		mdp_render_scale4x_icon.h \
		mdp_render_scale4x.h \
		scale2x.h \
		scalebit_4x.h

if GENS_X86_ASM
mdp_render_scale4x_la_SOURCES += scalebit_4x_mmx.c
noinst_HEADERS += scalebit_4x_mmx.h
endif
