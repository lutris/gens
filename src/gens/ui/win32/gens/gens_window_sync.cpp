/***************************************************************************
 * Gens: (Win32) Main Window - Synchronization Functions.                  *
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

#include <string.h>

#include "gens_window_sync.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_menu.hpp"

// Common UI functions.
#include "ui/common/gens/gens_menu.h"
#include "ui/common/gens/gens_window_sync.h"

#include "emulator/g_main.hpp"
#include "emulator/options.hpp"
#include "segacd/cd_sys.hpp"
#include "util/file/rom.hpp"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"

#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// Renderer / Blitter selection stuff.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/cpuflags.h"

// Plugin Manager and Render Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// File management functions.
#include "util/file/file.hpp"

// Video, Audio.
#include "video/vdraw.h"
#include "audio/audio.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// C++ includes
#include <string>
#include <list>
#include <deque>
using std::string;
using std::list;
using std::deque;


// Internal functions.
static void Sync_Gens_Window_GraphicsMenu_Backend(HMENU parent, int position);
static void Sync_Gens_Window_GraphicsMenu_Render(HMENU parent, int position);
#ifdef GENS_DEBUGGER
static void Sync_Gens_Window_CPUMenu_Debug(HMENU parent, int position);
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window(): Synchronize the Gens Main Window.
 */
void Sync_Gens_Window(void)
{
	// Synchronize all menus.
	Sync_Gens_Window_FileMenu();
	Sync_Gens_Window_FileMenu_ROMHistory();
	Sync_Gens_Window_GraphicsMenu();
	Sync_Gens_Window_CPUMenu();
	Sync_Gens_Window_SoundMenu();
	Sync_Gens_Window_OptionsMenu();
	Sync_Gens_Window_PluginsMenu();
}


/**
 * Sync_Gens_Window_FileMenu(): Synchronize the File Menu.
 * This does NOT synchronize the ROM History submenu.
 */
