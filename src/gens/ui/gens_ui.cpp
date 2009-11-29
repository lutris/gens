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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gens_ui.hpp"

// C includes.
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

// C++ includes.
#include <sstream>
#include <string>
using std::stringstream;
using std::string;

// libgsft includes.
#include "libgsft/gsft_space_elim.h"


// Full Screen Minization counters.
int GensUI::fsMinimize_Counter[(int)FSMINIMIZE_MAX] = {0, 0};
bool GensUI::fsMinimize_OldFS[(int)FSMINIMIZE_MAX] = {false, false};


/**
 * setWindowTitle_withAppVersion(): Set the window title with the app name and version at the beginning.
 * @param title Window title.
 */
static inline void setWindowTitle_withAppVersion(const string& title)
{
#ifdef GENS_GS_VERSION
	GensUI::setWindowTitle(GENS_APPNAME " " GENS_GS_VERSION " - " + title);
#else
	GensUI::setWindowTitle(GENS_APPNAME " - " + title);
#endif
}


/**
 * setWindowTitle_Idle(): Set the window title to "Idle".
 */
void GensUI::setWindowTitle_Idle(void)
{
	setWindowTitle_withAppVersion("Idle");
}


/**
 * isGraphChar(): Determine if a character is a graphical character.
 * @param chr Character.
 * @return True if the character is graphical; false if it's a space.
 */
static inline bool isGraphChar(char chr)
{
	return (isgraph(chr) || (chr & 0x80));
}


/**
 * setWindowTitle_Game(): Set the window title to the system name, followed by the game name.
 * @param systemName System name.
 * @param gameName Game name.
 * @param emptyGameName String to use for the game name if gameName is empty.
 */
void GensUI::setWindowTitle_Game(const string& systemName, const string& gameName,
				 const string& emptyGameName)
{
	string condGameName;
	
	if (gameName.empty())
	{
		// Specified game name is empty. Use the empty game name.
		condGameName = emptyGameName;
	}
	else
	{
		// Specified game name is not empty.
		// Run it through the space elimination algorithm.
		char *buf = (char*)malloc(gameName.size() + 1);
		gsft_space_elim(gameName.c_str(), gameName.size(), buf);
		condGameName = (buf[0] != 0x00 ? string(buf) : emptyGameName);
		free(buf);
	}
	
	// Set the title.
	setWindowTitle_withAppVersion(systemName + ": " + string(condGameName));
}


/**
 * setWindowTitle_Init(): Set the window title to the system name, followed by "Initializing, please wait..."
 * @param system System name.
 * @param reinit If true, sets the title to "Re-initializing" instead of "Initializing".
 */
void GensUI::setWindowTitle_Init(const string& system, const bool reinit)
{
	setWindowTitle_withAppVersion(system + ": " + (reinit ? "Rei" : "I") + "nitializing, please wait...");
}


/**
 * GensUI_update(): Update the Gens UI.
 * Wrapper function for C code.
 */
void GensUI_update(void)
{
	GensUI::update();
}


/**
 * GensUI_wakeup(): Wakeup from sleep.
 * Wrapper function for C code.
 */
void GensUI_wakeup(void)
{
	GensUI::wakeup();
}


/**
 * GensUI_setWindowVisibility: Set window visibility.
 * Wrapper function for C code.
 */
void GensUI_setWindowVisibility(const int visibility)
{
	GensUI::setWindowVisibility(visibility);
}
