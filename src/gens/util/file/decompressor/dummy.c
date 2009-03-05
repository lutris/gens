/***************************************************************************
 * Gens: Dummy Decompressor.                                               *
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

// This decompressor doesn't actually decompress anything.
// It's used if no other decompressors can recognize the file,
// in which case Gens assumes that the file is not compressed.

#include "dummy.h"

// Dummy decompressor functions.
static int decompressor_dummy_detect_format(FILE *zF);
static file_list_t* decompressor_dummy_get_file_info(FILE *zF, const char* filename);
static size_t decompressor_dummy_get_file(FILE *zF, const char* filename,
					  file_list_t *file_list,
					  void *buf, const size_t size);

// Dummy decompressor struct.
const decompressor_t decompressor_dummy =
{
	.detect_format	= decompressor_dummy_detect_format,
	.get_file_info	= decompressor_dummy_get_file_info,
	.get_file	= decompressor_dummy_get_file
};


/**
 * decompressor_dummy_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
static int decompressor_dummy_detect_format(FILE *zF)
{
	// Unused parameters.
	((void)zF);
	
	// The dummy decompressor accepts all files.
	return 1;
}


/**
 * decompressor_dummy_get_file_info(): Get information from all files in the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @return Pointer to the first file in the list, or NULL on error.
 */
static file_list_t* decompressor_dummy_get_file_info(FILE *zF, const char* filename)
{
	// The dummy decompressor treats the file as a single, uncompressed file.
	
	// Seek to the end of the file and get the position.
	fseek(zF, 0, SEEK_END);
	size_t filesize = ftell(zF);
	
	// Allocate memory for the file_list_t.
	file_list_t *file_list = (file_list_t*)malloc(sizeof(file_list_t));
	
	// Set the elements of the list.
	file_list->filesize = filesize;
	file_list->filename = (filename ? gens_strdup(filename) : NULL);
	file_list->next = NULL;
	
	// Return the list.
	return file_list;
}


/**
 * decompressor_dummy_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the GZip handler.)
 * @param filename Filename of the archive. (Unused in the GZip handler.)
 * @param file_list Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
size_t decompressor_dummy_get_file(FILE *zF, const char *filename,
				   file_list_t *file_list,
				   void *buf, const size_t size)
{
	// Unused parameters.
	((void)filename);
	((void)file_list);
	
	// All parameters (except filename and file_list) must be specified.
	if (!zF || !buf || !size)
		return 0;
	
	// Seek to the beginning of the file.
	fseek(zF, 0, SEEK_SET);
	
	// Read the file into the buffer.
	size_t retval = fread(buf, 1, size, zF);
	return retval;
}
