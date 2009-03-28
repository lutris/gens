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

// MiniZip.
#include "minizip/unzip.h"

// Zip decompressor functions.
static int decompressor_zip_detect_format(FILE *zF);
static file_list_t* decompressor_zip_get_file_info(FILE *zF, const char* filename);
static size_t decompressor_zip_get_file(FILE *zF, const char* filename,
					file_list_t *file_list,
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
 * decompressor_zip_get_file_info(): Get information from all files in the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @return Pointer to the first file in the list, or NULL on error.
 */
static file_list_t* decompressor_zip_get_file_info(FILE *zF, const char* filename)
{
	// Unused parameters.
	((void)zF);
	
	// Open the Zip file.
	unzFile f = unzOpen(filename);
	if (!f)
		return NULL;
	
	file_list_t *file_list_head = NULL;
	file_list_t *file_list_end = NULL;
	
	// Find the first ROM file in the Zip archive.
	int i = unzGoToFirstFile(f);
	char ROMFileName[132];
	unz_file_info zinfo;
	while (i == UNZ_OK)
	{
		unzGetCurrentFileInfo(f, &zinfo, ROMFileName, 128, NULL, 0, NULL, 0);
		
		// Allocate memory for the next file list element.
		file_list_t *file_list_cur = (file_list_t*)malloc(sizeof(file_list_t));
		
		// Store the ROM file information.
		file_list_cur->filename = gens_strdup(ROMFileName);
		file_list_cur->filesize = zinfo.uncompressed_size;
		file_list_cur->next = NULL;
		
		if (!file_list_head)
		{
			// List hasn't been created yet. Create it.
			file_list_head = file_list_cur;
			file_list_end = file_list_cur;
		}
		else
		{
			// Append the file list entry to the end of the list.
			file_list_end->next = file_list_cur;
			file_list_end = file_list_cur;
		}
		
		// Go to the next file.
		i = unzGoToNextFile(f);
	}
	
	// Close the Zip file.
	unzClose(f);
	
	// Return the list of files.
	return file_list_head;
}


/**
 * decompressor_zip_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the Zip handler.)
 * @param filename Filename of the archive.
 * @param file_list Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
static size_t decompressor_zip_get_file(FILE *zF, const char *filename,
					file_list_t *file_list,
					void *buf, const size_t size)
{
	// All parameters (except zF) must be specified.
	if (!filename || !file_list || !buf || !size)
		return 0;
	
	unzFile f = unzOpen(filename);
	if (!f)
		return 0;
	
	// Locate the ROM in the Zip file.
	if (unzLocateFile(f, file_list->filename, 1) != UNZ_OK ||
	    unzOpenCurrentFile(f) != UNZ_OK)
	{
		// Error loading the ROM file.
		LOG_MSG(z, LOG_MSG_LEVEL_CRITICAL,
			"Error extracting file '%s' from archive '%s'.",
			file_list->filename, filename);
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
			file_list->filename, filename, zip_err);
		return -1;
	}
	
	// Return the filesize.
	return size;
}
