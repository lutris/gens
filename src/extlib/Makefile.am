# Gens/GS build system.
# src/extlib/: "External" libraries that were specified to be built-in.

AUTOMAKE_OPTIONS = foreign

SUBDIRS =

if GENS_ZLIB
if GENS_ZLIB_INTERNAL
SUBDIRS += zlib
endif # GENS_ZLIB_INTERNAL
SUBDIRS += minizip
endif # GENS_ZLIB

if GENS_PNG
if GENS_PNG_INTERNAL
SUBDIRS += libpng
endif # GENS_PNG_INTERNAL
endif # GENS_PNG

if GENS_MP3
SUBDIRS += mp3_dec
endif # GENS_MP3

if GENS_LZMA
SUBDIRS += lzma
endif # GENS_LZMA
