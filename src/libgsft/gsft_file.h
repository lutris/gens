/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_file.h: File handling functions.                                   *
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

#ifndef __GSFT_FILE_H
#define __GSFT_FILE_H

#ifdef _WIN32
#define GSFT_DIR_SEP_CHR '\\'
#define GSFT_DIR_SEP_STR "\\"
#else
#define GSFT_DIR_SEP_CHR '/'
#define GSFT_DIR_SEP_STR "/"
#endif

#include "gsft_fncall.h"
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * gsft_file_abs_to_rel(): Convert an absolute pathname to a relative pathname.
 * @param path_abs	[in] Absolute pathname to convert.
 * @param path_root	[in] Root of the relative path.
 * @param path_rel	[out] Buffer for the relative pathname.
 * @param path_rel_len	[in] Length of the relative pathname buffer.
 */
DLL_LOCAL void GSFT_FNCALL gsft_file_abs_to_rel(const char *path_abs, const char *path_root,
						char *path_rel, size_t path_rel_len);

/**
 * gsft_file_rel_to_abs(): Convert a relative pathname to an absolute pathname.
 * @param path_rel	[in] Relative pathname to convert.
 * @param path_root	[in] Root of the relative ptah.
 * @param path_abs	[out] Buffer for the absolute pathname.
 * @param path_abs_len	[in] Length of the absolute pathname buffer.
 */
DLL_LOCAL void GSFT_FNCALL gsft_file_rel_to_abs(const char *path_rel, const char *path_root,
						char *path_abs, size_t path_abs_len);


/**
 * gsft_file_exists(): Determine if a file exists.
 * @param filename	[in] Filename.
 * @return 0 if the file doesn't exist; non-zero if the file exists.
 */
#define gsft_file_exists(filename)	(!access(filename, F_OK))


#ifdef __cplusplus
}
#endif

#endif /* __GSFT_FILE_H */
