/***************************************************************************
 * Gens: (Win32) Main Window.                                              *
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

#include "gens_window.h"
#include "gens_window_sync.hpp"
#include "gens_window_callbacks.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

WNDCLASS WndClass;
HWND Gens_hWnd = NULL;

#ifdef GENS_DEBUGGER
// Debug menu items
//GtkWidget *debugMenuItems[9];
//GtkWidget *debugSeparators[2];
#endif /* GENS_DEBUGGER */


// Menu identifier definitions
#include "gens_window_menu.h"


// Menu objects
HMENU MainMenu;
HMENU FileMenu;
HMENU FileMenu_ROMHistory;
HMENU FileMenu_ChangeState;
HMENU GraphicsMenu;
HMENU GraphicsMenu_Render;
HMENU GraphicsMenu_FrameSkip;
HMENU CPUMenu;
HMENU CPUMenu_Debug;
HMENU CPUMenu_Country;
HMENU SoundMenu;
HMENU SoundMenu_Rate;
HMENU OptionsMenu;
HMENU OptionsMenu_SegaCDSRAMSize;
HMENU HelpMenu;


static void create_gens_window_menubar(void);
static void create_gens_window_FileMenu(HMENU parent, int position);
static void create_gens_window_FileMenu_ChangeState(HMENU parent, int position);
static void create_gens_window_GraphicsMenu(HMENU parent, int position);
static void create_gens_window_GraphicsMenu_FrameSkip(HMENU parent, int position);
static void create_gens_window_CPUMenu(HMENU parent, int position);
static void create_gens_window_CPUMenu_Country(HMENU parent, int position);
static void create_gens_window_SoundMenu(HMENU parent, int position);
static void create_gens_window_SoundMenu_Rate(HMENU parent, int position);
static void create_gens_window_OptionsMenu(HMENU parent, int position);
static void create_gens_window_OptionsMenu_SegaCDSRAMSize(HMENU parent, int position);
static void create_gens_window_HelpMenu(HMENU parent, int position);
#if 0
static void create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(GtkWidget *container);
#endif


/**
 * initGens_hWnd(): Initialize the Gens window.
 * @return hWnd.
 */
HWND initGens_hWnd(void)
{
	// This function simply initializes the base window.
	// It's needed because DirectX needs the window handle to set cooperative levels.
	
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Gens_Window_WinProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	//WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = NULL;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens";
	
	RegisterClass(&WndClass);
	
	Gens_hWnd = CreateWindowEx(NULL, "Gens", "Gens", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				   320 * 2, 240 * 2, NULL, NULL, ghInstance, NULL);
	
	return Gens_hWnd;
}


