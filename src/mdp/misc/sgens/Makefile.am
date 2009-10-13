# MDP Miscellaneous Plugin: Sonic Gens.

# Win32 resources
.rc.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=RC $(RC) -I@srcdir@ -I$(top_builddir) $(DEFS) $< -o $@

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_misc_sgens.la

mdp_misc_sgens_la_CFLAGS	= $(AM_CFLAGS)
mdp_misc_sgens_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_misc_sgens_la_LIBADD	= $(top_builddir)/src/libgsft/libgsft.la

mdp_misc_sgens_la_SOURCES = \
		sgens_plugin.c \
		sgens_rom_type.c \
		sgens.c \
		sgens_widget_info.c

noinst_HEADERS = \
		sgens_plugin.h \
		sgens_icon.h \
		sgens_rom_type.h \
		sgens.h \
		sgens_window.h \
		sgens_widget_info.h

if GENS_UI_GTK
mdp_misc_sgens_la_SOURCES	+= sgens_window_gtk2.c
mdp_misc_sgens_la_CFLAGS	+= @GTK_CFLAGS@
mdp_misc_sgens_la_LIBADD	+= @GTK_LIBS@
noinst_HEADERS += \
		sgens_16x16.xpm \
		sgens_32x32.xpm
endif

if GENS_UI_WIN32
mdp_misc_sgens_la_SOURCES	+= sgens_window_win32.c sgens_dllmain.c resource.rc
mdp_misc_sgens_la_LIBADD	+= \
		-lgdi32 \
		$(top_builddir)/src/libgsft/libgsft_win32.la \
		$(top_builddir)/src/libgsft/libgsft_win32_gdi.la
noinst_HEADERS += \
		sgens.ico \
		sgens_dllmain.h \
		resource.h
endif

if GENS_OS_HAIKU
mdp_misc_sgens_la_SOURCES	+= sgens_window_haiku.c
endif
