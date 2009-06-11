# MDP Render Plugin: Normal renderer.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libmdp_render_1x.la

INCLUDES = -I@top_srcdir@/src/
libmdp_render_1x_la_CCASFLAGS	= $(INCLUDES)

libmdp_render_1x_la_CFLAGS	= $(AM_CFLAGS) -static
libmdp_render_1x_la_CXXFLAGS	= $(AM_CXXFLAGS) -static
libmdp_render_1x_la_LDFLAGS	= $(AM_LDFLAGS) -static

libmdp_render_1x_la_SOURCES = \
		mdp_render_1x_plugin.c \
		mdp_render_1x.cpp

noinst_HEADERS = \
		mdp_render_1x_plugin.h \
		mdp_render_1x_icon.h \
		mdp_render_1x.hpp

if GENS_X86_ASM
libmdp_render_1x_la_SOURCES += \
		mdp_render_1x_16_x86.S \
		mdp_render_1x_32_x86.S
noinst_HEADERS += \
		mdp_render_1x_x86.h
endif # GENS_X86_ASM
