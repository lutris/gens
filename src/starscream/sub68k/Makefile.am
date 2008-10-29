if GENS_OBJ_WIN32
NASM_OBJ_FORMAT = -f win32 -D __OBJ_WIN32
endif

if GENS_OBJ_ELF32
NASM_OBJ_FORMAT = -f elf -D __GCC2 -D __OBJ_ELF
endif

if GENS_OBJ_MACHO
NASM_OBJ_FORMAT = -f macho -D __GCC2 -D __OBJ_MACHO
endif

NASM = nasm -O3 -w-orphan-labels $(NASM_OBJ_FORMAT)

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

