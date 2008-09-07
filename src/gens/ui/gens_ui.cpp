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
#include "emulator/gens.h"
#include "gens_ui.hpp"

#include <sstream>
using std::stringstream;


/**
 * setWindowTitle_Idle(): Set the window title to "Idle".
 */
void GensUI::setWindowTitle_Idle(void)
{
	setWindowTitle(GENS_APPNAME " " GENS_VERSION " - Idle");
}


/**
 * setWindowTitle_Game(): Set the window title to the system name, followed by the game name.
 * @param system System name.
 * @param game Game name.
 */
void GensUI::setWindowTitle_Game(const string& systemName, const string& gameName)
{
	stringstream ss;
	string title;
	string condGameName;
	char curChar = 0x00;
	
	// Condense the game name by removing excess spaces.
	for (unsigned int cpos = 0; cpos < gameName.length(); cpos++)
	{
		if (curChar == ' ' && gameName.at(cpos) == ' ')
			continue;
		curChar = gameName.at(cpos);
		ss << curChar;
	}
	
	// Trim any excess spaces.
	condGameName = ss.str();
	if (condGameName.length() > 0)
	{
		if (condGameName.at(condGameName.length() - 1) == ' ')
			condGameName = condGameName.substr(0, condGameName.length() - 1);
		if (condGameName.at(0) == ' ')
			condGameName = condGameName.substr(1, condGameName.length() - 1);

	}
	
	// Create the title.
	title = string(GENS_APPNAME) + " " + GENS_VERSION + " - " + systemName + ": " + condGameName;
	
	// Set the title.
	setWindowTitle(title);
}


/**
 * setWindowTitle_Init(): Set the window title to the system name, followed by "Initializing, please wait..."
 * @param system System name.
 * @param reinit If true, sets the title to "Re-initializing" instead of "Initializing".
 */
void GensUI::setWindowTitle_Init(const string& system, const bool reinit)
{
	string title;
	
	// Create the title.
	title = string(GENS_APPNAME) + " " + GENS_VERSION + " - " + system +
		(reinit ? "Reinitializing" : "Initializing") + ", please wait...";
	
	// Set the title.
	setWindowTitle(title);
}
