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

// C++ includes.
#include <sstream>
#include <string>
using std::stringstream;
using std::string;


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
 * setWindowTitle_Game(): Set the window title to the system name, followed by the game name.
 * @param systemName System name.
 * @param gameName Game name.
 * @param emptyGameName String to use for the game name if gameName is empty.
 */
void GensUI::setWindowTitle_Game(const string& systemName, const string& gameName,
				 const string& emptyGameName)
{
	stringstream ss;
	string condGameName;
	char curChar = 0x00;
	
	// Condense the game name by removing excess spaces.
	for (unsigned int cpos = 0; cpos < gameName.length(); cpos++)
	{
		if (!isgraph(curChar) && isspace(gameName.at(cpos)))
			continue;
		curChar = gameName.at(cpos);
		ss << curChar;
	}
	
	// Trim any excess spaces.
	condGameName = ss.str();
	
	if (condGameName.length() > 0 &&
	    isspace(condGameName.at(condGameName.length() - 1)))
	{
		condGameName = condGameName.substr(0, condGameName.length() - 1);
	}
	
	if (condGameName.length() > 0 && isspace(condGameName.at(0)))
	{
		condGameName = condGameName.substr(1, condGameName.length() - 1);
	}
	
	// Check if the condensed game name is empty.
	if (condGameName.empty())
	{
		// Condensed game name is empty.
		// Replace it with emptyGameName.
		condGameName = emptyGameName;
	}
	
	// Set the title.
	setWindowTitle_withAppVersion(systemName + ": " + condGameName);
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
	// TODO: Implement GensUI::setWindowVisibility() on Win32.
#ifndef GENS_OS_WIN32
	GensUI::setWindowVisibility(visibility);
#endif
}
