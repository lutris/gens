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

#include "bmf_window_common.h"

#include "gens_ui.hpp"
#include "emulator/g_main.hpp"

// C includes.
#include <string.h>

// All textboxes to be displayed on the BIOS/Misc Files window are defined here.
const bmf_entry_t bmf_entries[] =
{
	{"Genesis BIOS File",		0, NULL},
	{"Genesis",			ROMFile, BIOS_Filenames.MD_TMSS},
	{"External 32X Firmware",	0, NULL},
	{"MC68000",			ROMFile, BIOS_Filenames._32X_MC68000},
	{"Master SH2",			ROMFile, BIOS_Filenames._32X_MSH2},
	{"Slave SH2",			ROMFile, BIOS_Filenames._32X_SSH2},
	{"SegaCD BIOS Files",		0, NULL},
	{"USA",				ROMFile, BIOS_Filenames.SegaCD_US},
	{"Europe",			ROMFile, BIOS_Filenames.MegaCD_EU},
	{"Japan",			ROMFile, BIOS_Filenames.MegaCD_JP},
	{"Compression Utilities",	0, NULL},
	{"RAR Binary",			AnyFile, Misc_Filenames.RAR_Binary},
	{NULL, 0, NULL},
};
