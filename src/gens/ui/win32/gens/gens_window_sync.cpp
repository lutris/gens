/***************************************************************************
 * Gens: (GTK+) Main Window - Synchronization Functions.                   *
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
#include <windows.h>

#include "gens_window.hpp"
#include "gens_window_sync.hpp"
#include "gens_window_callbacks.hpp"

// Menu identifier definitions
#include "gens_window_menu.h"

#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "segacd/cd_sys.hpp"
#include "util/sound/gym.hpp"

// Renderer / Blitter selection stuff.
#include "gens_core/gfx/renderers.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/misc.h"

// C++ includes
#include <string>
using std::string;


/**
 * Sync_Gens_Window(): Synchronize the Gens Main Window.
 */
void Sync_Gens_Window(void)
{
	// Synchronize all menus.
	Sync_Gens_Window_FileMenu();
	Sync_Gens_Window_GraphicsMenu();
	Sync_Gens_Window_CPUMenu();
	Sync_Gens_Window_SoundMenu();
	Sync_Gens_Window_OptionsMenu();
}


/**
 * Sync_Gens_Window_FileMenu(): Synchronize the File Menu.
 */
void Sync_Gens_Window_FileMenu(void)
{
	int i;
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_STATE;
	
	// ROM Format prefixes
	// TODO: Move this somewhere else.
	const char* ROM_Format_Prefix[5] = {"[----]", "[MD]", "[32X]", "[SCD]", "[SCDX]"};
	
	// Temporary variables for ROM History.
	int romFormat;
	char ROM_Name[GENS_PATH_MAX];
	// Number of ROMs found for ROM History.
	int romsFound = 0;
	
	// ROM History submenu
	DeleteMenu(FileMenu, 3, MF_BYPOSITION);
	FileMenu_ROMHistory = CreatePopupMenu();
	InsertMenu(FileMenu, 3, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)FileMenu_ROMHistory, "ROM &History");
	
	for (i = 0; i < 9; i++)
	{
		// Make sure this Recent ROM entry actually has an entry.
		if (strlen(Recent_Rom[i]) == 0)
			continue;
		
		// Increment the ROMs Found counter.
		romsFound++;
		
		// Determine the ROM format.
		// TODO: Improve the return variable from Detect_Format()
		romFormat = detectFormat_fopen(Recent_Rom[i]) >> 1;
		if (romFormat >= 1 && romFormat <= 4)
			strcpy(ROM_Name, ROM_Format_Prefix[romFormat]);
		else
			strcpy(ROM_Name, ROM_Format_Prefix[0]);
		
		// Add a tab, a dash, and a space.
		strcat(ROM_Name, "\t- ");
		
		// Get the ROM filename.
		Get_Name_From_Path(Recent_Rom[i], Str_Tmp);
		strcat(ROM_Name, Str_Tmp);
		
		// Add the ROM item to the ROM History submenu.
		InsertMenu(FileMenu_ROMHistory, i, MF_BYPOSITION | MF_STRING, ID_FILE_ROMHISTORY + i, ROM_Name);
	}
	
	// If no recent ROMs were found, disable the ROM History menu.
	if (romsFound == 0)
	{
		miimMenuItem.fState = MFS_DISABLED;
		SetMenuItemInfo(FileMenu, 3, TRUE, &miimMenuItem);
	}
	
	// TODO: Disable Close ROM if no ROM is loaded.
	
	// Savestate menu items
	miimMenuItem.fState = ((Genesis_Started || SegaCD_Started || _32X_Started) ? MFS_ENABLED : MFS_DISABLED);
	SetMenuItemInfo(FileMenu, ID_FILE_LOADSTATE, FALSE, &miimMenuItem);
	SetMenuItemInfo(FileMenu, ID_FILE_SAVESTATE, FALSE, &miimMenuItem);
	SetMenuItemInfo(FileMenu, ID_FILE_QUICKLOAD, FALSE, &miimMenuItem);
	SetMenuItemInfo(FileMenu, ID_FILE_QUICKSAVE, FALSE, &miimMenuItem);
	
	// Current savestate
	for (i = 0; i < 10; i++)
	{
		miimMenuItem.fState = (Current_State == i ? MFS_CHECKED : MFS_UNCHECKED);
		SetMenuItemInfo(FileMenu_ChangeState, ID_FILE_CHANGESTATE + i, FALSE, &miimMenuItem);
	}
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	int i;
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_STATE;
	
	// Full Screen
	miimMenuItem.fState = (draw->fullScreen() ? MFS_CHECKED : MFS_UNCHECKED);
	SetMenuItemInfo(GraphicsMenu, ID_GRAPHICS_FULLSCREEN, FALSE, &miimMenuItem);
	
	// VSync
	if (draw->fullScreen())
		miimMenuItem.fState = (Video.VSync_FS ? MFS_CHECKED : MFS_UNCHECKED);
	else
		miimMenuItem.fState = (Video.VSync_W ? MFS_CHECKED : MFS_UNCHECKED);
	SetMenuItemInfo(GraphicsMenu, ID_GRAPHICS_VSYNC, FALSE, &miimMenuItem);
	
	// Stretch
	miimMenuItem.fState = (draw->stretch() ? MFS_CHECKED : MFS_UNCHECKED);
	SetMenuItemInfo(GraphicsMenu, ID_GRAPHICS_STRETCH, FALSE, &miimMenuItem);
	
	// Render
	Sync_Gens_Window_GraphicsMenu_Render(GraphicsMenu, 5);
	
	// Sprite Limit
	miimMenuItem.fState = (Sprite_Over ? MFS_CHECKED : MFS_UNCHECKED);
	SetMenuItemInfo(GraphicsMenu, ID_GRAPHICS_SPRITELIMIT, FALSE, &miimMenuItem);
	
	// Frame Skip
	for (i = -1; i <= 8; i++)
	{
		miimMenuItem.fState = (Frame_Skip == i ? MFS_CHECKED : MFS_UNCHECKED);
		SetMenuItemInfo(GraphicsMenu_FrameSkip, ID_GRAPHICS_FRAMESKIP + (i + 1), FALSE, &miimMenuItem);
	}
	
	// Screen Shot
	miimMenuItem.fState = ((Genesis_Started || SegaCD_Started || _32X_Started) ? MFS_ENABLED : MFS_DISABLED);
	SetMenuItemInfo(GraphicsMenu, ID_GRAPHICS_SCREENSHOT, FALSE, &miimMenuItem);
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
void Sync_Gens_Window_GraphicsMenu_Render(HMENU parent, int position)
{
	// Render submenu
	DeleteMenu(parent, position, MF_BYPOSITION);
	GraphicsMenu_Render = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)GraphicsMenu_Render, "&Render");
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
	miimMenuItem.fType = MFT_RADIOCHECK | MFT_STRING;
	
	// Create the render entries.
	int i = 0;
	bool showRenderer;
	
	while (Renderers[i].name)
	{
		// Check if the current blitter exists for this video mode.
		showRenderer = false;
		if (bpp == 32)
		{
			// 32-bit
			if (Have_MMX && Renderers[i].blit_32_mmx)
				showRenderer = true;
			else if (Renderers[i].blit_32)
				showRenderer = true;
		}
		else // if (bpp == 15 || bpp == 16)
		{
			// 15/16-bit
			if (Have_MMX && Renderers[i].blit_16_mmx)
				showRenderer = true;
			else if (Renderers[i].blit_16)
				showRenderer = true;
		}
		
		if (showRenderer)
		{
			miimMenuItem.wID = ID_GRAPHICS_RENDER + i;
			if (draw->fullScreen())
				miimMenuItem.fState = (Video.Render_FS == i ? MFS_CHECKED : MFS_UNCHECKED);
			else
				miimMenuItem.fState = (Video.Render_W == i ? MFS_CHECKED : MFS_UNCHECKED);
			
			miimMenuItem.dwTypeData = Renderers[i].name;
			miimMenuItem.cch = strlen(Renderers[i].name);
			InsertMenuItem(GraphicsMenu_Render, i, TRUE, &miimMenuItem);
		}
		
		// Check the next renderer.
		i++;
	}
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
#ifdef GENS_DEBUGGER
	// Synchronize the Debug submenu.
	Sync_Gens_Window_CPUMenu_Debug(CPUMenu, 0);
