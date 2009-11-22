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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "emulator/gens.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	AnyFile		= 0,
	ROMFile		= 1,
	SavestateFile	= 2,
	CDImage		= 3,
	ConfigFile	= 4,
	GYMFile		= 5
} FileFilterType;

/**
 * GensUI_update(): Update the Gens UI.
 * Wrapper function for C code.
 */
void GensUI_update(void);

/**
 * GensUI_wakeup(): Wakeup from sleep.
 * Wrapper function for C code.
 */
void GensUI_wakeup(void);

/**
 * GensUI_setWindowVisibility: Set window visibility.
 * Wrapper function for C code.
 */
void GensUI_setWindowVisibility(const int visibility);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>

class GensUI
{
	public:
		// The following functions are implemented by ui/[ui]/gens_ui_[ui].cpp.
		static void init(int *argc, char **argv[]);
		static void update(void);
		static void sleep(const int ms, const bool noUpdate = false);
		static void wakeup(void);
		
		static void setWindowTitle(const std::string& title);
		static void setWindowVisibility(const bool visibility);
		
		static std::string openFile(const std::string& title, const std::string& initFile,
						const FileFilterType filterType, void* owner = NULL);
		static std::string saveFile(const std::string& title, const std::string& initFile,
						const FileFilterType filterType, void* owner = NULL);
		static std::string selectDir(const std::string& title, const std::string& initDir,
						void* owner = NULL);
		
		static void setMousePointer(bool busy);
		
		// The following functions are implemented by ui/gens_ui.cpp.
		static void setWindowTitle_Idle(void);
		static void setWindowTitle_Game(const std::string& system, const std::string& game,
						const std::string& emptyGameName = "");
		static void setWindowTitle_Init(const std::string& system, const bool reinit);
		
		// Message box styles for GensUI::msgBox().
		// Format: 00000000 00000000 00000000 BBBBIIII
		// IIII == Icon
		// BBBB == Buttons
		
		// Buttons
		static const unsigned int MSGBOX_BUTTONS_MASK		= 0x0000000F;
		static const unsigned int MSGBOX_BUTTONS_OK		= (0 << 0);
		static const unsigned int MSGBOX_BUTTONS_OK_CANCEL	= (1 << 0);
		static const unsigned int MSGBOX_BUTTONS_YES_NO		= (3 << 0);
		
		// Icons
		static const unsigned int MSGBOX_ICON_MASK		= 0x000000F0;
		static const unsigned int MSGBOX_ICON_NONE		= (0 << 4);
		static const unsigned int MSGBOX_ICON_ERROR		= (1 << 4);
		static const unsigned int MSGBOX_ICON_QUESTION		= (2 << 4);
		static const unsigned int MSGBOX_ICON_WARNING 		= (3 << 4);
		static const unsigned int MSGBOX_ICON_INFO		= (4 << 4);
		
		// Responses
		enum MsgBox_Response
		{
			MSGBOX_RESPONSE_OK	= 0,
			MSGBOX_RESPONSE_CANCEL	= 1,
			MSGBOX_RESPONSE_YES	= 2,
			MSGBOX_RESPONSE_NO	= 3
		};
		
		// msgBox function
		static MsgBox_Response msgBox(const std::string& msg, const std::string& title = GENS_APPNAME,
						const unsigned int style = MSGBOX_ICON_INFO | MSGBOX_BUTTONS_OK,
						void* owner = NULL);
		
		// Launch browser.
		static void LaunchBrowser(const std::string& url);
		
		// Full Screen minimization.
		enum fsMinimize_Type
		{
			FSMINIMIZE_DIALOG	= 0,
			FSMINIMIZE_ALTTAB	= 1,
			
			FSMINIMIZE_MAX
		};
		
		static void fsMinimize(fsMinimize_Type fst);
		static void fsRestore(fsMinimize_Type fst);
	
	private:
		static int fsMinimize_Counter[(int)FSMINIMIZE_MAX];
		static bool fsMinimize_OldFS[(int)FSMINIMIZE_MAX];
};

#endif /* __cplusplus */

#endif /* GENS_UI_HPP */
