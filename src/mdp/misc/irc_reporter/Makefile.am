# MDP Miscellaneous Plugin: IRC Reporter.

AUTOMAKE_OPTIONS = foreign subdir-objects

INCLUDES = -I@top_srcdir@/src/ -I/usr/include/dbus-1.0/ -I/usr/include/glib-2.0/ -I/usr/lib/glib-2.0/include/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_misc_irc_reporter.la

mdp_misc_irc_reporter_la_CFLAGS		= $(AM_CFLAGS)
mdp_misc_irc_reporter_la_CXXFLAGS	= $(AM_CXXFLAGS)
mdp_misc_irc_reporter_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_misc_irc_reporter_la_LIBADD		= $(top_builddir)/src/libgsft/libgsft.la -ldbus-glib-1

mdp_misc_irc_reporter_la_SOURCES = \
		irc_plugin.c	\
		irc.c		\
		dbus-service.c

noinst_HEADERS = \
		irc_plugin.h	\
		irc.h		\
		dbus-service.h

#dbus-binding-tool --mode=glib-server --prefix="irc_reporter" dbus-service.xml >dbus-service-glue.h
