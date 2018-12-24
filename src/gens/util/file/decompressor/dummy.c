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

// MDP includes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_strdup.h"
#include "libgsft/gsft_unused.h"

// C includes.
#include <stdlib.h>


// Dummy decompressor functions.
static int decompressor_dummy_detect_format(FILE *zF);
static int decompressor_dummy_get_file_info(FILE *zF, const char* filename,
					    mdp_z_entry_t** z_entry_out);
static size_t decompressor_dummy_get_file(FILE *zF, const char* filename,
					  mdp_z_entry_t *z_entry,
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
	GSFT_UNUSED_PARAMETER(zF);
	
	// The dummy decompressor accepts all files.
	return 1;
}


/**
 * decompressor_dummy_get_file_info(): Get information from all files in the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param z_entry_out	[out] Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code.
 */
static int decompressor_dummy_get_file_info(FILE *zF, const char* filename, mdp_z_entry_t** z_entry_out)
{
	GSFT_UNUSED_PARAMETER(filename);
	
	if (!zF || !z_entry_out)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// The dummy decompressor treats the file as a single, uncompressed file.
	
	// Seek to the end of the file and get the position.
	fseek(zF, 0, SEEK_END);
	size_t filesize = ftell(zF);
	
	// Allocate memory for the mdp_z_entry_t.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
	
	// Set the elements of the list.
	z_entry->filesize = filesize;
	z_entry->filename = (filename ? strdup(filename) : NULL);
	z_entry->next = NULL;
	
	// Return the list.
	*z_entry_out = z_entry;
	return MDP_ERR_OK;
}


/**
 * decompressor_dummy_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the GZip handler.)
 * @param filename Filename of the archive. (Unused in the GZip handler.)
 * @param z_entry Pointer to mdp_z_entry_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
size_t decompressor_dummy_get_file(FILE *zF, const char *filename,
				   mdp_z_entry_t *z_entry,
				   void *buf, const size_t size)
{
	GSFT_UNUSED_PARAMETER(filename);
	GSFT_UNUSED_PARAMETER(z_entry);
	
	// All parameters (except filename and file_list) must be specified.
	if (!zF || !buf || !size)
		return 0;
	
	// Seek to the beginning of the file.
	fseek(zF, 0, SEEK_SET);
	
	// Read the file into the buffer.
	size_t retval = fread(buf, 1, size, zF);
	return retval;
}
