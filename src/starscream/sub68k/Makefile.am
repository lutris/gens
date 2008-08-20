NASM = nasm -O3 -D __GCC2 -f elf

noinst_PROGRAMS = star
star_SOURCES = star.c

BUILT_SOURCES = sub68k.asm
CLEANFILES = sub68k.asm

EXTRA_DIST = STARDOC.TXT STARCPU.H
SUFFIXES = .asm

%.o:	%.asm
	$(NASM) $< -o $@

sub68k.asm:	star
		./star sub68k.asm -hog -name sub68k_

