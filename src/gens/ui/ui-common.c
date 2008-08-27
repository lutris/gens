/***************************************************************************
 * Gens: Common UI functions.                                              *
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


#include <stdio.h>
#include "gens.h"
#include "ui-common.h"


/**
 * UI_Set_Window_Title_Idle(): Set the window title to "Idle".
 */
void UI_Set_Window_Title_Idle(void)
{
	UI_Set_Window_Title(GENS_APPNAME " " GENS_VERSION " - Idle");
}


/**
 * UI_Set_Window_Title_Game(): Set the window title to the system name, followed by the game name.
 * @param system System name.
 * @param game Game name.
 */
void UI_Set_Window_Title_Game(const char* systemName, const char* gameName)
{
	char title[128];
	char condGameName[64];
	int gni, cgni = 0;
	
	// Condense the game name by removing excess spaces.
	for (gni = 0; gni < 64; gni++)
	{
		if (cgni != 0 && condGameName[cgni - 1] == ' ' && gameName[gni] == ' ')
			continue;
		else if (cgni == 0 && gameName[gni] == ' ')
			continue;
		condGameName[cgni] = gameName[gni];
		cgni++;
	}
	condGameName[cgni] = 0x00;
	
	// Create the title.
	sprintf(title, GENS_APPNAME " " GENS_VERSION " - %s: %s", systemName, condGameName);
	
	// Set the title.
	UI_Set_Window_Title(title);
}


/**
 * UI_Set_Window_Title_Init(): Set the window title to the system name, followed by "Initializing, please wait..."
 * @param system System name.
 * @param reinit If non-zero, sets the title to "Re-initializing" instead of "Initializing".
 */
void UI_Set_Window_Title_Init(const char* system, int reinit)
{
	char title[128];
	
	// Create the title.
	sprintf(title, GENS_APPNAME " " GENS_VERSION " - %s: %s, please wait...", system,
		(reinit ? "Reinitializing" : "Initializing"));
	
	// Set the title.
	UI_Set_Window_Title(title);
}