void Sync_Gens_Window_FileMenu(void)
{
	// Find the file menu.
	HMENU mnuFile = gens_menu_find_item(IDM_FILE_MENU);
	
        // Netplay is currently not usable.
	EnableMenuItem(mnuFile, IDM_FILE_NETPLAY, MF_BYCOMMAND | MF_GRAYED);
	
	// Some menu items should be enabled or disabled, depending on if a game is loaded or not.
	const unsigned int enableFlags = ((Game != NULL) ? MF_ENABLED : MF_GRAYED);
	
	// Disable "Close ROM" if no ROM is loaded.
	EnableMenuItem(mnuFile, IDM_FILE_CLOSEROM, MF_BYCOMMAND | enableFlags);
	
	// Savestate menu items
	EnableMenuItem(mnuFile, IDM_FILE_LOADSTATE, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(mnuFile, IDM_FILE_SAVESTATE, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(mnuFile, IDM_FILE_QUICKLOAD, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(mnuFile, IDM_FILE_QUICKSAVE, MF_BYCOMMAND | enableFlags);
	
	// Current savestate
	HMENU mnuChangeState = gens_menu_find_item(IDM_FILE_CHANGESTATE);
	CheckMenuRadioItem(mnuChangeState,
			   IDM_FILE_CHANGESTATE_0,
			   IDM_FILE_CHANGESTATE_9,
			   IDM_FILE_CHANGESTATE_0 + Current_State,
			   MF_BYCOMMAND);
}


/**
 * Sync_Gens_Window_FileMenu_ROMHistory(): Synchronize the File, ROM History submenu.
 */
void Sync_Gens_Window_FileMenu_ROMHistory(void)
{
	// Find the file menu.
	HMENU mnuFile = gens_menu_find_item(IDM_FILE_MENU);
	
	// Find the ROM History submenu.
	HMENU mnuROMHistory = gens_menu_find_item(IDM_FILE_ROMHISTORY);
	
	// Delete and/or recreate the ROM History submenu.
#ifdef GENS_CDROM
	static const unsigned int posROMHistory = 3;
#else /* !GENS_CDROM */
	static const unsigned int posROMHistory = 2;
#endif /* GENS_CDROM */
	
	DeleteMenu(mnuFile, (UINT)mnuROMHistory, MF_BYCOMMAND);
	DeleteMenu(mnuFile, IDM_FILE_ROMHISTORY, MF_BYCOMMAND);
	gens_menu_map.erase(IDM_FILE_ROMHISTORY);
	if (mnuROMHistory)
		DestroyMenu(mnuROMHistory);
	
	mnuROMHistory = CreatePopupMenu();
	InsertMenu(mnuFile, posROMHistory, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)mnuROMHistory, TEXT("ROM &History"));
	gens_menu_map.insert(gensMenuMapItem_t(IDM_FILE_ROMHISTORY, mnuROMHistory));
	
	string sROMHistoryEntry;
	int romFormat;
	int romNum = 0;
	
	for (deque<ROM::Recent_ROM_t>::iterator rom = ROM::Recent_ROMs.begin();
	     rom != ROM::Recent_ROMs.end(); rom++)
	{
		// Determine the ROM format.
		// TODO: Improve the return variable from Detect_Format()
		romFormat = ((*rom).type & ROMTYPE_SYS_MASK);
		if (romFormat >= ROMTYPE_SYS_MD && romFormat <= ROMTYPE_SYS_MCD32X)
			sROMHistoryEntry = gws_rom_format_prefix[romFormat];
		else
			sROMHistoryEntry = gws_rom_format_prefix[0];
		
		// Add a tab, a dash, and a space.
		sROMHistoryEntry += "\t- ";
		
		// Get the ROM filename.
		sROMHistoryEntry += File::GetNameFromPath((*rom).filename);
		
		if (!(*rom).z_filename.empty())
		{
			// ROM has a compressed filename.
			sROMHistoryEntry += " [" + (*rom).z_filename + "]";
		}
		
		// Add the ROM item to the ROM History submenu.
		InsertMenu(mnuROMHistory, -1, MF_BYPOSITION | MF_STRING,
			   IDM_FILE_ROMHISTORY_1 + romNum, sROMHistoryEntry.c_str());
		
		// Increment the ROM number.
		romNum++;
	}
	
	// If no recent ROMs were found, disable the ROM History menu.
	if (romNum == 0)
		EnableMenuItem(mnuFile, (UINT)mnuROMHistory, MF_BYCOMMAND | MF_GRAYED);
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	HMENU mnuGraphics = gens_menu_find_item(IDM_GRAPHICS_MENU);
	
	unsigned int flags;
	
	// Enable Full Screen, VSync, and Stretch only if the current backend support them.
	flags = (vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_FULLSCREEN ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(mnuGraphics, IDM_GRAPHICS_FULLSCREEN, MF_BYCOMMAND | flags);
	flags = (vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_VSYNC ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(mnuGraphics, IDM_GRAPHICS_VSYNC, MF_BYCOMMAND | flags);
	flags = (vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_STRETCH ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(mnuGraphics, (UINT)(gens_menu_find_item(IDM_GRAPHICS_STRETCH)), MF_BYCOMMAND | flags);
	
	// Full Screen
	const bool fullScreen = vdraw_get_fullscreen();
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_FULLSCREEN,
		      MF_BYCOMMAND | (fullScreen ? MF_CHECKED : MF_UNCHECKED));
	
	// Show Menu Bar
	EnableMenuItem(mnuGraphics, IDM_GRAPHICS_MENUBAR,
		       MF_BYCOMMAND | (fullScreen ? MF_DISABLED : MF_ENABLED));
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_MENUBAR,
		      MF_BYCOMMAND | (Settings.showMenuBar ? MF_CHECKED : MF_UNCHECKED));
	
	// VSync
	if (fullScreen)
		flags = (Video.VSync_FS ? MF_CHECKED : MF_UNCHECKED);
	else
		flags = (Video.VSync_W ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_VSYNC, MF_BYCOMMAND | flags);
	
	// Stretch
	HMENU mnuStretch = gens_menu_find_item(IDM_GRAPHICS_STRETCH);
	CheckMenuRadioItem(mnuStretch,
			   IDM_GRAPHICS_STRETCH_NONE,
			   IDM_GRAPHICS_STRETCH_FULL,
			   IDM_GRAPHICS_STRETCH_NONE + Options::stretch(),
			   MF_BYCOMMAND);
	
	// Backend
	Sync_Gens_Window_GraphicsMenu_Backend(mnuGraphics, 5);
	
	// Render
	Sync_Gens_Window_GraphicsMenu_Render(mnuGraphics, 8);
	
	// Sprite Limit
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_SPRITELIMIT,
		      MF_BYCOMMAND | (Sprite_Over ? MF_CHECKED : MF_UNCHECKED));
	
	// Frame Skip
	HMENU mnuFrameSkip = gens_menu_find_item(IDM_GRAPHICS_FRAMESKIP);
	CheckMenuRadioItem(mnuFrameSkip,
			   IDM_GRAPHICS_FRAMESKIP_AUTO,
			   IDM_GRAPHICS_FRAMESKIP_8,
			   IDM_GRAPHICS_FRAMESKIP_AUTO + (Frame_Skip + 1),
			   MF_BYCOMMAND);
	
	// Screenshot
	EnableMenuItem(mnuGraphics, IDM_GRAPHICS_SCREENSHOT,
		      MF_BYCOMMAND | ((Game != NULL) ? MF_ENABLED : MF_GRAYED));
}


/**
 * Sync_Gens_Window_GraphicsMenu_Backend(): Synchronize the Graphics, Backend submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void Sync_Gens_Window_GraphicsMenu_Backend(HMENU parent, int position)
{
	// Find the Backend submenu.
	HMENU mnuBackend = gens_menu_find_item(IDM_GRAPHICS_BACKEND);
	
	// Delete and/or recreate the Backend submenu.
	DeleteMenu(parent, (UINT)mnuBackend, MF_BYCOMMAND);
	DeleteMenu(parent, IDM_GRAPHICS_BACKEND, MF_BYCOMMAND);
	gens_menu_map.erase(IDM_GRAPHICS_BACKEND);
	if (mnuBackend)
		DestroyMenu(mnuBackend);
	
	// Create a new submenu.
	mnuBackend = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)mnuBackend, TEXT("&Backend\tShift+R"));
	gens_menu_map.insert(gensMenuMapItem_t(IDM_GRAPHICS_BACKEND, mnuBackend));
	
	// Add the backends.
	int curBackend = 0;
	while (vdraw_backends[curBackend])
	{
		InsertMenu(mnuBackend, -1, MF_BYPOSITION | MF_STRING,
			   IDM_GRAPHICS_BACKEND + 1 + curBackend, vdraw_backends[curBackend]->name);
		
		// Check if this backend is broken.
		EnableMenuItem(mnuBackend, IDM_GRAPHICS_BACKEND + 1 + curBackend,
			       MF_BYCOMMAND | (vdraw_backends_broken[curBackend] ? MF_GRAYED : MF_ENABLED));
		
		// Next backend.
		curBackend++;
	}
	
	// Set the radio button indicator on the current backend.
	CheckMenuRadioItem(mnuBackend,
			   IDM_GRAPHICS_BACKEND + 1,
			   IDM_GRAPHICS_BACKEND + curBackend,
			   IDM_GRAPHICS_BACKEND + 1 + vdraw_cur_backend_id,
			   MF_BYCOMMAND);
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void Sync_Gens_Window_GraphicsMenu_Render(HMENU parent, int position)
{
	// Find the Render submenu.
	HMENU mnuRender = gens_menu_find_item(IDM_GRAPHICS_RENDER);
	
	// Delete and/or recreate the Render submenu.
	DeleteMenu(parent, (UINT)mnuRender, MF_BYCOMMAND);
	DeleteMenu(parent, IDM_GRAPHICS_RENDER, MF_BYCOMMAND);
	gens_menu_map.erase(IDM_GRAPHICS_RENDER);
	if (mnuRender)
		DestroyMenu(mnuRender);
	
	// Render submenu
	mnuRender = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)mnuRender, TEXT("&Render"));
	gens_menu_map.insert(gensMenuMapItem_t(IDM_GRAPHICS_RENDER, mnuRender));
	
	// Create the render entries.
	unsigned int i = IDM_GRAPHICS_RENDER_NORMAL;
	unsigned int selMenuItem = 0;
	list<mdp_render_t*>::iterator& selMDP = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	
	for (list<mdp_render_t*>::iterator curPlugin = RenderMgr::begin();
	     curPlugin != RenderMgr::end(); curPlugin++, i++)
	{
		InsertMenu(mnuRender, -1, MF_BYPOSITION | MF_STRING, i, (*curPlugin)->tag);
		
		// Check if this render mode is selected.
		if (selMDP == curPlugin)
			selMenuItem = i;
		
		// Insert a separator between the built-in renderers
		// and the external renderers if external renderers exist.
		if (i == IDM_GRAPHICS_RENDER_NORMAL + 1)
		{
			if (RenderMgr::size() > 2)
				InsertMenu(mnuRender, -1, MF_BYPOSITION | MF_SEPARATOR, IDM_SEPARATOR, NULL);
		}
	}
	
	if (selMenuItem != 0)
	{
		// Found a selected render mode.
		CheckMenuRadioItem(mnuRender,
				   IDM_GRAPHICS_RENDER_NORMAL,
				   IDM_GRAPHICS_RENDER_NORMAL + RenderMgr::size() - 1,
				   selMenuItem, MF_BYCOMMAND);
	}
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
	// TODO: Figure out how to hide menu items instead of deleting/recreating them.
	
	static const unsigned int flags = MF_BYCOMMAND | MF_STRING;
	
	HMENU mnuCPU = gens_menu_find_item(IDM_CPU_MENU);
	
#ifdef GENS_DEBUGGER
	// Synchronize the Debug submenu.
	Sync_Gens_Window_CPUMenu_Debug(mnuCPU, 0);
#endif /* GENS_DEBUGGER */
	
	// Hide and show appropriate RESET items.
	RemoveMenu(mnuCPU, IDM_CPU_RESET68K, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETMAIN68K, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETSUB68K, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETMAINSH2, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETSUBSH2, MF_BYCOMMAND);
	
	if (SegaCD_Started)
	{
		// SegaCD: Show Main 68000 and Sub 68000.
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETMAIN68K, TEXT("Reset Main 68000"));
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETSUB68K, TEXT("Reset Sub 68000"));
	}
	else
	{
		// No SegaCD: Only show one 68000.
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESET68K, TEXT("Reset 68000"));
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETMAINSH2, TEXT("Reset Main SH2"));
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETSUBSH2, TEXT("Reset Sub SH2"));
	}
	
	// Country code
	HMENU mnuCountry = gens_menu_find_item(IDM_CPU_COUNTRY);
	CheckMenuRadioItem(mnuCountry,
			   IDM_CPU_COUNTRY_AUTO,
			   IDM_CPU_COUNTRY_JAPAN_PAL,
			   IDM_CPU_COUNTRY_AUTO + (Country + 1),
			   MF_BYCOMMAND);
	
	// SegaCD Perfect Sync
	CheckMenuItem(mnuCPU, IDM_CPU_SEGACDPERFECTSYNC,
		      MF_BYCOMMAND | (SegaCD_Accurate ? MF_CHECKED : MF_UNCHECKED));
}


