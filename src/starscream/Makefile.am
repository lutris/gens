# starscream: Motorola 68000 emulator.

AUTOMAKE_OPTIONS = subdir-objects

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

BUILT_SOURCES = main68k.asm sub68k.asm
CLEANFILES = star_m68k${EXEEXT} star_s68k${EXEEXT} main68k.asm sub68k.asm

EXTRA_DIST = stardoc_m68k.txt stardoc_s68k.txt STARCPU.H
SUFFIXES = .asm

# Hack to get Starscream to build using the BUILD compiler when
# cross-compiling for Win32.
# NOTE: This will only work with gcc.
star_m68k$(EXEEXT):
	$(build_alias)-gcc -o star_m68k$(EXEEXT) ${srcdir}/star_m68k.c
star_s68k$(EXEEXT):
	$(build_alias)-gcc -o star_s68k$(EXEEXT) ${srcdir}/star_s68k.c

main68k.asm: star_m68k$(EXEEXT)
	./star_m68k$(EXEEXT) main68k.asm -hog -name main68k_
sub68k.asm: star_s68k$(EXEEXT)
	./star_s68k$(EXEEXT) sub68k.asm -hog -name sub68k_

# Build libstarscream.a
noinst_LTLIBRARIES = libstarscream.la

libstarscream_la_CFLAGS		= $(AM_CFLAGS) -static
libstarscream_la_LDFLAGS	= $(AM_LDFLAGS) -static

libstarscream_la_SOURCES	= main68k.asm sub68k.asm dummy.c
