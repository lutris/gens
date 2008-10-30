/***************************************************************************
 * Gens: Main Menu callbacks.                                              *
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

#include "gens_menu_callbacks.hpp"
#include "gens_menu.h"

#include "emulator/g_main.hpp"
#include "gens/gens_window.hpp"
#include "gens/gens_window_sync.hpp"
#include "game_genie/game_genie_window_misc.h"
#include "controller_config/controller_config_window_misc.hpp"
#include "bios_misc_files/bios_misc_files_window_misc.hpp"
#include "directory_config/directory_config_window_misc.hpp"
#include "general_options/general_options_window_misc.hpp"
#include "about/about_window.hpp"
#include "color_adjust/color_adjust_window_misc.h"
#include "country_code/country_code_window_misc.h"

#ifdef GENS_OPENGL
#include "opengl_resolution/opengl_resolution_window_misc.h"
#endif /* GENS_OPENGL */

#ifdef GENS_CDROM
#include "select_cdrom/select_cdrom_window_misc.hpp"
#endif /* GENS_CDROM */

#include "emulator/ui_proxy.hpp"
#include "util/file/config_file.hpp"

#include "util/sound/gym.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/save.hpp"
#include "gens_core/cpu/z80/z80.h"
#include "util/gfx/imageutil.hpp"

// Sega CD
#include "emulator/g_mcd.hpp"

// 32X
#include "gens_core/cpu/sh2/sh2.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Debugger
#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

// C includes
#include <cstring>

// C++ includes
#include <string>
using std::string;

static int GensWindow_MenuItemCallback_FileMenu(uint16_t menuID, uint16_t state);
/*
static int GensWindow_MenuItemCallback_GraphicsMenu(uint16_t menuID, uint16_t state);
static int GensWindow_MenuItemCallback_CPUMenu(uint16_t menuID, uint16_t state);
static int GensWindow_MenuItemCallback_SoundMenu(uint16_t menuID, uint16_t state);
static int GensWindow_MenuItemCallback_OptionsMenu(uint16_t menuID, uint16_t state);
static int GensWindow_MenuItemCallback_HelpMenu(uint16_t menuID, uint16_t state);
*/

/**
 * GensWindow_MenuItemCallback(): Menu item callback handler.
 * @param menuID Menu ID.
 * @param state Menu state. (Used for check/radio menu items.)
 * @return Non-zero if the callback was handled; 0 if the callback wasn't handled.
 */
int GensWindow_MenuItemCallback(uint16_t menuID, uint16_t state)
{
	// Determine which menu this menu item is from.
	switch (menuID & 0xF000)
	{
		case IDM_FILE_MENU:
			return GensWindow_MenuItemCallback_FileMenu(menuID, state);
			break;
		/*
		case IDM_GRAPHICS_MENU:
			return GensWindow_MenuItemCallback_GraphicsMenu(menuID, state);
			break;
		case IDM_CPU_MENU:
			return GensWindow_MenuItemCallback_CPUMenu(menuID, state);
			break;
		case IDM_SOUND_MENU:
			return GensWindow_MenuItemCallback_SoundMenu(menuID, state);
			break;
		case IDM_OPTIONS_MENU:
			return GensWindow_MenuItemCallback_OptionsMenu(menuID, state);
			break;
		case IDM_HELP_MENU:
			return GensWindow_MenuItemCallback_HelpMenu(menuID, state);
			break;
		*/
	}
	
	// Menu item not handled.
	return 0;
}


