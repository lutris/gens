BUILT_SOURCES = main68k.asm sub68k.asm
CLEANFILES = star_m68k star_s68k main68k.asm sub68k.asm

EXTRA_DIST = stardoc_m68k.txt stardoc_s68k.txt STARCPU.H
SUFFIXES = .asm

%.o: %.asm
	$(NASM) $< -o $@

# Hack to get Starscream to build using the BUILD compiler when
# cross-compiling for Win32.
# NOTE: This will only work with gcc.
star_m68k:
	$(build_alias)-gcc -o star_m68k ${srcdir}/star_m68k.c
star_s68k:
	$(build_alias)-gcc -o star_s68k ${srcdir}/star_s68k.c

main68k.asm: star_m68k
	./star_m68k main68k.asm -hog -name main68k_
sub68k.asm: star_s68k
	./star_s68k sub68k.asm -hog -name sub68k_
