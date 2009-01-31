# MDP Miscellaneous Plugin: Layer Options.

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = mdp_misc_layer_options.la

mdp_misc_layer_options_la_LDFLAGS = -module -no-undefined -avoid-version

mdp_misc_layer_options_la_SOURCES = \
		vlopt_plugin.c \
		vlopt.cpp

noinst_HEADERS = \
		vlopt_plugin.h \
		vlopt_icon.h \
		vlopt.hpp \
		vlopt_window.h

if GENS_UI_GTK
mdp_misc_layer_options_la_SOURCES += \
		vlopt_window_gtk2.c
mdp_misc_layer_options_la_CFLAGS = @GTK_CFLAGS@
mdp_misc_layer_options_la_CXXFLAGS = @GTK_CFLAGS@
endif