#endif /* GENS_DEBUGGER */
	
	// Hide and show appropriate RESET items.
	RemoveMenu(CPUMenu, ID_CPU_RESET68K, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, ID_CPU_RESETMAIN68K, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, ID_CPU_RESETSUB68K, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, ID_CPU_RESETMAINSH2, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, ID_CPU_RESETSUBSH2, MF_BYCOMMAND);
	
	if (SegaCD_Started)
	{
		// SegaCD: Show Main 68000 and Sub 68000.
		InsertMenu(CPUMenu, 6, flags, ID_CPU_RESETMAIN68K, "Reset Main 68000");
		InsertMenu(CPUMenu, 7, flags, ID_CPU_RESETSUB68K, "Reset Sub 68000");
	}
	else
	{
		// No SegaCD: Only show one 68000.
		InsertMenu(CPUMenu, 5, flags, ID_CPU_RESET68K, "Reset 68000");
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		InsertMenu(CPUMenu, 8, flags, ID_CPU_RESETMAINSH2, "Reset Main SH2");
		InsertMenu(CPUMenu, 9, flags, ID_CPU_RESETSUBSH2, "Reset Sub SH2");
	}
	
	// Country code
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_STATE;
	
	int i;
	for (i = -1; i < 4; i++)
	{
		miimMenuItem.fState = (i == Country ? MFS_CHECKED : MFS_UNCHECKED);
		SetMenuItemInfo(CPUMenu_Country, ID_CPU_COUNTRY + (i + 1), FALSE, &miimMenuItem);
	}
	
	// SegaCD Perfect Sync
	miimMenuItem.fState = (SegaCD_Accurate ? MFS_CHECKED : MFS_UNCHECKED);
	SetMenuItemInfo(CPUMenu, ID_CPU_SEGACDPERFECTSYNC, FALSE, &miimMenuItem);
}


