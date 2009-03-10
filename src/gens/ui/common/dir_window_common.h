/***************************************************************************
 * Gens: Directory Configuration Window. (Common Data)                     *
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

#ifndef GENS_UI_COMMON_DIR_WINDOW_HPP
#define GENS_UI_COMMON_DIR_WINDOW_HPP

#ifdef __cplusplus
extern "C" {
#endif

// Directory struct.
typedef struct _dir_entry_t
{
	const char* title;
	char* entry;
} dir_entry_t;

// TODO: Make the number of directory entries dynamic.
#define DIR_WINDOW_ENTRIES_COUNT 6
extern const dir_entry_t dir_window_entries[DIR_WINDOW_ENTRIES_COUNT + 1];

#ifdef __cplusplus
}
#endif

#endif /* GENS_UI_COMMON_DIR_WINDOW_HPP */
