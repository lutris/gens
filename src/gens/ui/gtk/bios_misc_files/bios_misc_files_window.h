/***************************************************************************
 * Gens: (GTK+) BIOS/Misc Files Window                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#ifndef GTK_BIOS_MISC_FILES_WINDOW_H
#define GTK_BIOS_MISC_FILES_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

// BIOS/Misc File entries
// If entry is NULL, it's a frame heading.
struct BIOSMiscFileEntry_t
{
	const char* title;
	const char* tag;
	FileFilterType filter;
	char* entry;
};

// Contains all the BIOS/Misc File entries.
extern const struct BIOSMiscFileEntry_t BIOSMiscFiles[];

GtkWidget* create_bios_misc_files_window(void); 
extern GtkWidget *bios_misc_files_window;

#ifdef __cplusplus
}
#endif

#endif
