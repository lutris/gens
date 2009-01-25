# MDP Miscellaneous Plugin: Game Genie.

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = mdp_misc_game_genie.la

mdp_misc_game_genie_la_LDFLAGS = -module -no-undefined -avoid-version

mdp_misc_game_genie_la_SOURCES = \
		mdp_misc_game_genie_plugin.c \
		mdp_misc_game_genie.cpp \
		patch_code.cpp

noinst_HEADERS = \
		mdp_misc_game_genie_plugin.h \
		mdp_misc_game_genie_icon.h \
		mdp_misc_game_genie.hpp \
		mdp_misc_game_genie_window.hpp \
		patch_code.hpp

if GENS_UI_GTK
mdp_misc_game_genie_la_SOURCES += \
		mdp_misc_game_genie_window_gtk2.cpp
mdp_misc_game_genie_la_CFLAGS = @GTK_CFLAGS@
mdp_misc_game_genie_la_CXXFLAGS = @GTK_CFLAGS@
endif