HWND create_gens_window(void)
{
	// Create the menu bar.
	create_gens_window_menubar();
	SetMenu(Gens_hWnd, MainMenu);
	
	return Gens_hWnd;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 */
static void create_gens_window_menubar(void)
{
	// TODO: Popup menu if fullscreen.
	DestroyMenu(MainMenu);
	
	// Create the main menu.
	MainMenu = CreateMenu();
	
	// Menus
	create_gens_window_FileMenu(MainMenu, 0);
	create_gens_window_GraphicsMenu(MainMenu, 1);
	create_gens_window_CPUMenu(MainMenu, 2);
	create_gens_window_SoundMenu(MainMenu, 3);
	create_gens_window_OptionsMenu(MainMenu, 4);
	create_gens_window_HelpMenu(MainMenu, 5);
}


/**
 * create_gens_window_FileMenu(): Create the File menu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_FileMenu(HMENU parent, int position)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
	// File
	DeleteMenu(parent, position, MF_BYPOSITION);
	FileMenu = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, FileMenu, "&File");
	
	InsertMenu(FileMenu, 0, flags, ID_FILE_OPENROM, "&Open ROM...");
#ifdef GENS_CDROM
	InsertMenu(FileMenu, 1, flags, ID_FILE_BOOTCD, "&Boot CD");
#endif /* GENS_CDROM */
	InsertMenu(FileMenu, 2, flags | MF_GRAYED, ID_FILE_NETPLAY, "&Netplay");
	// ROM History submenu is handled by Sync_Gens_Window_FileMenu().
	InsertMenu(FileMenu, 4, flags, ID_FILE_CLOSEROM, "&Close ROM");
	
	InsertMenu(FileMenu, 5, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(FileMenu, 6, flags, ID_FILE_GAMEGENIE, "&Game Genie");
	
	InsertMenu(FileMenu, 7, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(FileMenu, 8, flags, ID_FILE_LOADSTATE, "&Load State...");
	InsertMenu(FileMenu, 9, flags, ID_FILE_SAVESTATE, "&Save State As...");
	InsertMenu(FileMenu, 10, flags, ID_FILE_QUICKLOAD, "Quick Load");
	InsertMenu(FileMenu, 11, flags, ID_FILE_QUICKSAVE, "Quick Save");
	//InsertMenu(FileMenu, 12, flags, ID_FILE_CHANGESTATE, "Change State");
	create_gens_window_FileMenu_ChangeState(FileMenu, 12);
	
	InsertMenu(FileMenu, 13, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(FileMenu, 14, flags, ID_FILE_QUIT, "&Quit");
}


/**
 * create_gens_window_FileMenu_ChangeState(): Create the File, Change State submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_FileMenu_ChangeState(HMENU parent, int position)
{
	// File, Change State
	DeleteMenu(parent, position, MF_BYPOSITION);
	FileMenu_ChangeState = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, FileMenu_ChangeState, "Change State");
	
	MENUITEMINFO miimStateItem;
	char mnuTitle[2];
	int i;
	
	memset(&miimStateItem, 0x00, sizeof(miimStateItem));
	miimStateItem.cbSize = sizeof(miimStateItem);
	miimStateItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
	miimStateItem.fType = MFT_RADIOCHECK | MFT_STRING;
	
	// Create the save slot entries.
	for (i = 0; i < 10; i++)
	{
		miimStateItem.wID = ID_FILE_CHANGESTATE + i;
		miimStateItem.fState = (Current_State == i ? MFS_CHECKED : MFS_UNCHECKED);
		mnuTitle[0] = '0' + i;
		mnuTitle[1] = 0;
		miimStateItem.dwTypeData = &mnuTitle;
		miimStateItem.cch = 1;
		InsertMenuItem(FileMenu_ChangeState, i, TRUE, &miimStateItem);
	}
}


/**
 * create_gens_window_GraphicsMenu(): Create the Graphics menu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_GraphicsMenu(HMENU parent, int position)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
	// Graphics
	DeleteMenu(parent, position, MF_BYPOSITION);
	GraphicsMenu = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, GraphicsMenu, "&Graphics");
	
	InsertMenu(GraphicsMenu, 0, flags, ID_GRAPHICS_FULLSCREEN, "&Full Screen");
	InsertMenu(GraphicsMenu, 1, flags, ID_GRAPHICS_VSYNC, "&VSync");
	InsertMenu(GraphicsMenu, 2, flags, ID_GRAPHICS_STRETCH, "&Stretch");
	
	InsertMenu(GraphicsMenu, 3, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 4, flags, ID_GRAPHICS_COLORADJUST, "&Color Adjust...");
	Sync_Gens_Window_GraphicsMenu_Render(GraphicsMenu, 5);
	
	InsertMenu(GraphicsMenu, 6, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 7, flags, ID_GRAPHICS_SPRITELIMIT, "Sprite Limit");
	
	InsertMenu(GraphicsMenu, 8, MF_SEPARATOR, NULL, NULL);
	
	create_gens_window_GraphicsMenu_FrameSkip(GraphicsMenu, 9);
	
	InsertMenu(GraphicsMenu, 10, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 11, flags, ID_GRAPHICS_SCREENSHOT, "Screen Shot");
}


/**
 * create_gens_window_GraphicsMenu_FrameSkip(): Create the Graphics, Frame Skip submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_GraphicsMenu_FrameSkip(HMENU parent, int position)
{
	// Graphics, Frame Skip
	DeleteMenu(parent, position, MF_BYPOSITION);
	GraphicsMenu_FrameSkip = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, GraphicsMenu_FrameSkip, "Frame Skip");
	
	char mnuTitle[8];
	int i;
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
	miimMenuItem.fType = MFT_RADIOCHECK | MFT_STRING;
	
	// Create the frame skip entries.
	for (i = -1; i <= 8; i++)
	{
		if (i >= 0)
		{
			sprintf(mnuTitle, "%d", i);
			miimMenuItem.cch = 1;
		}
		else
		{
			strcpy(mnuTitle, "Auto");
			miimMenuItem.cch = 4;
		}
		
		miimMenuItem.wID = ID_GRAPHICS_FRAMESKIP + (i + 1);
		miimMenuItem.fState = (Frame_Skip == i ? MFS_CHECKED : MFS_UNCHECKED);
		miimMenuItem.dwTypeData = &mnuTitle;
		InsertMenuItem(GraphicsMenu_FrameSkip, i + 1, TRUE, &miimMenuItem);
	}
}


/**
 * create_gens_window_CPUMenu(): Create the CPU menu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_CPUMenu(HMENU parent, int position)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
	// CPU
	DeleteMenu(parent, position, MF_BYPOSITION);
	CPUMenu = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, CPUMenu, "&CPU");
	
#ifdef GENS_DEBUGGER
	Sync_Gens_Window_CPUMenu_Debug(CPUMenu, 0);
	InsertMenu(CPUMenu, 1, MF_SEPARATOR, NULL, NULL);
#endif /* GENS_DEBUGGER */
	
	create_gens_window_CPUMenu_Country(CPUMenu, 2);
	
	InsertMenu(CPUMenu, 3, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(CPUMenu, 4, flags, ID_CPU_HARDRESET, "Hard &Reset");
	// SegaCD and 32X menu items are handled in the Sync_Gens_Window_CPUMenu() function.
	InsertMenu(CPUMenu, 10, flags, ID_CPU_RESETZ80, "Reset Z80");
	
	InsertMenu(CPUMenu, 11, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(CPUMenu, 12, flags, ID_CPU_SEGACDPERFECTSYNC, "SegaCD Perfect Sync (SLOW)");
}


/**
 * create_gens_window_CPUMenu_Country_SubMenu(): Create the CPU, Country submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_CPUMenu_Country(HMENU parent, int position)
{
	// TODO: Move this array somewhere else.
	const char* CountryCodes[5] =
	{
		"Auto Detect",
		"Japan (NTSC)",
		"USA (NTSC)",
		"Europe (PAL)",
		"Japan (PAL)",
	};
	
	// CPU, Country
	DeleteMenu(parent, position, MF_BYPOSITION);
	CPUMenu_Country = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, CPUMenu_Country, "&Country");
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
	miimMenuItem.fType = MFT_RADIOCHECK | MFT_STRING;
	
	// Create the country code entries.
	int i;
	for (i = 0; i < 5; i++)
	{
		miimMenuItem.wID = ID_CPU_COUNTRY + i;
		miimMenuItem.fState = (i == 0 ? MFS_CHECKED : MFS_UNCHECKED);
		miimMenuItem.dwTypeData = CountryCodes[i];
		miimMenuItem.cch = strlen(CountryCodes[i]);
		
		InsertMenuItem(CPUMenu_Country, i, TRUE, &miimMenuItem);
	}
	
	// Separator
	InsertMenu(CPUMenu_Country, 5, MF_SEPARATOR, NULL, NULL);
	
	// Add the Auto-Detection Order configuration option.
	InsertMenu(CPUMenu_Country, 6, MF_BYPOSITION | MF_STRING, ID_CPU_COUNTRY_ORDER, "Auto-Detection Order...");
}


/**
 * create_gens_window_SoundMenu(): Create the Sound menu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_SoundMenu(HMENU parent, int position)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
	// Sound
	DeleteMenu(parent, position, MF_BYPOSITION);
	SoundMenu = CreatePopupMenu();
	InsertMenu(parent, 3, MF_BYPOSITION | MF_POPUP | MF_STRING, SoundMenu, "&Sound");
	
	InsertMenu(SoundMenu, 0, flags, ID_SOUND_ENABLE, "&Enable");
	
	InsertMenu(SoundMenu, 1, MF_SEPARATOR, NULL, NULL);
	
	create_gens_window_SoundMenu_Rate(SoundMenu, 2);
	InsertMenu(SoundMenu, 3, flags, ID_SOUND_STEREO, "&Stereo");
	
	InsertMenu(SoundMenu, 4, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(SoundMenu, 5, flags, ID_SOUND_Z80, "&Z80");
	
	InsertMenu(SoundMenu, 6, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(SoundMenu, 7, flags, ID_SOUND_YM2612, "&YM2612");
	InsertMenu(SoundMenu, 8, flags, ID_SOUND_YM2612_IMPROVED, "YM2612 Improved");
	InsertMenu(SoundMenu, 9, flags, ID_SOUND_DAC, "&DAC");
	InsertMenu(SoundMenu, 10, flags, ID_SOUND_DAC_IMPROVED, "DAC Improved");
	InsertMenu(SoundMenu, 11, flags, ID_SOUND_PSG, "&PSG");
	InsertMenu(SoundMenu, 12, flags, ID_SOUND_PSG_IMPROVED, "PSG Improved");
	InsertMenu(SoundMenu, 13, flags, ID_SOUND_PCM, "P&CM");
	InsertMenu(SoundMenu, 14, flags, ID_SOUND_PWM, "P&WM");
	InsertMenu(SoundMenu, 15, flags, ID_SOUND_CDDA, "CDDA");
	
	InsertMenu(SoundMenu, 16, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(SoundMenu, 17, flags, ID_SOUND_WAVDUMP, "Start WAV Dump");
	InsertMenu(SoundMenu, 18, flags, ID_SOUND_GYMDUMP, "Start GYM Dump");
}


/**
 * create_gens_window_SoundMenu_Rate(): Create the Sound, Rate submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_SoundMenu_Rate(HMENU parent, int position)
{
	// Sample rates are referenced by an index.
	// The index is not sorted by rate; the xx000 rates are 3, 4, 5.
	// This is probably for backwards-compatibilty with older Gens.
	const int SndRates[6][2] =
	{
		{0, 11025}, {3, 16000}, {1, 22050},
		{4, 32000}, {2, 44100}, {5, 48000},
	};
	
	int i;
	char SndName[16];
	
	// Sound, Rate
	DeleteMenu(parent, position, MF_BYPOSITION);
	SoundMenu_Rate = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, SoundMenu_Rate, "&Rate");
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
	miimMenuItem.fType = MFT_RADIOCHECK | MFT_STRING;
	
	// Create the rate entries.
	for (i = 0; i < 6; i++)
	{
		sprintf(SndName, "%d Hz", SndRates[i][1]);
		
		miimMenuItem.wID = ID_SOUND_RATE + SndRates[i][0];
		miimMenuItem.fState = (SndRates[i][1] == 22050 ? MFS_CHECKED : MFS_UNCHECKED);
		miimMenuItem.dwTypeData = &SndName;
		miimMenuItem.cch = strlen(SndName);
		
		InsertMenuItem(SoundMenu_Rate, i, TRUE, &miimMenuItem);
	}
}


/**
 * create_gens_window_OptionsMenu(): Create the Options menu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_OptionsMenu(HMENU parent, int position)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
	// Options
	DeleteMenu(parent, position, MF_BYPOSITION);
	OptionsMenu = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, OptionsMenu, "&Options");
	
	InsertMenu(OptionsMenu, 0, flags, ID_OPTIONS_GENERAL, "&General Options...");
	InsertMenu(OptionsMenu, 1, flags, ID_OPTIONS_JOYPADS, "&Joypads...");
	InsertMenu(OptionsMenu, 2, flags, ID_OPTIONS_DIRECTORIES, "&Directories...");
	InsertMenu(OptionsMenu, 3, flags, ID_OPTIONS_BIOS_MISC_FILES, "&BIOS/Misc Files...");
	
	InsertMenu(OptionsMenu, 4, MF_SEPARATOR, NULL, NULL);
	
#ifdef GENS_CDROM
	InsertMenu(OptionsMenu, 5, flags, ID_OPTIONS_CURRENT_CD_DRIVE, "Current &CD Drive...");
#endif /* GENS_CDROM */
	create_gens_window_OptionsMenu_SegaCDSRAMSize(OptionsMenu, 6);
	
	InsertMenu(OptionsMenu, 7, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(OptionsMenu, 8, flags, ID_OPTIONS_LOADCONFIG, "&Load Config...");
	InsertMenu(OptionsMenu, 9, flags, ID_OPTIONS_SAVECONFIGAS, "&Save Config As...");
}


/**
 * create_gens_window_OptionsMenu_SegaCDSRAMSize(): Create the Options, Sega CD SRAM Size submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_OptionsMenu_SegaCDSRAMSize(HMENU parent, int position)
{
	int i;
	char SRAMName[16];
	
	// Options, SegaCD SRAM Size
	DeleteMenu(parent, position, MF_BYPOSITION);
	OptionsMenu_SegaCDSRAMSize = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, OptionsMenu_SegaCDSRAMSize, "SegaCD S&RAM Size");
	
	MENUITEMINFO miimMenuItem;
	memset(&miimMenuItem, 0x00, sizeof(miimMenuItem));
	miimMenuItem.cbSize = sizeof(miimMenuItem);
	miimMenuItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
	miimMenuItem.fType = MFT_RADIOCHECK | MFT_STRING;
	
	// Create the rate entries.
	for (i = -1; i <= 3; i++)
	{
		if (i == -1)
			strcpy(SRAMName, "None");
		else
			sprintf(SRAMName, "%d KB", 8 << i);
		
		miimMenuItem.wID = ID_OPTIONS_SEGACDSRAMSIZE + (i + 1);
		miimMenuItem.fState = (i == -1 ? MFS_CHECKED : MFS_UNCHECKED);
		miimMenuItem.dwTypeData = SRAMName;
		miimMenuItem.cch = strlen(SRAMName);
		
		InsertMenuItem(OptionsMenu_SegaCDSRAMSize, i + 1, TRUE, &miimMenuItem);
	}
}


/**
 * create_gens_window_HelpMenu(): Create the Help menu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_HelpMenu(HMENU parent, int position)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
	// Help
	DeleteMenu(parent, position, MF_BYPOSITION);
	HelpMenu = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, HelpMenu, "&Help");
	
	InsertMenu(HelpMenu, 0, flags, ID_HELP_ABOUT, "&About");
}
