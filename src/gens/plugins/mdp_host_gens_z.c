/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (File Compression)        *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mdp_host_gens_z.h"
#include "mdp/mdp_error.h"

#ifdef GENS_ZLIB
	#include <zlib.h>
#endif

/* C includes. */
#include <stdlib.h>

/* File Decompressors. */
#include "util/file/decompressor/decompressor.h"
#include "util/file/decompressor/dummy.h"
#ifdef GENS_ZLIB
	#include "util/file/decompressor/md_gzip.h"
	#include "util/file/decompressor/md_zip.h"
#endif
#ifdef GENS_LZMA
	#include "util/file/decompressor/md_7z.h"
#endif
#include "util/file/decompressor/md_rar_t.h"

/* libgsft includes. */
#include "libgsft/gsft_strdup.h"

/* Array of decompressors. */
static const decompressor_t* const decompressors[] =
{
	#ifdef GENS_ZLIB
		&decompressor_gzip,
		&decompressor_zip,
	#endif
	#ifdef GENS_LZMA
		&decompressor_7z,
	#endif
	&decompressor_rar,
	
	// Last decompressor is the Dummy decompressor.
	&decompressor_dummy,
	NULL
};


/**
 * mdp_host_crc32(): Calculate the CRC32 of the specified data.
 * @param buf		[in]  Data.
 * @param length	[in]  Length of the data.
 * @param crc32_out	[out] Pointer to variable to store the CRC32 in.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_crc32(const uint8_t* buf, int length, uint32_t *crc32_out)
{
#ifndef GENS_ZLIB
	/* ZLib support wasn't compiled in. */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
#else
	/* ZLib support was compiled in. */
	if (!crc32_out)
		return -MDP_ERR_UNKNOWN;	/* TODO: Add a specific error code for this. */
	
	*crc32_out = crc32(0, buf, length);
	return MDP_ERR_OK;
#endif
}


/**
 * mdp_host_z_open(): Open a compressed file.
 * @param filename	[in]  Filename of the compressed file.
 * @param z_out		[out] Pointer to mdp_z_t*, which will contain an allocated mdp_z_t.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_z_open(const char* filename, mdp_z_t **z_out)
{
	if (!filename || !z_out)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Attempt to open the file.
	FILE *f = fopen(filename, "rb");
	if (!f)
		return -MDP_ERR_Z_ARCHIVE_NOT_FOUND;
	
	/* Attempt to find a usable decompressor. */
	const decompressor_t *cmp = NULL;
	int zID = 0;
	while (decompressors[zID])
	{
		if (decompressors[zID]->detect_format(f))
		{
			/* Found a usable decompressor. */
			cmp = decompressors[zID];
			break;
		}
		
		/* Next decompressor. */
		zID++;
	}
	
	if (!cmp)
	{
		/* No decompressors found. */
		/* This is an error, since the "dummy" decompressor should always be usable. */
		fclose(f);
		return -MDP_ERR_Z_ARCHIVE_NOT_SUPPORTED;
	}
	
	/* Get the file information. */
	mdp_z_entry_t *z_entry_head = NULL;
	int rval = cmp->get_file_info(f, filename, &z_entry_head);
	if (rval != MDP_ERR_OK)
	{
		/* An error occurred while attempting to read the archive. */
		fclose(f);
		
		if (z_entry_head)
		{
			// Free the allocated mdp_z_entry_t.
			z_entry_t_free(z_entry_head);
		}
		
		return rval;
	}
	
	/* Allocate the mdp_z_t. */
	mdp_z_t *z = (mdp_z_t*)malloc(sizeof(mdp_z_t));
	
	/* Fill the data in the mdp_z_t struct. */
	z->files	= z_entry_head;
	z->filename	= strdup(filename);
	z->f		= f;
	z->data		= (void*)(cmp);		/* Data parameter contains decompressor function pointers. */
	
	/* Return the mdp_z_t struct. */
	*z_out = z;
	
	return MDP_ERR_OK;
}


/* TODO: Test this function. */
/**
 * mdp_host_z_get_file(): Get a file from an opened compressed archive.
 * @param z_file Opened compressed archive.
 * @param z_entry File to open.
 * @param buf Buffer to store the file in.
 * @param size Size of the buffer.
 * @return Size of data read, or MDP error code.
 */
int MDP_FNCALL mdp_host_z_get_file(mdp_z_t *z_file, mdp_z_entry_t *z_entry, void *buf, size_t size)
{
	if (!z_file || !z_entry || !buf || size == 0)
	{
		/* Missing parameter. */
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	else if (!z_file->files || !z_file->filename || !z_file->f || !z_file->data)
	{
		/* One or more required elements in z_file is/are missing. */
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	
	/* Get the file from the decompressor. */
	decompressor_t *cmp = (decompressor_t*)(z_file->data);
	int rval = cmp->get_file(z_file->f, z_file->filename, z_entry, buf, size);
	
	/* If rval is positive, it's a filesize. */
	/* If 0, return an error code. */
	if (rval == 0)
		return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	else
		return rval;
}


/* TODO: Test this function. */
/**
 * mdp_host_z_close(): Close a compressed archive.
 * @param z_file Opened compressed archive.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_z_close(mdp_z_t *z_file)
{
	if (!z_file)
	{
		/* Missing parameter. */
		return -MDP_ERR_INVALID_PARAMETERS;
	}
	
	/* Close the FILE stream. */
	if (z_file->f)
		fclose(z_file->f);
	
	/* Free the filename. */
	if (z_file->filename)
		free(z_file->filename);
	
	/* Free the list of file entries. */
	if (z_file->files)
	{
		mdp_z_entry_t *z_cur = z_file->files;
		mdp_z_entry_t *z_next;
		
		do
		{
			if (z_cur->filename)
				free(z_cur->filename);
			
			/* Save the "next" pointer. */
			z_next = z_cur->next;
			
			/* Free the current entry. */
			free(z_cur);
			
			/* Go to the next entry. */
			z_cur = z_next;
		} while (z_cur);
	}
	
	/* Free the mdp_z_t. */
	free(z_file);
	
	return MDP_ERR_OK;
}