#ifdef GENS_DEBUGGER
/**
 * Sync_Gens_Window_CPUMenu_Debug(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void Sync_Gens_Window_CPUMenu_Debug(HMENU parent, int position)
{
	// Debug submenu
	unsigned int flags = MF_BYPOSITION | MF_POPUP | MF_STRING;
	if (Game == NULL)
		flags |= MF_GRAYED;
	
	HMENU mnuDebug = gens_menu_find_item(IDM_CPU_DEBUG);
	
	// Delete and/or recreate the Debug submenu.
	DeleteMenu(parent, (UINT)mnuDebug, MF_BYCOMMAND);
	gens_menu_map.erase(IDM_CPU_DEBUG);
	if (mnuDebug)
		DestroyMenu(mnuDebug);
	
	// Debug submenu
	mnuDebug = CreatePopupMenu();
	InsertMenu(parent, position, flags, (UINT_PTR)mnuDebug, TEXT("&Debug"));
	gens_menu_map.insert(gensMenuMapItem_t(IDM_CPU_DEBUG, mnuDebug));
	
	if (flags & MF_GRAYED)
		return;
	
	// Create the debug entries.
	for (int i = 0; i < 9; i++)
	{
		if ((i >= 0 && i <= 2) ||
		    (i >= 3 && i <= 5 && SegaCD_Started) ||
		    (i >= 6 && i <= 8 && _32X_Started))
		{
			if (i % 3 == 0 && (i >= 3 && i <= 6))
			{
				// Every three entires, add a separator.
				InsertMenu(mnuDebug, i + 1, MF_SEPARATOR,
					   IDM_CPU_DEBUG_SEGACD_SEPARATOR + ((i / 3) - 1), NULL);
			}
			
			InsertMenu(mnuDebug, i + (i / 3), MF_BYPOSITION | MF_STRING,
				   IDM_CPU_DEBUG_MC68000 + i, gws_debug_items[i]);
			
			if ((int)debug_mode == (i + 1))
				CheckMenuItem(mnuDebug, IDM_CPU_DEBUG_MC68000 + i, MF_BYCOMMAND | MF_CHECKED);
		}
	}
}
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
	HMENU mnuSound = gens_menu_find_item(IDM_SOUND_MENU);
	
	// Get the Enabled flag for the other menu items.
	bool soundEnabled = audio_get_enabled();
	
	// Enabled
	CheckMenuItem(mnuSound, IDM_SOUND_ENABLE,
		      MF_BYCOMMAND | (soundEnabled ? MF_CHECKED : MF_UNCHECKED));
	
	const uint16_t soundMenuItems[10][2] =
	{
		{IDM_SOUND_STEREO,		audio_get_stereo()},
		{IDM_SOUND_Z80,			Z80_State & Z80_STATE_ENABLED},
		{IDM_SOUND_YM2612,		YM2612_Enable},
		{IDM_SOUND_YM2612_IMPROVED,	YM2612_Improv},
		{IDM_SOUND_DAC,			DAC_Enable},
		{IDM_SOUND_PSG,			PSG_Enable},
		{IDM_SOUND_PSG_SINE,		PSG_Improv},
		{IDM_SOUND_PCM,			PCM_Enable},
		{IDM_SOUND_PWM,			PWM_Enable},
		{IDM_SOUND_CDDA,		CDDA_Enable},
	};
	
	for (int i = 0; i < 10; i++)
	{
		EnableMenuItem(mnuSound, soundMenuItems[i][0],
			       MF_BYCOMMAND | (soundEnabled ? MF_ENABLED : MF_GRAYED));
		
		CheckMenuItem(mnuSound, soundMenuItems[i][0],
			      MF_BYCOMMAND | (soundMenuItems[i][1] ? MF_CHECKED : MF_UNCHECKED));
	}
	
	// Rate
	// TODO: This const array is from gens_window.c.
	// Move it somewhere else.
	const int SndRates[6][2] = {{0, 11025}, {1, 22050}, {2, 44100}};
	
	HMENU mnuRate = gens_menu_find_item(IDM_SOUND_RATE);
	for (int i = 0; i < 3; i++)
	{
		if (SndRates[i][1] == audio_get_sound_rate())
		{
			CheckMenuRadioItem(mnuRate,
					   IDM_SOUND_RATE_11025,
					   IDM_SOUND_RATE_44100,
					   IDM_SOUND_RATE_11025 + SndRates[i][0],
					   MF_BYCOMMAND);
			break;
		}
	}
	
	const TCHAR *dumpLabel;
	
	uint32_t audioDumpFlags = MF_BYCOMMAND | (((Game != NULL) && audio_get_enabled()) ? MF_ENABLED : MF_GRAYED);
	
	// WAV dumping.
	dumpLabel = (WAV_Dumping ? TEXT("Stop WAV Dump") : TEXT("Start WAV Dump"));
	ModifyMenu(mnuSound, IDM_SOUND_WAVDUMP,
		   MF_BYCOMMAND | MF_STRING,
		   IDM_SOUND_WAVDUMP, dumpLabel);
	EnableMenuItem(mnuSound, IDM_SOUND_WAVDUMP, audioDumpFlags);
	
	// GYM dumping.
	dumpLabel = (GYM_Dumping ? TEXT("Stop GYM Dump") : TEXT("Start GYM Dump"));
	ModifyMenu(mnuSound, IDM_SOUND_GYMDUMP,
		   MF_BYCOMMAND | MF_STRING,
		   IDM_SOUND_GYMDUMP, dumpLabel);
	EnableMenuItem(mnuSound, IDM_SOUND_GYMDUMP, audioDumpFlags);
}


/**
 * Sync_Gens_Window_PluginsMenu(): Synchronize the Plugins menu.
 */
