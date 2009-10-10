# Gens/GS firmware images.
# This includes the reverse-engineered 32X firmware.

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libgens_fw.la

CBOB = $(top_builddir)/src/cbob/cbob$(EXEEXT)

libgens_fw_la_CFLAGS	= $(AM_CFLAGS) -static
libgens_fw_la_LDFLAGS	= $(AM_LDFLAGS) -static

# TODO: Write macros to make this easier to maintain.
bios32xg.bin.c:
	$(CBOB) $(srcdir)/bios32xg.bin $@
bios32xm.bin.c:
	$(CBOB) $(srcdir)/bios32xm.bin $@
bios32xs.bin.c:
	$(CBOB) $(srcdir)/bios32xs.bin $@

CLEANFILES = bios32xg.bin.c bios32xm.bin.c bios32xs.bin.c

libgens_fw_la_SOURCES = \
		bios32xg.bin.c \
		bios32xm.bin.c \
		bios32xs.bin.c
