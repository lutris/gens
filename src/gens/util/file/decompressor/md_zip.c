/***************************************************************************
 * Gens: Zip Decompressor.                                                 *
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

#include "md_zip.h"

// Message logging.
#include "macros/log_msg.h"

// C includes.
#include <stdlib.h>

// MiniZip.
#include "minizip/unzip.h"

// MDP includes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_strdup.h"
#include "libgsft/gsft_unused.h"


// Zip decompressor functions.
static int decompressor_zip_detect_format(FILE *zF);
static int decompressor_zip_get_file_info(FILE *zF, const char* filename,
					  mdp_z_entry_t** z_entry_out);
static size_t decompressor_zip_get_file(FILE *zF, const char* filename,
					mdp_z_entry_t *z_entry,
					void *buf, const size_t size);

// Zip decompressor struct.
const decompressor_t decompressor_zip =
{
	.detect_format	= decompressor_zip_detect_format,
	.get_file_info	= decompressor_zip_get_file_info,
	.get_file	= decompressor_zip_get_file
};


/**
 * decompressor_zip_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
static int decompressor_zip_detect_format(FILE *zF)
{
	// Magic Number for Zip:
	// First four bytes: "PK\x03\x04"
	static const unsigned char zip_magic[] = {'P', 'K', 0x03, 0x04};
	
	unsigned char buf[sizeof(zip_magic)];
	fseek(zF, 0, SEEK_SET);
	fread(buf, sizeof(unsigned char), sizeof(zip_magic), zF);
	return (memcmp(buf, zip_magic, sizeof(zip_magic)) == 0);
}


/**
 * decompressor_zip_get_file_info(): Get information about all files in the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param z_entry_out	[out] Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code.
 */
static int decompressor_zip_get_file_info(FILE *zF, const char* filename, mdp_z_entry_t** z_entry_out)
{
	GSFT_UNUSED_PARAMETER(zF);
	
	if (!filename || !z_entry_out)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Open the Zip file.
	unzFile f = unzOpen(filename);
	if (!f)
		return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	
	mdp_z_entry_t *z_entry_head = NULL;
	mdp_z_entry_t *z_entry_end = NULL;
	
	// Find the first ROM file in the Zip archive.
	int i = unzGoToFirstFile(f);
	char ROMFileName[132];
	unz_file_info zinfo;
	while (i == UNZ_OK)
	{
		unzGetCurrentFileInfo(f, &zinfo, ROMFileName, 128, NULL, 0, NULL, 0);
		
		// Allocate memory for the next file list element.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
		
		// Store the ROM file information.
		z_entry_cur->filename = strdup(ROMFileName);
		z_entry_cur->filesize = zinfo.uncompressed_size;
		z_entry_cur->next = NULL;
		
		if (!z_entry_head)
		{
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_end = z_entry_cur;
		}
		else
		{
			// Append the file list entry to the end of the list.
			z_entry_end->next = z_entry_cur;
			z_entry_end = z_entry_cur;
		}
		
		// Go to the next file.
		i = unzGoToNextFile(f);
	}
	
	// Close the Zip file.
	unzClose(f);
	
	// If there are no files in the archive, return an error.
	if (!z_entry_head)
		return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	
	// Return the list of files.
	*z_entry_out = z_entry_head;
	return MDP_ERR_OK;
}


/**
 * decompressor_zip_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the Zip handler.)
 * @param filename Filename of the archive.
 * @param z_entry Pointer to mdp_z_entry_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
static size_t decompressor_zip_get_file(FILE *zF, const char *filename,
					mdp_z_entry_t *z_entry,
					void *buf, const size_t size)
{
	GSFT_UNUSED_PARAMETER(zF);
	
	// All parameters (except zF) must be specified.
	if (!filename || !z_entry || !buf || !size)
		return 0;
	
	unzFile f = unzOpen(filename);
	if (!f)
		return 0;
	
	// Locate the ROM in the Zip file.
	if (unzLocateFile(f, z_entry->filename, 1) != UNZ_OK ||
	    unzOpenCurrentFile(f) != UNZ_OK)
	{
		// Error loading the ROM file.
		LOG_MSG(z, LOG_MSG_LEVEL_CRITICAL,
			"Error extracting file '%s' from archive '%s'.",
			z_entry->filename, filename);
		unzClose(f);
		return -1;
	}
	
	// Decompress the ROM.
	int zResult = unzReadCurrentFile(f, buf, size);
	unzClose(f);
	if ((zResult <= 0) || (zResult != size))
	{
		const char* zip_err;
		
		switch (zResult)
		{
			case UNZ_ERRNO:
				zip_err = "Unknown...";
				break;
			case UNZ_EOF:
				zip_err = "Unexpected end of file.";
				break;
			case UNZ_PARAMERROR:
				zip_err = "Parameter error.";
				break;
			case UNZ_BADZIPFILE:
				zip_err = "Bad ZIP file.";
				break;
			case UNZ_INTERNALERROR:
				zip_err = "Internal error.";
				break;
			case UNZ_CRCERROR:
				zip_err = "CRC error.";
				break;
			default:
				zip_err = "Unknown error.";
				break;
		}
		
		LOG_MSG(z, LOG_MSG_LEVEL_CRITICAL,
			"Error extracting file '%s' from archive '%s': %s",
			z_entry->filename, filename, zip_err);
		return -1;
	}
	
	// Return the filesize.
	return size;
}
