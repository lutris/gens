## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS = gtk

INCLUDES = \
-I@srcdir@/../emulator -I@srcdir@/../util/file -I@srcdir@/../port -I@srcdir@/../ -I@srcdir@

lib_LIBRARIES = libgens_ui.a

libgens_ui_a_SOURCES = ui-common.c

noinst_HEADERS = ui-common.h
		
libgens_ui_a_CFLAGS = \
			  -DDATADIR=\"@datadir@/@PACKAGE@\" \
			  @SDL_CFLAGS@ \
			  @GTK_CFLAGS@ \
			  @CFLAGS@
