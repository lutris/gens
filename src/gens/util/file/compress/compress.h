/***************************************************************************
 * Gens: File Compression Function Definitions.                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

/***************************************************************************
 * This file defines various function prototypes that are required
 * to be implemented by file compression handlers. These function
 * pointers are then stored in a struct that is then used by
 * the ROM loading code to identify and decompress ROM images.
 ***************************************************************************/

#ifndef GENS_COMPRESSION_H
#define GENS_COMPRESSION_H

#include <stdio.h>

// Function pointer calling conventions from SDL's begin_code.h
#ifndef GENS_FNCALL
#if defined(__WIN32__) && !defined(__GNUC__)
#define GENS_FNCALL __cdecl
#else
#ifdef __OS2__
#define GENS_FNCALL _System
#else
#define GENS_FNCALL
#endif
#endif
#endif

// TODO: Fix the workaround for the zlib bug, which requires filenames to be
// specified instead of FILE pointers for COMPRESS_Get_First_File_Size() and
// COMPRESS_Get_First_File().

/**
 * Compress_Data_t: Data structure used for various functions.
 */
struct COMPRESS_FileInfo_t
{
	int filesize;
	char filename[256];
};

/**
 * COMPRESS_Detect_Format(): Detects if a file is in this compression handler's format.
 * @param filename Filename of the archive.
 * @return 0 if this is not in this handler's format; non-zero if it is.
 */
typedef int (GENS_FNCALL *COMPRESS_Detect_Format)(FILE *f);

/**
 * COMPRESS_Get_First_File_Info(): Gets information about the first file in the specified archive.
 * @param filename Filename of the archive.
 * @param retFileInfo Struct to store information about the file.
 * @return 1 on success; 0 on error.
 */
typedef int (GENS_FNCALL *COMPRESS_Get_First_File_Info)(const char *filename, struct COMPRESS_FileInfo_t *retFileInfo);

/**
 * COMPRESS_Get_File(): Gets the specified file from the specified archive.
 * @param filename Filename of the archive.
 * @param fileInfo Information about the file to extract.
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
typedef int (GENS_FNCALL *COMPRESS_Get_File)(const char *filename, const struct COMPRESS_FileInfo_t *fileInfo, void *buf, int size);

/**
 * COMPRESS_Handler(): Struct containing function pointers for compression handlers.
 */
struct COMPRESS_Handler_t
{
	COMPRESS_Detect_Format		detect_format;
	COMPRESS_Get_First_File_Info	get_first_file_info;
	COMPRESS_Get_File		get_file;
};

// struct containing all of the available compression methods.
extern const struct COMPRESS_Handler_t CompressMethods[];

// Compression handler includes.
#include "gzip.h"
#include "dummy.h"
#include "Zip.h"

#endif
