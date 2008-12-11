AUTOMAKE_OPTIONS = foreign

SUBDIRS = starscream mdZ80

if GENS_MP3
SUBDIRS += mp3_dec
endif # GENS_MP3

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

SUBDIRS += gens mdp
