# MDP Miscellaneous Plugin: VDP Layer Options.

# Win32 resources
.rc.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=RC $(RC) -I@srcdir@ -I$(top_builddir) $(DEFS) $< -o $@

AUTOMAKE_OPTIONS = subdir-objects

INCLUDES = -I@top_srcdir@/src/

mdpdir = $(libdir)/mdp
mdp_LTLIBRARIES = mdp_misc_vdp_layer_options.la

mdp_misc_vdp_layer_options_la_CFLAGS	= $(AM_CFLAGS)
mdp_misc_vdp_layer_options_la_LDFLAGS	= $(AM_LDFLAGS) -module -no-undefined -avoid-version
mdp_misc_vdp_layer_options_la_LIBADD	= $(top_builddir)/src/libgsft/libgsft.la

mdp_misc_vdp_layer_options_la_SOURCES = \
		vlopt_plugin.c \
		vlopt.c \
		vlopt_options.c

noinst_HEADERS = \
		vlopt_plugin.h \
		vlopt_icon.h \
		vlopt.h \
		vlopt_options.h \
		vlopt_window.h

if GENS_UI_GTK
mdp_misc_vdp_layer_options_la_SOURCES	+= vlopt_window_gtk2.c
mdp_misc_vdp_layer_options_la_CFLAGS	+= @GTK_CFLAGS@
mdp_misc_vdp_layer_options_la_LIBADD	+= @GTK_LIBS@
noinst_HEADERS += \
		vlopt_16x16.xpm \
		vlopt_32x32.xpm
endif

if GENS_UI_WIN32
mdp_misc_vdp_layer_options_la_SOURCES	+= vlopt_window_win32.c vlopt_dllmain.c resource.rc
mdp_misc_vdp_layer_options_la_LIBADD	+= \
		-lgdi32 \
		$(top_builddir)/src/libgsft/libgsft_win32.la \
		$(top_builddir)/src/libgsft/libgsft_win32_gdi.la
noinst_HEADERS += \
		vlopt.ico \
		vlopt_dllmain.h \
		resource.h
endif

if GENS_OS_HAIKU
mdp_misc_vdp_layer_options_la_SOURCES	+= vlopt_window_haiku.c
endif
