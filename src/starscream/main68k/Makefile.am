NASM = nasm -O3 -D __GCC2 -f elf

noinst_PROGRAMS = star
star_SOURCES = star.c

BUILT_SOURCES = main68k.asm
CLEANFILES = main68k.asm

EXTRA_DIST = STARDOC.TXT STARCPU.H
SUFFIXES = .asm

%.o:	%.asm
	$(NASM) $< -o $@

main68k.asm:	star
		./star main68k.asm -hog -name main68k_

