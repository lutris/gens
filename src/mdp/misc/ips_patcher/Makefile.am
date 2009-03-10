# MDP Miscellaneous Plugin: IPS Patcher.

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

lib_LTLIBRARIES = mdp_misc_ips_patcher.la

mdp_misc_ips_patcher_la_CFLAGS		= $(AM_CFLAGS)
mdp_misc_ips_patcher_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_misc_ips_patcher_la_LDFLAGS		= $(AM_LDFLAGS) -module -no-undefined -avoid-version

mdp_misc_ips_patcher_la_SOURCES = \
		ips_plugin.c	\
		ips.c		\
		ips_file.cpp

noinst_HEADERS = \
		ips_plugin.h	\
		ips_icon.h	\
		ips.h		\
		ips_file.cpp
