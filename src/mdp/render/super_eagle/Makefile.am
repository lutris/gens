# MDP Render Plugin: Super Eagle renderer.

AUTOMAKE_OPTIONS = subdir-objects

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_super_eagle.la

INCLUDES = -I@top_srcdir@/src/
mdp_render_super_eagle_la_CCASFLAGS	= $(INCLUDES)

mdp_render_super_eagle_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_super_eagle_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

NASM += -I@srcdir@/../

mdp_render_super_eagle_la_SOURCES = \
		mdp_render_super_eagle_plugin.c \
		mdp_render_super_eagle.c \
		mdp_render_super_eagle_16_x86.S

noinst_HEADERS = \
		mdp_render_super_eagle_plugin.h \
		mdp_render_super_eagle.h \
		mdp_render_super_eagle_x86.h
