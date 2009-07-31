# MDP Render Plugin: Scale2x renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_scale2x.la

INCLUDES = -I@top_srcdir@/src/

mdp_render_scale2x_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_scale2x_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_render_scale2x_la_SOURCES = \
		mdp_render_scale2x_plugin.c \
		mdp_render_scale2x.c \
		scale2x.c \
		scalebit_2x.c

noinst_HEADERS = \
		mdp_render_scale2x_plugin.h \
		mdp_render_scale2x_icon.h \
		mdp_render_scale2x.h \
		scale2x.h \
		scalebit_2x.h

if GENS_X86_ASM
mdp_render_scale2x_la_SOURCES += scalebit_2x_mmx.c
noinst_HEADERS += scalebit_2x_mmx.h
endif
