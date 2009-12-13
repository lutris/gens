/***************************************************************************
 * Gens: RAR Decompressor. (Win32)                                         *
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

#ifndef GENS_DECOMPRESSOR_MD_RAR_WIN32_HPP
#define GENS_DECOMPRESSOR_MD_RAR_WIN32_HPP

#include "decompressor.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// RAR decompressor functions.
int decompressor_rar_win32_detect_format(FILE *zF);
int decompressor_rar_win32_get_file_info(FILE *zF, const char* filename,
						mdp_z_entry_t** z_entry_out);
int decompressor_rar_win32_get_file(FILE *zF, const char *filename,
					mdp_z_entry_t *z_entry, void *buf,
					const size_t size, size_t *ret_size);

#ifdef __cplusplus
}
#endif

#endif /* GENS_DECOMPRESSOR_MD_RAR_WIN32_HPP */
