# MDP Miscellaneous Plugin: IRC Reporter.

AUTOMAKE_OPTIONS = foreign subdir-objects

INCLUDES = -I@top_srcdir@/src/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_misc_irc_reporter.la

mdp_misc_irc_reporter_la_CFLAGS		= $(AM_CFLAGS) $(DBUS_CFLAGS) $(DBUS_GLIB_CFLAGS)
mdp_misc_irc_reporter_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_misc_irc_reporter_la_LIBADD		= $(top_builddir)/src/libgsft/libgsft.la $(DBUS_LIBS) $(DBUS_GLIB_LIBS)

mdp_misc_irc_reporter_la_SOURCES = \
		irc_plugin.c	\
		irc.c		\
		dbus-service.c

noinst_HEADERS = \
		irc_plugin.h	\
		irc.h		\
		dbus-service.h

#dbus-binding-tool --mode=glib-server --prefix="irc_reporter" dbus-service.xml >dbus-service-glue.h
