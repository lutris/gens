## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

liblzma_a_CFLAGS = $(AM_CFLAGS) -O3 -funroll-loops

noinst_LIBRARIES = liblzma.a

liblzma_a_SOURCES = \
		7zBuf.c \
		7zBuf2.c \
		7zCrc.c \
		7zFile.c \
		7zStream.c \
		Bcj2.c \
		Bra86.c \
		LzmaDec.c \
		Archive/7z/7zAlloc.c \
		Archive/7z/7zDecode.c \
		Archive/7z/7zExtract.c \
		Archive/7z/7zHeader.c \
		Archive/7z/7zIn.c \
		Archive/7z/7zItem.c

noinst_HEADERS = \
		7zBuf.h \
		7zCrc.h \
		7zFile.h \
		7zVersion.h \
		Bra.h \
		Bcj2.h \
		CpuArch.h \
		LzmaDec.h \
		Types.h \
		Archive/7z/7zAlloc.h \
		Archive/7z/7zDecode.h \
		Archive/7z/7zExtract.h \
		Archive/7z/7zHeader.h \
		Archive/7z/7zIn.h \
		Archive/7z/7zItem.h
