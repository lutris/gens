# libgsft_w32u: Win32 Unicode Translation Layer.

AUTOMAKE_OPTIONS = foreign subdir-objects

noinst_LTLIBRARIES = libgsft_w32u.la

libgsft_w32u_la_CFLAGS		= $(AM_CFLAGS) -static -O3 -funroll-loops
libgsft_w32u_la_CXXFLAGS	= $(AM_CXXFLAGS) -static -O3 -funroll-loops
libgsft_w32u_la_LDFLAGS		= $(AM_LDFLAGS) -static
libgsft_w32u_la_LIBADD		= -lkernel32

libgsft_w32u_la_SOURCES = \
		w32u.c \
		w32u_priv.c \
		w32u_windowsx.c \
		w32u_commctrl.c \
		w32u_shellapi.c \
		w32u_libc.c \
		w32u_commdlg.c \
		w32u_shlobj.c

noinst_HEADERS = \
		w32u.h \
		w32u_priv.h \
		w32u_windowsx.h \
		w32u_commctrl.h \
		w32u_shellapi.h \
		w32u_libc.h \
		w32u_commdlg.h \
		w32u_shlobj.h
