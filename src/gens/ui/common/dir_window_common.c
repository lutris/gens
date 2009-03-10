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

#include "dir_window_common.h"

// Includes with directory defines.
// TODO: Consolidate these into Settings.PathNames
#include "util/file/save.hpp"
#include "util/gfx/imageutil.hpp"
#include "util/file/rom.hpp"


// Directory entries.
const dir_entry_t dir_window_entries[DIR_WINDOW_ENTRIES_COUNT + 1] =
{
	{"Savestates",	State_Dir},
	{"SRAM Backup",	SRAM_Dir},
	{"BRAM Backup",	BRAM_Dir},
	{"WAV Dump",	PathNames.Dump_WAV_Dir},
	{"GYM Dump",	PathNames.Dump_GYM_Dir},
	{"Screenshots",	PathNames.Screenshot_Dir},
	{NULL, NULL}
};
