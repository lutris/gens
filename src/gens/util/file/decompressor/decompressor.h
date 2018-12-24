/***************************************************************************
 * Gens: File Decompression Function Definitions.                          *
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

#ifndef GENS_DECOMPRESSOR_H
#define GENS_DECOMPRESSOR_H

#ifdef __cplusplus
extern "C" {
#endif

// C includes.
#include <stdio.h>
#include <string.h>

// MDP decompression structs.
#include "mdp/mdp_z.h"

/**
 * z_entry_t_free(): Free a list of files.
 * @param z_entry Pointer to the first file in the list.
 */
void z_entry_t_free(mdp_z_entry_t *z_entry);

/**
 * decompressor_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
typedef int (*decompressor_detect_format)(FILE *zF);

/**
 * decompressor_get_file_info(): Get information about all files in the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param z_entry_out	[out] Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code.
 */
typedef int (*decompressor_get_file_info)(FILE *zF, const char *filename, mdp_z_entry_t** z_entry_out);

/**
 * decompressor_get_file(): Get a file from the archive.
 * @param zF Open file handle.
 * @param filename Filename of the archive.
 * @param file_list Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
typedef size_t (*decompressor_get_file)(FILE *zF, const char *filename,
					mdp_z_entry_t *z_entry,
					void *buf, const size_t size);

/**
 * decompressor_t: Struct containing function pointers to various decompresssors.
 */
typedef struct _decompressor_t
{
	decompressor_detect_format	detect_format;
	decompressor_get_file_info	get_file_info;
	decompressor_get_file		get_file;
} decompressor_t;

#ifdef __cplusplus
}
#endif

#endif /* GENS_DECOMPRESSOR_H */
