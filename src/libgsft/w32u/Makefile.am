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
		w32u_windows.c \
		w32u_windowsW.c \
		w32u_windowsA.c \
		w32u_windowsx.c \
		w32u_windowsxW.c \
		w32u_windowsxA.c \
		w32u_commctrl.c \
		w32u_shellapi.c \
		w32u_shellapiW.c \
		w32u_shellapiA.c \
		w32u_libc.c \
		w32u_libcW.c \
		w32u_libcA.c \
		w32u_commdlg.c \
		w32u_shlobj.c \
		w32u_shlobjW.c \
		w32u_shlobjA.c

noinst_HEADERS = \
		w32u.h \
		w32u_priv.h \
		w32u_windows.h \
		w32u_windowsW.h \
		w32u_windowsA.h \
		w32u_windowsx.h \
		w32u_windowsxW.h \
		w32u_windowsxA.h \
		w32u_commctrl.h \
		w32u_shellapi.h \
		w32u_shellapiW.h \
		w32u_shellapiA.h \
		w32u_libc.h \
		w32u_libcW.h \
		w32u_libcA.h \
		w32u_commdlg.h \
		w32u_shlobj.h \
		w32u_shlobjW.h \
		w32u_shlobjA.h
