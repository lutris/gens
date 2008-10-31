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

// New menu handler.
#include "ui/common/gens/gens_menu.h"

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
#if 0
	int i;
	
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
		romFormat = ROM::detectFormat_fopen(Recent_Rom[i]) >> 1;
		if (romFormat >= 1 && romFormat <= 4)
			strcpy(ROM_Name, ROM_Format_Prefix[romFormat]);
		else
			strcpy(ROM_Name, ROM_Format_Prefix[0]);
		
		// Add a tab, a dash, and a space.
		strcat(ROM_Name, "\t- ");
		
		// Get the ROM filename.
		ROM::getNameFromPath(Recent_Rom[i], Str_Tmp);
		strcat(ROM_Name, Str_Tmp);
		
		// Add the ROM item to the ROM History submenu.
		InsertMenu(FileMenu_ROMHistory, i, MF_BYPOSITION | MF_STRING, IDM_FILE_ROMHISTORY + i, ROM_Name);
	}
	
	// If no recent ROMs were found, disable the ROM History menu.
	if (romsFound == 0)
		EnableMenuItem(FileMenu, 3, MF_BYPOSITION | MF_GRAYED);
	
	// TODO: Disable Close ROM if no ROM is loaded.
	
	// Savestate menu items
	unsigned int enableFlags = ((Genesis_Started || SegaCD_Started || _32X_Started) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(FileMenu, IDM_FILE_LOADSTATE, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(FileMenu, IDM_FILE_SAVESTATE, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(FileMenu, IDM_FILE_QUICKLOAD, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(FileMenu, IDM_FILE_QUICKSAVE, MF_BYCOMMAND | enableFlags);
	
	// Current savestate
	CheckMenuRadioItem(FileMenu_ChangeState,
			   IDM_FILE_CHANGESTATE_0,
			   IDM_FILE_CHANGESTATE_9,
			   IDM_FILE_CHANGESTATE + Current_State,
			   MF_BYCOMMAND);
#endif
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
#if 0
	// Full Screen
	CheckMenuItem(GraphicsMenu, IDM_GRAPHICS_FULLSCREEN,
		      MF_BYCOMMAND | (draw->fullScreen() ? MF_CHECKED : MF_UNCHECKED));
	
	// VSync
	unsigned int checkFlags;
	if (draw->fullScreen())
		checkFlags = (Video.VSync_FS ? MF_CHECKED : MF_UNCHECKED);
	else
		checkFlags = (Video.VSync_W ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(GraphicsMenu, IDM_GRAPHICS_VSYNC, MF_BYCOMMAND | checkFlags);
	
	// Stretch
	CheckMenuItem(GraphicsMenu, IDM_GRAPHICS_STRETCH,
		      MF_BYCOMMAND | (draw->stretch() ? MF_CHECKED : MF_UNCHECKED));
	
	// Render
	Sync_Gens_Window_GraphicsMenu_Render(GraphicsMenu, 5);
	
	// Sprite Limit
	CheckMenuItem(GraphicsMenu, IDM_GRAPHICS_SPRITELIMIT,
		      MF_BYCOMMAND | (Sprite_Over ? MF_CHECKED : MF_UNCHECKED));
	
	// Frame Skip
	CheckMenuRadioItem(GraphicsMenu_FrameSkip,
			   IDM_GRAPHICS_FRAMESKIP_AUTO,
			   IDM_GRAPHICS_FRAMESKIP_8,
			   IDM_GRAPHICS_FRAMESKIP + (Frame_Skip + 1),
			   MF_BYCOMMAND);
	
	// Screen Shot
	CheckMenuItem(GraphicsMenu, IDM_GRAPHICS_SCREENSHOT,
		      MF_BYCOMMAND | ((Genesis_Started || SegaCD_Started || _32X_Started) ? MF_CHECKED : MF_UNCHECKED));
#endif
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
void Sync_Gens_Window_GraphicsMenu_Render(HMENU parent, int position)
{
#if 0
	// Render submenu
	DeleteMenu(parent, position, MF_BYPOSITION);
	GraphicsMenu_Render = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)GraphicsMenu_Render, "&Render");
	
	// Create the render entries.
	bool renderSelected = false;
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
			if (draw->fullScreen())
				renderSelected = (Video.Render_FS == i);
			else
				renderSelected = (Video.Render_W == i);
			
			InsertMenu(GraphicsMenu_Render, i, MF_BYPOSITION | MF_STRING,
				   IDM_GRAPHICS_RENDER + i, Renderers[i].name);
			
			if (renderSelected)
			{
				CheckMenuRadioItem(GraphicsMenu_Render,
						   IDM_GRAPHICS_RENDER,
						   IDM_GRAPHICS_RENDER + (Renderers_Count - 1),
						   IDM_GRAPHICS_RENDER + i,
						   MF_BYCOMMAND);
			}
		}
		
		// Check the next renderer.
		i++;
	}
#endif
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
#if 0
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
#ifdef GENS_DEBUGGER
	// Synchronize the Debug submenu.
	Sync_Gens_Window_CPUMenu_Debug(CPUMenu, 0);
#endif /* GENS_DEBUGGER */
	
	// Hide and show appropriate RESET items.
	RemoveMenu(CPUMenu, IDM_CPU_RESET68K, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, IDM_CPU_RESETMAIN68K, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, IDM_CPU_RESETSUB68K, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, IDM_CPU_RESETMAINSH2, MF_BYCOMMAND);
	RemoveMenu(CPUMenu, IDM_CPU_RESETSUBSH2, MF_BYCOMMAND);
	
	if (SegaCD_Started)
	{
		// SegaCD: Show Main 68000 and Sub 68000.
		InsertMenu(CPUMenu, 6, flags, IDM_CPU_RESETMAIN68K, "Reset Main 68000");
		InsertMenu(CPUMenu, 7, flags, IDM_CPU_RESETSUB68K, "Reset Sub 68000");
	}
	else
	{
		// No SegaCD: Only show one 68000.
		InsertMenu(CPUMenu, 5, flags, IDM_CPU_RESET68K, "Reset 68000");
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		InsertMenu(CPUMenu, 8, flags, IDM_CPU_RESETMAINSH2, "Reset Main SH2");
		InsertMenu(CPUMenu, 9, flags, IDM_CPU_RESETSUBSH2, "Reset Sub SH2");
	}
	
	// Country code
	CheckMenuRadioItem(CPUMenu_Country,
			   IDM_CPU_COUNTRY_AUTO,
			   IDM_CPU_COUNTRY_JAPAN_PAL,
			   IDM_CPU_COUNTRY + (Country + 1),
			   MF_BYCOMMAND);
	
	// SegaCD Perfect Sync
	CheckMenuItem(CPUMenu, IDM_CPU_SEGACDPERFECTSYNC,
		      MF_BYCOMMAND | (SegaCD_Accurate ? MF_CHECKED : MF_UNCHECKED));
#endif
}


#ifdef GENS_DEBUGGER
/**
 * Sync_Gens_Window_CPUMenu_Debug(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
void Sync_Gens_Window_CPUMenu_Debug(HMENU parent, int position)
{
#if 0
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
			
			InsertMenu(CPUMenu_Debug, i + (i / 3), MF_BYPOSITION | MF_STRING, IDM_CPU_DEBUG + i, DebugStr[i]);
		}
	}
#endif
}
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
#if 0
	// Get the Enabled flag for the other menu items.
	bool soundEnabled = audio->enabled();
	
	// Enabled
	CheckMenuItem(SoundMenu, IDM_SOUND_ENABLE,
		      MF_BYCOMMAND | (soundEnabled ? MF_CHECKED : MF_UNCHECKED));
	
	const int soundMenuItems[11][2] =
	{
		{audio->stereo(), IDM_SOUND_STEREO},
		{Z80_State & 1, IDM_SOUND_Z80},
		{YM2612_Enable, IDM_SOUND_YM2612},
		{YM2612_Improv, IDM_SOUND_YM2612_IMPROVED},
		{DAC_Enable, IDM_SOUND_DAC},
		{DAC_Improv, IDM_SOUND_DAC_IMPROVED},
		{PSG_Enable, IDM_SOUND_PSG},
		{PSG_Improv, IDM_SOUND_PSG_IMPROVED},
		{PCM_Enable, IDM_SOUND_PCM},
		{PWM_Enable, IDM_SOUND_PWM},
		{CDDA_Enable, IDM_SOUND_CDDA},
	};
	
	for (int i = 0; i < 11; i++)
	{
		EnableMenuItem(SoundMenu, soundMenuItems[i][1],
			       MF_BYCOMMAND | (soundEnabled ? MF_ENABLED : MF_GRAYED));
		
		CheckMenuItem(SoundMenu, soundMenuItems[i][1],
			      MF_BYCOMMAND | (soundMenuItems[i][0] ? MF_CHECKED : MF_UNCHECKED));
	}
	
	// Rate
	// TODO: This const array is from gens_window.c.
	// Move it somewhere else.
	const int SndRates[6][2] =
	{
		{0, 11025}, {3, 16000}, {1, 22050},
		{4, 32000}, {2, 44100}, {5, 48000},
	};
	
	for (int i = 0; i < 6; i++)
	{
		if (SndRates[i][1] == audio->soundRate())
		{
			CheckMenuRadioItem(SoundMenu,
					   IDM_SOUND_RATE_11025,
					   IDM_SOUND_RATE_48000,
					   IDM_SOUND_RATE + SndRates[i][0],
					   MF_BYCOMMAND);
			break;
		}
	}
	
	char dumpLabel[16];
	
	// WAV dumping
	// TODO: Always disabled for now, since WAV dumping isn't implemented yet.
	strcpy(dumpLabel, (audio->dumpingWAV() ? "Stop WAV Dump" : "Start WAV Dump"));
	ModifyMenu(SoundMenu, IDM_SOUND_WAVDUMP, MF_BYCOMMAND | MF_STRING, IDM_SOUND_WAVDUMP, dumpLabel);
	EnableMenuItem(SoundMenu, IDM_SOUND_WAVDUMP, MF_BYCOMMAND | MF_GRAYED);
	
	// GYM dumping
	strcpy(dumpLabel, (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump"));
	ModifyMenu(SoundMenu, IDM_SOUND_GYMDUMP, MF_BYCOMMAND | MF_STRING, IDM_SOUND_GYMDUMP, dumpLabel);
	EnableMenuItem(SoundMenu, IDM_SOUND_GYMDUMP,
		       MF_BYCOMMAND | ((Genesis_Started || SegaCD_Started || _32X_Started) ? MF_ENABLED : MF_GRAYED));
#endif
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
#if 0
	// SegaCD SRAM Size
	int SRAM_ID = (BRAM_Ex_State & 0x100 ? BRAM_Ex_Size : -1);
	CheckMenuRadioItem(OptionsMenu_SegaCDSRAMSize,
			   IDM_OPTIONS_SEGACDSRAMSIZE_NONE,
			   IDM_OPTIONS_SEGACDSRAMSIZE_64KB,
			   IDM_OPTIONS_SEGACDSRAMSIZE + (SRAM_ID + 1),
			   MF_BYCOMMAND);
#endif
}
