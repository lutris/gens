# MDP Miscellaneous Plugin: VDP Debugger.

# Win32 resources
.rc.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=RC $(RC) -I@srcdir@ -I$(top_builddir) $(DEFS) $< -o $@

AUTOMAKE_OPTIONS = foreign subdir-objects

INCLUDES = -I@top_srcdir@/src/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_misc_vdp_debugger.la

mdp_misc_vdp_debugger_la_CFLAGS		= $(AM_CFLAGS)
mdp_misc_vdp_debugger_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_misc_vdp_debugger_la_LIBTOOLFLAGS	= $(AM_LIBTOOLFLAGS) --tag=disable-static
mdp_misc_vdp_debugger_la_LIBADD		= $(top_builddir)/src/libgsft/libgsft.la

mdp_misc_vdp_debugger_la_SOURCES = \
		vdpdbg_plugin.c \
		vdpdbg.c

noinst_HEADERS = \
		vdpdbg_plugin.h \
		vdpdbg.h \
		vdpdbg_window.h

if GENS_UI_GTK
mdp_misc_vdp_debugger_la_SOURCES	+= vdpdbg_window_gtk2.c
mdp_misc_vdp_debugger_la_CFLAGS		+= @GTK_CFLAGS@
mdp_misc_vdp_debugger_la_LIBADD		+= @GTK_LIBS@
endif
