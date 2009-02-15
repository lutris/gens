# MDP Miscellaneous Plugin: Game Genie.

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = mdp_misc_game_genie.la

mdp_misc_game_genie_la_LDFLAGS = -module -no-undefined -avoid-version

mdp_misc_game_genie_la_SOURCES = \
		gg_plugin.c \
		gg.cpp \
		patch_code.cpp

noinst_HEADERS = \
		gg_plugin.h \
		gg_icon.h \
		gg.hpp \
		gg_window.h \
		patch_code.hpp

if GENS_UI_GTK
mdp_misc_game_genie_la_SOURCES += \
		gg_window_gtk2.c
mdp_misc_game_genie_la_CFLAGS = @GTK_CFLAGS@
mdp_misc_game_genie_la_CXXFLAGS = @GTK_CFLAGS@
endif

if GENS_UI_WIN32
mdp_misc_game_genie_la_LIBADD = -lgdi32
endif