static int GensWindow_MenuItemCallback_FileMenu(uint16_t menuID, uint16_t state)
{
	string filename;
	
	switch (menuID)
	{
		case IDM_FILE_OPENROM:
			/*
			if ((Check_If_Kaillera_Running()))
				return 0;
			*/
			if (audio->playingGYM())
				Stop_Play_GYM();
			if (ROM::getROM() != -1)
				Sync_Gens_Window();
			break;
		
#ifdef GENS_CDROM
		case IDM_FILE_BOOTCD:
			if (!ASPI_Initialized || !Num_CD_Drive)
			{
				fprintf(stderr, "%s: ASPI not initialized and/or no CD-ROM drive(s) detected.\n", __func__);
				break;
			}
			
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			if (audio->playingGYM())
				Stop_Play_GYM();
			
			ROM::freeROM(Game); // Don't forget it !
			SegaCD_Started = Init_SegaCD(NULL);
			Sync_Gens_Window();
			break;
#endif /* GENS_CDROM */
		
#ifdef GENS_OS_WIN32
		case IDM_FILE_NETPLAY:
			// Netplay isn't supported yet in Gens/GS.
			// There are only two ways this message could be received:
			// 1. Someone sent the message using another program.
			// 2. Someone used a Win32 API tool to remove the MF_GRAYED style.
			//
			// So, let's give them what they really want: A Rick Roll! :)
			Paused = 1;
			//Pause_Screen();
			audio->clearSoundBuffer();
			ShellExecute(NULL, NULL, "http://www.youtube.com/watch?v=oHg5SJYRHA0", NULL, NULL, SW_MAXIMIZE);
			break;
#endif /* GENS_OS_WIN32 */
		
		case IDM_FILE_CLOSEROM:
			if (audio->soundInitialized())
				audio->clearSoundBuffer();
	
#ifdef GENS_DEBUGGER
			Debug = 0;
#endif /* GENS_DEBUGGER */
	
			/* TODO: NetPlay
			if (Net_Play)
			{
				if (Video.Full_Screen)
					Set_Render(0, -1, 1);
			}
			*/
			
			ROM::freeROM(Game);
			Sync_Gens_Window();
			break;
		
		case IDM_FILE_GAMEGENIE:
			Open_Game_Genie();
			break;
		
		case IDM_FILE_ROMHISTORY_0:
		case IDM_FILE_ROMHISTORY_1:
		case IDM_FILE_ROMHISTORY_2:
		case IDM_FILE_ROMHISTORY_3:
		case IDM_FILE_ROMHISTORY_4:
		case IDM_FILE_ROMHISTORY_5:
		case IDM_FILE_ROMHISTORY_6:
		case IDM_FILE_ROMHISTORY_7:
		case IDM_FILE_ROMHISTORY_8:
		case IDM_FILE_ROMHISTORY_9:
			// ROM History.
			if (audio->playingGYM())
				Stop_Play_GYM();
			
			if (strlen(Recent_Rom[menuID - IDM_FILE_ROMHISTORY]) > 0)
				ROM::openROM(Recent_Rom[menuID - IDM_FILE_ROMHISTORY]);
			
			Sync_Gens_Window();
			break;
		
		case IDM_FILE_LOADSTATE:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			filename = Savestate::selectFile(false, State_Dir);
			if (!filename.empty())
				Savestate::loadState(filename.c_str());
			break;
		
		case IDM_FILE_SAVESTATE:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			filename = Savestate::selectFile(true, State_Dir);
			if (!filename.empty())
				Savestate::saveState(filename.c_str());
			break;
		
		case IDM_FILE_QUICKLOAD:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			filename = Savestate::getStateFilename();
			Savestate::loadState(filename);
			break;
		
		case IDM_FILE_QUICKSAVE:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			filename = Savestate::getStateFilename();
			Savestate::saveState(filename.c_str());
			break;
		
		case IDM_FILE_CHANGESTATE_0:
		case IDM_FILE_CHANGESTATE_1:
		case IDM_FILE_CHANGESTATE_2:
		case IDM_FILE_CHANGESTATE_3:
		case IDM_FILE_CHANGESTATE_4:
		case IDM_FILE_CHANGESTATE_5:
		case IDM_FILE_CHANGESTATE_6:
		case IDM_FILE_CHANGESTATE_7:
		case IDM_FILE_CHANGESTATE_8:
		case IDM_FILE_CHANGESTATE_9:
			// Change state.
			Set_Current_State(menuID - IDM_FILE_CHANGESTATE);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDM_FILE_EXIT:
			close_gens();
			break;
		
		default:
			// Unknown menu item ID.
			return 0;
	}
	
	// Menu item handled.
	return 1;
}
