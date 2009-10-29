/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_file.c: File handling functions.                                   *
 *                                                                         *
 * Copyright (c) 2009 by David Korth.                                      *
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

#include "gsft_file.h"
#include "gsft_szprintf.h"
#include "gsft_strlcpy.h"

#include <stdio.h>


/**
 * gsft_file_abs_to_rel(): Convert an absolute pathname to a relative pathname.
 * @param path_abs	[in] Absolute pathname to convert.
 * @param path_root	[in] Root of the relative path.
 * @param path_rel	[out] Buffer for the relative pathname.
 * @param path_rel_len	[in] Length of the relative pathname buffer.
 */
void GSFT_FNCALL gsft_file_abs_to_rel(const char *path_abs, const char *path_root,
				      char *path_rel, size_t path_rel_len)
{
	// Check if the absolute pathname has the root path in it.
	const size_t path_root_len = strlen(path_root);
	if (strncmp(path_abs, path_root, path_root_len) != 0)
	{
		// Root path not found. Copy the absolute path as-is.
		strlcpy(path_rel, path_abs, path_rel_len);
		return;
	}
	
	// Root path found.
	szprintf(path_rel, path_rel_len,
		 "." GSFT_DIR_SEP_STR "%s", &path_abs[path_root_len]);
	return;
}


/**
 * gsft_file_rel_to_abs(): Convert a relative pathname to an absolute pathname.
 * @param path_rel	[in] Relative pathname to convert.
 * @param path_root	[in] Root of the relative ptah.
 * @param path_abs	[out] Buffer for the absolute pathname.
 * @param path_abs_len	[in] Length of the absolute pathname buffer.
 */
void GSFT_FNCALL gsft_file_rel_to_abs(const char *path_rel, const char *path_root,
				      char *path_abs, size_t path_abs_len)
{
	// Check if the relative pathname is actually relative.
	if (path_rel[0] != '.' || path_rel[1] != GSFT_DIR_SEP_CHR)
	{
		// Not relative. Copy the path as-is.
		strlcpy(path_abs, path_rel, path_abs_len);
		return;
	}
	
	// Relative pathname.
	szprintf(path_abs, path_abs_len, "%s%s", path_root, &path_rel[2]);
	return;
}
