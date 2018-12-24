# MDP Render Plugin: Blargg's NTSC Filter.

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_render_blargg_ntsc.la

mdp_render_blargg_ntsc_la_CFLAGS	= $(AM_CFLAGS)
mdp_render_blargg_ntsc_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_render_blargg_ntsc_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_render_blargg_ntsc_la_LIBADD	= $(top_builddir)/src/libgsft/libgsft.la

mdp_render_blargg_ntsc_la_SOURCES = \
		mdp_render_blargg_ntsc_plugin.c \
		mdp_render_blargg_ntsc.c \
		md_ntsc.cpp \
		ntsc_window_common.c

noinst_HEADERS = \
		mdp_render_blargg_ntsc_plugin.h \
		mdp_render_blargg_ntsc.h \
		md_ntsc.hpp \
		md_ntsc_config.h \
		md_ntsc_impl.h \
		ntsc_window.h \
		ntsc_window_common.h

if GENS_UI_GTK
mdp_render_blargg_ntsc_la_SOURCES	+= ntsc_window_gtk2.c
mdp_render_blargg_ntsc_la_CFLAGS	+= @GTK_CFLAGS@
mdp_render_blargg_ntsc_la_LIBADD	+= @GTK_LIBS@
#noinst_HEADERS += \
#		vlopt_16x16.xpm \
#		vlopt_32x32.xpm
endif

if GENS_UI_WIN32
mdp_render_blargg_ntsc_la_SOURCES	+= ntsc_window_win32.c
#mdp_render_blargg_ntsc_la_SOURCES	+= ntsc_dllmain.c resource.rc
mdp_render_blargg_ntsc_la_LIBADD	+= \
		-lgdi32 \
		$(top_builddir)/src/libgsft/libgsft_win32.la \
		$(top_builddir)/src/libgsft/libgsft_win32_gdi.la
#noinst_HEADERS += \
#		vlopt.ico \
#		vlopt_dllmain.h \
#		resource.h
endif

if GENS_OS_HAIKU
mdp_render_blargg_ntsc_la_SOURCES	+= ntsc_window_haiku.c
endif
