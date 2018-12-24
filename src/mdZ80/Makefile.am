# mdZ80: Z80 emulator.

AUTOMAKE_OPTIONS = subdir-objects

STRIP_FPIC=$(SHELL) '$(top_srcdir)/strip_fPIC.sh'

.asm.lo:
	$(LIBTOOL) $(LIBTOOLFLAGS) --mode=compile --tag=ASM $(STRIP_FPIC) $(NASM) $< -o $@

noinst_LTLIBRARIES = libmdZ80.la

INCLUDES = -I@top_srcdir@/src/
libmdZ80_la_CCASFLAGS	= $(INCLUDES)

libmdZ80_la_CFLAGS	= $(AM_CFLAGS) -static
libmdZ80_la_LDFLAGS	= $(AM_LDFLAGS) -static

libmdZ80_la_SOURCES = \
		mdZ80_x86.asm \
		mdZ80.c \
		mdZ80_DAA.c \
		mdZ80_INC_DEC.c

noinst_HEADERS = \
		mdZ80.h \
		mdZ80_flags.h \
		mdZ80_opcodes.h \
		mdZ80_DAA.h \
		mdZ80_INC_DEC.h
