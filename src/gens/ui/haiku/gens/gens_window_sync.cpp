/***************************************************************************
 * Gens: (Haiku) Main Window - Synchronization Functions.                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Copyright (c) 2009 by Phil Costin                                       *
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

// Message logging.
#include "macros/log_msg.h"


/**
 * STUB functions. These are merely present to allow
 * the program to be compiled without problems.
 *
 * TODO: Fill in these functions!
 */


// Internal functions.
// TODO: Use appropriate parameters to indicate the parent menu.
static void Sync_Gens_Window_GraphicsMenu_Backend(void);
static void Sync_Gens_Window_GraphicsMenu_Render(void);
#ifdef GENS_DEBUGGER
static void Sync_Gens_Window_CPUMenu_Debug(void);
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window(): Synchronize the Gens Main Window.
 */
void Sync_Gens_Window(void)
{
	STUB();
}


/**
 * Sync_Gens_Window_FileMenu(): Synchronize the File Menu.
 * This does NOT synchronize the ROM History submenu.
 */
void Sync_Gens_Window_FileMenu(void)
{
	STUB();
}


/**
 * Sync_Gens_Window_FileMenu_ROMHistory(): Synchronize the File, ROM History submenu.
 */
void Sync_Gens_Window_FileMenu_ROMHistory(void)
{
	STUB();
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	STUB();
}


/**
 * Sync_Gens_Window_GraphicsMenu_Backend(): Synchronize the Graphics, Backend submenu.
 * TODO: Use appropriate parameters to indicate the parent menu.
 */
static void Sync_Gens_Window_GraphicsMenu_Backend(HMENU parent, int position)
{
	STUB();
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * TODO: Use appropriate parameters to indicate the parent menu.
 */
static void Sync_Gens_Window_GraphicsMenu_Render(void)
{
	STUB();
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
	STUB();
}


#ifdef GENS_DEBUGGER
/**
 * Sync_Gens_Window_CPUMenu_Debug(): Synchronize the Graphics, Render submenu.
 * TODO: Use appropriate parameters to indicate the parent menu.
 */
static void Sync_Gens_Window_CPUMenu_Debug(void)
{
	STUB();
}
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
	STUB();
}


/**
 * Sync_Gens_Window_PluginsMenu(): Synchronize the Plugins menu.
 */
void Sync_Gens_Window_PluginsMenu(void)
{
	STUB();
}


/**
 * Sync_Gens_Window_OptionsMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
	STUB();
}
