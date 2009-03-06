# MDP Render Plugin: Double renderer.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libmdp_render_2x.a

INCLUDES = -I@top_srcdir@/src/

libmdp_render_2x_a_CFLAGS = $(AM_CFLAGS)
libmdp_render_2x_a_CXXFLAGS = $(AM_CXXFLAGS)

libmdp_render_2x_a_SOURCES = \
		mdp_render_2x_plugin.c \
		mdp_render_2x.cpp

noinst_HEADERS = \
		mdp_render_2x_plugin.h \
		mdp_render_2x_icon.h \
		mdp_render_2x.hpp

if GENS_X86_ASM
libmdp_render_2x_a_SOURCES += \
 		mdp_render_2x_16_x86.S \
		mdp_render_2x_32_x86.S
noinst_HEADERS += \
		mdp_render_2x_x86.h
endif # GENS_X86_ASM
