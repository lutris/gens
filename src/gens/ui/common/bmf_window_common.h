/***************************************************************************
 * Gens: BIOS/Misc Files Window. (Common Data)                             *
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

#ifndef GENS_UI_COMMON_BIOS_MISC_FILES_WINDOW_HPP
#define GENS_UI_COMMON_BIOS_MISC_FILES_WINDOW_HPP

#include "gens_ui.hpp"

#ifdef __cplusplus
extern "C" {
#endif

// BIOS/Misc File entries
// If entry is NULL, it's a frame heading.
typedef struct _bmf_entry_t
{
	const char* title;
	FileFilterType filter;
	char* entry;
} bmf_entry_t;

extern const bmf_entry_t bmf_entries[];

#ifdef __cplusplus
}
#endif

#endif /* GENS_UI_COMMON_BIOS_MISC_FILES_WINDOW_HPP */
