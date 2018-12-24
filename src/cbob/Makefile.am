# cbob: C Binary Object Builder.

AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = cbob

cbob_SOURCES = cbob.c

# Hack to get CBOB to build using the BUILD compiler when
# cross-compiling for Win32.
# NOTE: This will only work with gcc.
cbob.o: cbob.c
	$(build_alias)-gcc -o cbob.o -c ${srcdir}/cbob.c

cbob$(EXEEXT): cbob.o
	$(build_alias)-gcc -o cbob$(EXEEXT) cbob.o