void Sync_Gens_Window_PluginsMenu(void)
{
	// Find the Plugins menu.
	HMENU mnuPlugins = gens_menu_find_item(IDM_PLUGINS_MENU);
	
	// Delete and/or recreate the Plugins menu.
	static const unsigned int posPlugins = 5;
	
	DeleteMenu(MainMenu, (UINT)mnuPlugins, MF_BYCOMMAND);
	gens_menu_map.erase(IDM_PLUGINS_MENU);
	if (mnuPlugins)
		DestroyMenu(mnuPlugins);
	
	mnuPlugins = CreatePopupMenu();
	InsertMenu(MainMenu, posPlugins, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)mnuPlugins, TEXT("&Plugins"));
	gens_menu_map.insert(gensMenuMapItem_t(IDM_PLUGINS_MENU, mnuPlugins));
	
	// Add the Plugin Manager menu item.
	InsertMenu(mnuPlugins, -1, MF_BYPOSITION | MF_STRING,
		   IDM_PLUGINS_MANAGER, TEXT("Plugin Manager"));
	
	if (PluginMgr::lstMenuItems.size() != 0)
	{
		// Plugin menu items were created.
		
		// Add the Plugin Manager separator.
		InsertMenu(mnuPlugins, -1, MF_BYPOSITION | MF_SEPARATOR,
			   IDM_SEPARATOR, NULL);
		
		// Create the plugin menu items.
		for (list<mdpMenuItem_t>::iterator curMenuItem = PluginMgr::lstMenuItems.begin();
		     curMenuItem != PluginMgr::lstMenuItems.end(); curMenuItem++)
		{
			InsertMenu(mnuPlugins, -1, MF_BYPOSITION | MF_STRING,
					(*curMenuItem).id, (*curMenuItem).text.c_str());
			
			// Set the menu item check state.
			CheckMenuItem(mnuPlugins, (*curMenuItem).id,
					MF_BYCOMMAND | ((*curMenuItem).checked ? MF_CHECKED : MF_UNCHECKED));
		}
	}
}


/**
 * Sync_Gens_Window_OptionsMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
	HMENU mnuOptions = gens_menu_find_item(IDM_OPTIONS_MENU);
	
	// SRAM Enabled
	CheckMenuItem(mnuOptions, IDM_OPTIONS_SRAM_ENABLED,
		      MF_BYCOMMAND | (SRAM_Enabled ? MF_CHECKED : MF_UNCHECKED));
	
	// SegaCD SRAM Size
	int SRAM_ID = (BRAM_Ex_State & 0x100 ? BRAM_Ex_Size : -1);
	CheckMenuRadioItem(mnuOptions,
			   IDM_OPTIONS_SEGACDSRAMSIZE_NONE,
			   IDM_OPTIONS_SEGACDSRAMSIZE_64KB,
			   IDM_OPTIONS_SEGACDSRAMSIZE_NONE + (SRAM_ID + 1),
			   MF_BYCOMMAND);
}
