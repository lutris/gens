# MDP Miscellaneous Plugin: Sonic Gens.

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

lib_LTLIBRARIES = mdp_misc_sgens.la

mdp_misc_sgens_la_CFLAGS	= $(AM_CFLAGS)
mdp_misc_sgens_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_misc_sgens_la_SOURCES = \
		sgens_plugin.c \
		sgens_rom_type.c \
		sgens.c \
		sgens_widget_info.c

noinst_HEADERS = \
		sgens_plugin.h \
		sgens_icon.h \
		sgens_rom_type.h \
		sgens.h \
		sgens_window.h \
		sgens_widget_info.h

if GENS_UI_GTK
mdp_misc_sgens_la_SOURCES	+= sgens_window_gtk2.c
mdp_misc_sgens_la_CFLAGS	+= @GTK_CFLAGS@
mdp_misc_sgens_la_LIBADD	 = @GTK_LIBS@
endif

if GENS_UI_WIN32
mdp_misc_sgens_la_SOURCES	+= sgens_window_win32.c
mdp_misc_sgens_la_LIBADD	 = -lgdi32
endif
