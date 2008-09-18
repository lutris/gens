NASM = nasm -O3 -D __GCC2

if LINUX
NASM += -f elf
endif

if WIN32
NASM += -f win32
endif

BUILT_SOURCES = sub68k.asm
CLEANFILES = star sub68k.asm

EXTRA_DIST = STARDOC.TXT STARCPU.H
SUFFIXES = .asm

%.o:	%.asm
	$(NASM) $< -o $@

# Hack to get Starscream to build using the BUILD compiler when
# cross-compiling for Win32.
# NOTE: This will only work with gcc.
star:
	${build_alias}-gcc -o star ${srcdir}/star.c

sub68k.asm:	star
		./star sub68k.asm -hog -name sub68k_

