# MDP Miscellaneous Plugin: Game Genie.

# Win32 resources.
.rc.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=RC $(RC) -I@srcdir@ -I$(top_builddir) $(DEFS) $< -o $@

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_misc_game_genie.la

mdp_misc_game_genie_la_CFLAGS	= $(AM_CFLAGS)
mdp_misc_game_genie_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_misc_game_genie_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_misc_game_genie_la_LIBADD	= $(top_builddir)/src/libgsft/libgsft.la

mdp_misc_game_genie_la_SOURCES = \
		gg_plugin.c \
		gg.cpp \
		gg_code.c \
		gg_file.cpp \
		gg_engine.cpp

noinst_HEADERS = \
		gg_plugin.h \
		gg_icon.h \
		gg.hpp \
		gg_window.hpp \
		gg_code.h \
		gg_file.hpp \
		gg_engine.cpp

if GENS_UI_GTK
mdp_misc_game_genie_la_SOURCES	+= gg_window_gtk2.cpp
mdp_misc_game_genie_la_CFLAGS	+= @GTK_CFLAGS@
mdp_misc_game_genie_la_CXXFLAGS	+= @GTK_CFLAGS@
mdp_misc_game_genie_la_LIBADD	+= @GTK_LIBS@
noinst_HEADERS += \
		gg_32x32.xpm
endif

if GENS_UI_WIN32
mdp_misc_game_genie_la_SOURCES	+= gg_window_win32.cpp gg_dllmain.c resource.rc
mdp_misc_game_genie_la_LIBADD	+= \
		-lgdi32 \
		$(top_builddir)/src/libgsft/libgsft_win32.la \
		$(top_builddir)/src/libgsft/libgsft_win32_gdi.la
noinst_HEADERS += \
		gg.ico \
		gg_dllmain.h \
		resource.h
endif

if GENS_OS_HAIKU
mdp_misc_game_genie_la_SOURCES += gg_window_haiku.cpp
endif
