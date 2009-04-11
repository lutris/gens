# MDP Miscellaneous Plugin: VDP Layer Options.

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

lib_LTLIBRARIES = mdp_misc_vdp_layer_options.la

mdp_misc_vdp_layer_options_la_CFLAGS	= $(AM_CFLAGS)
mdp_misc_vdp_layer_options_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_misc_vdp_layer_options_la_SOURCES = \
		vlopt_plugin.c \
		vlopt.c \
		vlopt_options.c

noinst_HEADERS = \
		vlopt_plugin.h \
		vlopt_icon.h \
		vlopt.h \
		vlopt_options.h \
		vlopt_window.h

if GENS_UI_GTK
mdp_misc_vdp_layer_options_la_SOURCES	+= vlopt_window_gtk2.c
mdp_misc_vdp_layer_options_la_CFLAGS	+= @GTK_CFLAGS@
mdp_misc_vdp_layer_options_la_LIBADD	+= @GTK_LIBS@
endif

if GENS_UI_WIN32
mdp_misc_vdp_layer_options_la_SOURCES	+= vlopt_window_win32.c
mdp_misc_vdp_layer_options_la_LIBADD	 = -lgdi32
endif
