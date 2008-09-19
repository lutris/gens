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
 
#ifndef GENS_UI_HPP
#define GENS_UI_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "emulator/gens.hpp"

typedef enum
{
	AnyFile		= 0,
	ROMFile		= 1,
	SavestateFile	= 2,
	CDImage		= 3,
	ConfigFile	= 4,
	GYMFile		= 5,
} FileFilterType;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>
using std::string;

// Message box icons for GensUI::msgBox().
enum MSGBOX_ICON
{
	MSGBOX_ICON_NONE = 0,
	MSGBOX_ICON_INFO = 1,
	MSGBOX_ICON_QUESTION = 2,
	MSGBOX_ICON_WARNING = 3,
	MSGBOX_ICON_ERROR = 4,
};

class GensUI
{
	public:
		// The following functions are implemented by ui/[ui]/gens_ui_[ui].cpp.
		static void init(int argc, char *argv[]);
		static void update(void);
		static void sleep(const int ms);
		
		static void setWindowTitle(const string& title);
		static void setWindowVisibility(const bool visibility);
		
		static void UI_Hide_Embedded_Window(void);
		static void UI_Show_Embedded_Window(const int w, const int h);
		static int UI_Get_Embedded_WindowID(void);
		
		static void msgBox(const string& msg, const string& title = GENS_APPNAME, const MSGBOX_ICON icon = MSGBOX_ICON_INFO);
		static string openFile(const string& title, const string& initFile, const FileFilterType filterType);
		static string saveFile(const string& title, const string& initFile, const FileFilterType filterType);
		static string selectDir(const string& title, const string& initDir);
		
		// The following functions are implemented by ui/gens_ui.cpp.
		static void setWindowTitle_Idle(void);
		static void setWindowTitle_Game(const string& system, const string& game);
		static void setWindowTitle_Init(const string& system, const bool reinit);
};

#endif /* __cplusplus */

#endif /* GENS_UI_HPP */
