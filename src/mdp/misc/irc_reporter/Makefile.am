# MDP Miscellaneous Plugin: IRC Reporter.

AUTOMAKE_OPTIONS = foreign subdir-objects

INCLUDES = -I@top_srcdir@/src/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_misc_irc_reporter.la

mdp_misc_irc_reporter_la_CFLAGS		= $(AM_CFLAGS) $(DBUS_CFLAGS) $(DBUS_GLIB_CFLAGS)
mdp_misc_irc_reporter_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_misc_irc_reporter_la_LIBTOOLFLAGS	= $(AM_LIBTOOLFLAGS) --tag=disable-static
mdp_misc_irc_reporter_la_LIBADD		= $(top_builddir)/src/libgsft/libgsft.la $(DBUS_LIBS) $(DBUS_GLIB_LIBS)

BUILT_SOURCES = @abs_builddir@/dbus-service-glue.h

@abs_builddir@/dbus-service-glue.h: dbus-service.xml
	$(LIBTOOL) --mode=execute $(DBUS_BINDING_TOOL) --prefix=irc_reporter --mode=glib-server $< >$@

CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = dbus-service.xml

mdp_misc_irc_reporter_la_SOURCES = \
		irc_plugin.c	\
		irc.cpp		\
		dbus-service.c
		

noinst_HEADERS = \
		irc_plugin.h	\
		irc.hpp		\
		dbus-service.h	\
		$(BUILT_SOURCES)
