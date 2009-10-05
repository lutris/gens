# cbob: C Binary Object Builder.

AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = cbob

cbob_SOURCES = cbob.c

# Hack to get CBOB to build using the BUILD compiler when
# cross-compiling for Win32.
# NOTE: This will only work with gcc.
cbob$(EXEEXT):
	$(build_alias)-gcc -o cbob$(EXEEXT) ${srcdir}/cbob.c