#ifdef GENS_DEBUGGER
/**
 * Sync_Gens_Window_CPUMenu_Debug(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
void Sync_Gens_Window_CPUMenu_Debug(HMENU parent, int position)
{
	// Debug submenu
	unsigned int flags = MF_BYPOSITION | MF_POPUP | MF_STRING;
	if (!(Genesis_Started || SegaCD_Started || _32X_Started))
		flags |= MF_GRAYED;
	
	DeleteMenu(parent, position, MF_BYPOSITION);
	CPUMenu_Debug = CreatePopupMenu();
	InsertMenu(parent, position, flags, (UINT_PTR)CPUMenu_Debug, "&Debug");
	
	if (flags & MF_GRAYED)
		return;
	
	// TODO: Move this array somewhere else.
	const char* DebugStr[9] =
	{
		"&Genesis - 68000",
		"Genesis - &Z80",
		"Genesis - &VDP",
		"&SegaCD - 68000",
		"SegaCD - &CDC",
		"SegaCD - GF&X",
		"32X - Main SH2",
		"32X - Sub SH2",
		"32X - VDP",
	};
	
	int i;
	
	// Create the debug entries.
	for (i = 0; i < 9; i++)
	{
		if ((i >= 0 && i <= 2) ||
		    (i >= 3 && i <= 5 && SegaCD_Started) ||
		    (i >= 6 && i <= 8 && _32X_Started))
		{
			if (i % 3 == 0 && (i >= 3 && i <= 6))
			{
				// Every three entires, add a separator.
				InsertMenu(CPUMenu_Debug, i + 1, MF_SEPARATOR, NULL, NULL);
			}
			
			InsertMenu(CPUMenu_Debug, i + (i / 3), MF_BYPOSITION | MF_STRING, ID_CPU_DEBUG + i, DebugStr[i]);
		}
	}
}
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
	int i;
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_STATE;
	
	// Get the Enabled flag for the other menu items.
	bool soundEnabled = audio->enabled();
	unsigned int flags = (soundEnabled ? MFS_ENABLED : MFS_DISABLED);
	
	// Enabled
	miimMenuItem.fState = (soundEnabled ? MFS_CHECKED : MFS_UNCHECKED);
	SetMenuItemInfo(SoundMenu, ID_SOUND_ENABLE, FALSE, &miimMenuItem);
	
	const int soundMenuItems[11][2] =
	{
		{audio->stereo(), ID_SOUND_STEREO},
		{Z80_State & 1, ID_SOUND_Z80},
		{YM2612_Enable, ID_SOUND_YM2612},
		{YM2612_Improv, ID_SOUND_YM2612_IMPROVED},
		{DAC_Enable, ID_SOUND_DAC},
		{DAC_Improv, ID_SOUND_DAC_IMPROVED},
		{PSG_Enable, ID_SOUND_PSG},
		{PSG_Improv, ID_SOUND_PSG_IMPROVED},
		{PCM_Enable, ID_SOUND_PCM},
		{PWM_Enable, ID_SOUND_PWM},
		{CDDA_Enable, ID_SOUND_CDDA},
	};
	
	for (i = 0; i < 11; i++)
	{
		miimMenuItem.fState = flags | (soundMenuItems[i][0] ? MFS_CHECKED : MFS_UNCHECKED);
		SetMenuItemInfo(SoundMenu, soundMenuItems[i][1], FALSE, &miimMenuItem);
	}
	
	// Rate
	// TODO: This const array is from gens_window.c.
	// Move it somewhere else.
	const int SndRates[6][2] =
	{
		{0, 11025}, {3, 16000}, {1, 22050},
		{4, 32000}, {2, 44100}, {5, 48000},
	};
	
	for (i = 0; i < 6; i++)
	{
		miimMenuItem.fState = (audio->soundRate() == SndRates[i][1] ? MFS_CHECKED : MFS_UNCHECKED);
		SetMenuItemInfo(SoundMenu, ID_SOUND_RATE + SndRates[i][0], FALSE, &miimMenuItem);
	}
	
	char dumpLabel[16];
	miimMenuItem.fMask = MIIM_STATE | MIIM_STRING;
	//miimMenuItem.fState = ((Genesis_Started || SegaCD_Started || _32X_Started) ? MFS_ENABLED : MFS_DISABLED);
	
	// WAV dumping
	// TODO: Always disabled for now, since WAV dumping isn't implemented yet.
	strcpy(dumpLabel, (audio->dumpingWAV() ? "Stop WAV Dump" : "Start WAV Dump"));
	miimMenuItem.fState = MFS_DISABLED;
	miimMenuItem.dwTypeData = dumpLabel;
	miimMenuItem.cch = strlen(dumpLabel);
	SetMenuItemInfo(SoundMenu, ID_SOUND_WAVDUMP, FALSE, &miimMenuItem);
	
	// GYM dumping
	strcpy(dumpLabel, (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump"));
	miimMenuItem.fState = ((Genesis_Started || SegaCD_Started || _32X_Started) ? MFS_ENABLED : MFS_DISABLED);
	miimMenuItem.dwTypeData = dumpLabel;
	miimMenuItem.cch = strlen(dumpLabel);
	SetMenuItemInfo(SoundMenu, ID_SOUND_GYMDUMP, FALSE, &miimMenuItem);
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
	// SegaCD SRAM Size
	int SRAM_ID = (BRAM_Ex_State & 0x100 ? BRAM_Ex_Size : -1);
	int i;
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_STATE;
	
	for (i = -1; i <= 3; i++)
	{
		miimMenuItem.fState = (i == SRAM_ID ? MFS_CHECKED : MFS_UNCHECKED);
		SetMenuItemInfo(OptionsMenu_SegaCDSRAMSize, ID_OPTIONS_SEGACDSRAMSIZE + (i + 1), FALSE, &miimMenuItem);
	}
}
