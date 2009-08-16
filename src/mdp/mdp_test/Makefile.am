# MDP: Mega Drive Plugins
# Interface Specification Version 0.1.0
# Test Suite

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = mdp_test

# Initialize CFLAGS, CXXFLAGS, and LDFLAGS.
mdp_test_CFLAGS     = $(AM_CFLAGS)
mdp_test_CXXFLAGS   = $(AM_CXXFLAGS)
mdp_test_LDFLAGS    = $(AM_LDFLAGS)

INCLUDES = -I@srcdir@/ -I@top_srcdir@/src/

mdp_test_SOURCES = main.c plugin_check.c
noinst_HEADERS = main.h plugin_check.h

if !GENS_OS_WIN32
mdp_test_LDADD = -ldl
endif
