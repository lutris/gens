NASM = nasm -O3 -D __GCC2

if LINUX
NASM += -f elf
endif

if WIN32
NASM += -f win32
endif

BUILT_SOURCES = main68k.asm
CLEANFILES = star main68k.asm

EXTRA_DIST = STARDOC.TXT STARCPU.H
SUFFIXES = .asm

%.o:	%.asm
	$(NASM) $< -o $@

# Hack to get Starscream to build using the BUILD compiler when
# cross-compiling for Win32.
# NOTE: This will only work with gcc.
star:
	${build_alias}-gcc -o star ${srcdir}/star.c

main68k.asm:	star
		./star main68k.asm -hog -name main68k_

