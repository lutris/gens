# MDP Miscellaneous Plugin: Game Genie.

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = mdp_misc_game_genie.la

mdp_misc_game_genie_la_LDFLAGS = -module -no-undefined -avoid-version

mdp_misc_game_genie_la_SOURCES = \
		mdp_misc_game_genie_plugin.c \
		mdp_misc_game_genie.cpp \
		patch_codes.cpp

noinst_HEADERS = \
		mdp_misc_game_genie_plugin.h \
		mdp_misc_game_genie.hpp \
		patch_codes.hpp
