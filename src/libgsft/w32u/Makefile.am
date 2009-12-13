# libgsft_w32u: Win32 Unicode Translation Layer.

AUTOMAKE_OPTIONS = foreign subdir-objects

noinst_LTLIBRARIES = libgsft_w32u.la

libgsft_w32u_la_CFLAGS		= $(AM_CFLAGS) -O3 -funroll-loops
libgsft_w32u_la_CXXFLAGS	= $(AM_CXXFLAGS) -O3 -funroll-loops
libgsft_w32u_la_LDFLAGS		= $(AM_LDFLAGS)
libgsft_w32u_la_LIBADD		= -lkernel32 -luser32 -lshell32 -lcomdlg32
libgsft_w32u_la_LIBTOOLFLAGS	= $(AM_LIBTOOLFLAGS) --tag=disable-shared

libgsft_w32u_la_SOURCES = \
		w32u.c \
		w32u_charset.c \
		w32u_windows.c \
		W/w32u_windowsW.c \
		A/w32u_windowsA.c \
		w32u_windowsx.c \
		W/w32u_windowsxW.c \
		A/w32u_windowsxA.c \
		w32u_commctrl.c \
		W/w32u_commctrlW.c \
		A/w32u_commctrlA.c \
		w32u_shellapi.c \
		W/w32u_shellapiW.c \
		A/w32u_shellapiA.c \
		w32u_libc.c \
		W/w32u_libcW.c \
		A/w32u_libcA.c \
		w32u_commdlg.c \
		W/w32u_commdlgW.c \
		A/w32u_commdlgA.c \
		w32u_shlobj.c \
		W/w32u_shlobjW.c \
		A/w32u_shlobjA.c \
		w32u_winnls.c \
		W/w32u_winnlsW.c \
		A/w32u_winnlsA.c

noinst_HEADERS = \
		w32u.h \
		w32u_charset.h \
		w32u_windows.h \
		W/w32u_windowsW.h \
		A/w32u_windowsA.h \
		w32u_windowsx.h \
		W/w32u_windowsxW.h \
		A/w32u_windowsxA.h \
		w32u_commctrl.h \
		W/w32u_commctrlW.h \
		A/w32u_commctrlA.h \
		w32u_shellapi.h \
		W/w32u_shellapiW.h \
		A/w32u_shellapiA.h \
		w32u_libc.h \
		W/w32u_libcW.h \
		A/w32u_libcA.h \
		w32u_commdlg.h \
		W/w32u_commdlgW.h \
		A/w32u_commdlgA.h \
		w32u_shlobj.h \
		W/w32u_shlobjW.h \
		A/w32u_shlobjA.h \
		w32u_winnls.h \
		W/w32u_winnlsW.h \
		A/w32u_winnlsA.h
