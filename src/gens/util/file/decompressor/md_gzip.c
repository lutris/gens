/***************************************************************************
 * Gens: GZip Decompressor.                                                *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#include "md_gzip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// GZip decompressor functions.
static int decompressor_gzip_detect_format(FILE *zF);
static file_list_t* decompressor_gzip_get_file_info(FILE *zF, const char* filename);
static int decompressor_gzip_get_file(FILE *zF, const char* filename,
				      file_list_t *file_list,
				      unsigned char *buf, const int size);

// GZip decompressor struct.
decompressor_t decompressor_gzip =
{
	.detect_format	= decompressor_gzip_detect_format,
	.get_file_info	= decompressor_gzip_get_file_info,
	.get_file	= decompressor_gzip_get_file
};


/**
 * decompressor_gzip_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
static int decompressor_gzip_detect_format(FILE *zF)
{
	unsigned char buf[2];
	fseek(zF, 0, SEEK_SET);
	fread(buf, 2, sizeof(unsigned char), zF);
	return (buf[0] == 0x1F && buf[1] == 0x8B);
}


/**
 * decompressor_gzip_get_file_info(): Get information from all files in the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @return Pointer to the first file in the list, or NULL on error.
 */
static file_list_t* decompressor_gzip_get_file_info(FILE *zF, const char* filename)
{
	// GZip-compressed files can only have one file.
	gzFile gzfd;
	char buf[1024];
	
	gzfd = gzopen(filename, "rb");
	if (!gzfd)
	{
		// Error obtaining a GZip file descriptor.
		return NULL;
	}
	
	// Read through the GZip file until we hit an EOF.
	int filesize = 0;
	while (!gzeof(gzfd))
	{
		filesize += gzread(gzfd, buf, 1024);
	}
	
	// Close the GZip fd.
	gzclose(gzfd);
	
	// Allocate memory for the file_list_t.
	file_list_t *file_list = malloc(sizeof(file_list_t));
	
	// Set the elements of the list.
	file_list->filesize = filesize;
	file_list->filename = (filename ? strdup(filename) : NULL);
	file_list->next = NULL;
	
	// Return the list.
	return file_list;
}


/**
 * decompressor_gzip_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the GZip handler.)
 * @param filename Filename of the archive. (Unused in the GZip handler.)
 * @param file_list Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or -1 on error.
 */
int decompressor_gzip_get_file(FILE *zF, const char *filename,
			       file_list_t *file_list,
			       unsigned char *buf, const int size)
{
	// Unused parameters.
	((void)filename);
	((void)file_list);
	
	// All parameters (except filename and file_list) must be specified.
	if (!zF || !buf || (size < 0))
		return -1;
	
	gzFile gzfd;
	int retval;
	
	fseek(zF, 0, SEEK_SET);
	gzfd = gzopen(filename, "rb");
	if (!gzfd)
	{
		// Error obtaining a GZip file descriptor.
		return -1;
	}
	
	// Decompress the GZip file into memory.
	retval = gzread(gzfd, buf, size);
	gzclose(gzfd);
	return retval;
}
