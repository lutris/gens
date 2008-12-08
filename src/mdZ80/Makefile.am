## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

ASMFILES = mdZ80.o

%.o: %.asm
	$(NASM) $< -o $@

noinst_LIBRARIES = libmdZ80.a

libmdZ80_a_SOURCES = \
		mdZ80.asm \
		mdZ80_DAA.c \
		mdZ80_INC_DEC.c

noinst_HEADERS = \
		mdZ80.h \
		mdZ80_DAA.h \
		mdZ80_INC_DEC.h

libmdZ80_a_LIBADD = \
		$(ASMFILES)

libmdZ80_a_DEPENDENCIES = \
		$(ASMFILES)

CLEANFILES = \
		$(ASMFILES)
