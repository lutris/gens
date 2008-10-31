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

#include "gens_window.hpp"
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

// Gens Win32 resources
#include "ui/win32/resource.h"

// New menu handler.
#include "ui/common/gens/gens_menu.h"
#include "ui/common/gens/gens_menu_callbacks.hpp"
static void Win32_ParseMenu(GensMenuItem_t *menu, HMENU container);

// Accelerator table for the main Gens window. [Menu commands.]
HACCEL hAccelTable_Menu = NULL;

// Unordered map containing all the menu items.
// Map key is the menu ID.
// TODO: unordered_map is gcc-4.x and later.
// For gcc-3.x, use __gnu_cxx::hash_map.
#include <tr1/unordered_map>
using std::tr1::unordered_map;
win32MenuMap gensMenuMap;

// Menu objects
HMENU MainMenu;
#if 0
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
#endif

void create_gens_window_menubar(void);

#if 0
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
#endif


/**
 * initGens_hWnd(): Initialize the Gens window.
 * @return hWnd.
 */
HWND initGens_hWnd(void)
{
	// This function simply initializes the base window.
	// It's needed because DirectX needs the window handle in order to set cooperative levels.
	
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Gens_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = NULL;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens";
	
	RegisterClass(&WndClass);
	
	Gens_hWnd = CreateWindowEx(NULL, "Gens", "Gens", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				   320 * 2, 240 * 2, NULL, NULL, ghInstance, NULL);
	
	// Accept dragged files.
	DragAcceptFiles(Gens_hWnd, TRUE);
	
	return Gens_hWnd;
}


HWND create_gens_window(void)
{
	// Create the menu bar.
	create_gens_window_menubar();
	
	return Gens_hWnd;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 */
void create_gens_window_menubar(void)
{
	// TODO: Popup menu if fullscreen.
	DestroyMenu(MainMenu);
	
	// Create the main menu.
	if (draw->fullScreen())
		MainMenu = CreatePopupMenu();
	else
		MainMenu = CreateMenu();
	
	// Menus
	Win32_ParseMenu(&gmiMain[0], MainMenu);
#if 0
	create_gens_window_FileMenu(MainMenu, 0);
	create_gens_window_GraphicsMenu(MainMenu, 1);
	create_gens_window_CPUMenu(MainMenu, 2);
	create_gens_window_SoundMenu(MainMenu, 3);
	create_gens_window_OptionsMenu(MainMenu, 4);
	create_gens_window_HelpMenu(MainMenu, 5);
#endif
	
	// Set the menu bar.
	if (draw->fullScreen())
		SetMenu(Gens_hWnd, NULL);
	else
		SetMenu(Gens_hWnd, MainMenu);
}


/**
 * Win32_ParseMenu(): Parse the menu structs.
 * @param menu First item of the array of menu structs to parse.
 * @param container Container to add the menu items to.
 */
static void Win32_ParseMenu(GensMenuItem_t *menu, HMENU container)
{
	HMENU mnuSubMenu;
	string sMenuText;
	
	// Win32 InsertMenu() parameters.
	unsigned int uFlags;
	UINT_PTR uIDNewItem;
	
	while (menu->id != 0)
	{
		// Check what type of menu item this is.
		switch ((menu->flags & GMF_ITEM_MASK))
		{
			case GMF_ITEM_SEPARATOR:
				// Separator.
				uFlags = MF_BYPOSITION | MF_SEPARATOR;
				uIDNewItem = (UINT_PTR)menu->id;
				break;
			
			case GMF_ITEM_SUBMENU:
				// Submenu.
				if (!menu->submenu)
				{
					// No submenu specified. Create a normal menu item for now.
					uFlags = MF_BYPOSITION | MF_STRING;
					uIDNewItem = (UINT_PTR)menu->id;
				}
				else
				{
					// Submenu specified.
					mnuSubMenu = CreatePopupMenu();
					Win32_ParseMenu(menu->submenu, mnuSubMenu);
					uFlags = MF_BYPOSITION | MF_STRING | MF_POPUP;
					uIDNewItem = (UINT_PTR)mnuSubMenu;
				}
				break;
			
			case GMF_ITEM_CHECK:
			case GMF_ITEM_RADIO:
			default:
				// Menu item. (Win32 doesn't treat check or radio items as different types.)
				uFlags = MF_BYPOSITION | MF_STRING;
				uIDNewItem = (UINT_PTR)menu->id;
				break;
		}
		
		// Set the menu text.
		if (menu->text)
			sMenuText = menu->text;
		else
			sMenuText.clear();
		
		// Check for an accelerator.
		if (menu->accelKey != 0)
		{
			// Accelerator specified.
			// TODO: Add the accelerator to the accelerator table.
			//int accelModifier = 0;
			int accelKey;
			
			sMenuText += "\t";
			
			// Determine the modifier.
			if (menu->accelModifier & GMAM_CTRL)
			{
				//accelModifier |= GDK_CONTROL_MASK;
				sMenuText += "Ctrl+";
			}
			if (menu->accelModifier & GMAM_ALT)
			{
				//accelModifier |= GDK_MOD1_MASK;
				sMenuText += "Alt+";
			}
			if (menu->accelModifier & GMAM_SHIFT)
			{
				//accelModifier |= GDK_SHIFT_MASK;
				sMenuText += "Shift+";
			}
			
			// Determine the key.
			// TODO: Add more special keys.
			char tmpKey[8];
			switch (menu->accelKey)
			{
				case GMAK_BACKSPACE:
					accelKey = VK_BACK;
					sMenuText += "Backspace";
					break;
				
				case GMAK_ENTER:
					accelKey = VK_RETURN;
					sMenuText += "Enter";
					break;
				
				case GMAK_TAB:
					accelKey = VK_TAB;
					sMenuText += "Tab";
					break;
				
				case GMAK_F1: case GMAK_F2:  case GMAK_F3:  case GMAK_F4:
				case GMAK_F5: case GMAK_F6:  case GMAK_F7:  case GMAK_F8:
				case GMAK_F9: case GMAK_F10: case GMAK_F11: case GMAK_F12:
					accelKey = (menu->accelKey - GMAK_F1) + VK_F1;
					sprintf(tmpKey, "F%d", (menu->accelKey - GMAK_F1 + 1));
					sMenuText += string(tmpKey);
					break;
					
				default:
					accelKey = menu->accelKey;
					sMenuText += (char)(menu->accelKey);
					break;
			}
			
			// Add the accelerator.
			// TODO
		}
		
		// Add the menu item to the container.
		InsertMenu(container, -1, uFlags, uIDNewItem, sMenuText.c_str());
		
		// Add the menu to the menu map. (Exception is if id is 0 or IDM_SEPARATOR.)
#if 0
		if (menu->id != 0 && menu->id != IDM_SEPARATOR)
		{
			gensMenuMap.insert(gtkMenuMapItem(menu->id, mnuItem));
		}
#endif
		
		// Next menu item.
		menu++;
	}
}


#if 0
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)FileMenu, "&File");
	
	InsertMenu(FileMenu, 0, flags, IDM_FILE_OPENROM, "&Open ROM...\tCtrl+O");
#ifdef GENS_CDROM
	InsertMenu(FileMenu, 1, flags, IDM_FILE_BOOTCD, "&Boot CD\tCtrl+B");
#endif /* GENS_CDROM */
	InsertMenu(FileMenu, 2, flags | MF_GRAYED, IDM_FILE_NETPLAY, "&Netplay");
	InsertMenu(FileMenu, 3, flags, NULL, "ROM &History"); // DUMMY menu item
	InsertMenu(FileMenu, 4, flags, IDM_FILE_CLOSEROM, "&Close ROM\tCtrl+W");
	
	InsertMenu(FileMenu, 5, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(FileMenu, 6, flags, IDM_FILE_GAMEGENIE, "&Game Genie\tCtrl+G");
	
	InsertMenu(FileMenu, 7, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(FileMenu, 8, flags, IDM_FILE_LOADSTATE, "&Load State...\tShift+F8");
	InsertMenu(FileMenu, 9, flags, IDM_FILE_SAVESTATE, "&Save State As...\tShift+F5");
	InsertMenu(FileMenu, 10, flags, IDM_FILE_QUICKLOAD, "Quick Load\tF8");
	InsertMenu(FileMenu, 11, flags, IDM_FILE_QUICKSAVE, "Quick Save\tF5");
	create_gens_window_FileMenu_ChangeState(FileMenu, 12);
	
	InsertMenu(FileMenu, 13, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(FileMenu, 14, flags, IDM_FILE_EXIT, "E&xit\tCtrl+Q");
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)FileMenu_ChangeState, "Change State");
	
	char mnuTitle[2] = {'\0', '\0'};
	
	// Create the save slot entries.
	for (int i = 0; i < 10; i++)
	{
		mnuTitle[0] = '0' + (char)i;
		InsertMenu(FileMenu_ChangeState, i, MF_BYPOSITION | MF_STRING,
			   IDM_FILE_CHANGESTATE + i, mnuTitle);
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)GraphicsMenu, "&Graphics");
	
	InsertMenu(GraphicsMenu, 0, flags, IDM_GRAPHICS_FULLSCREEN, "&Full Screen\tAlt+Enter");
	InsertMenu(GraphicsMenu, 1, flags, IDM_GRAPHICS_VSYNC, "&VSync\tShift+F3");
	InsertMenu(GraphicsMenu, 2, flags, IDM_GRAPHICS_STRETCH, "&Stretch\tShift+F2");
	
	InsertMenu(GraphicsMenu, 3, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 4, flags, IDM_GRAPHICS_COLORADJUST, "&Color Adjust...");
	Sync_Gens_Window_GraphicsMenu_Render(GraphicsMenu, 5);
	
	InsertMenu(GraphicsMenu, 6, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 7, flags, IDM_GRAPHICS_SPRITELIMIT, "Sprite Limit");
	
	InsertMenu(GraphicsMenu, 8, MF_SEPARATOR, NULL, NULL);
	
	create_gens_window_GraphicsMenu_FrameSkip(GraphicsMenu, 9);
	
	InsertMenu(GraphicsMenu, 10, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 11, flags, IDM_GRAPHICS_SCREENSHOT, "Screen Shot\tShift+Backspace");
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)GraphicsMenu_FrameSkip, "Frame Skip");
	
	char mnuTitle[8];
	
	// Create the frame skip entries.
	for (int i = -1; i <= 8; i++)
	{
		if (i >= 0)
			sprintf(mnuTitle, "%d", i);
		else
			strcpy(mnuTitle, "Auto");
		
		InsertMenu(GraphicsMenu_FrameSkip, i + 1, MF_BYPOSITION | MF_STRING,
			   IDM_GRAPHICS_FRAMESKIP + (i + 1), mnuTitle);
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)CPUMenu, "&CPU");
	
#ifdef GENS_DEBUGGER
	Sync_Gens_Window_CPUMenu_Debug(CPUMenu, 0);
	InsertMenu(CPUMenu, 1, MF_SEPARATOR, NULL, NULL);
#endif /* GENS_DEBUGGER */
	
	create_gens_window_CPUMenu_Country(CPUMenu, 2);
	
	InsertMenu(CPUMenu, 3, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(CPUMenu, 4, flags, IDM_CPU_HARDRESET, "Hard &Reset\tTab");
	// SegaCD and 32X menu items are handled in the Sync_Gens_Window_CPUMenu() function.
	InsertMenu(CPUMenu, 10, flags, IDM_CPU_RESETZ80, "Reset Z80");
	
	InsertMenu(CPUMenu, 11, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(CPUMenu, 12, flags, IDM_CPU_SEGACDPERFECTSYNC, "SegaCD Perfect Sync (SLOW)");
}


/**
 * create_gens_window_CPUMenu_Country_SubMenu(): Create the CPU, Country submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_CPUMenu_Country(HMENU parent, int position)
{
	// TODO: Move this array somewhere else.
	char* CountryCodes[5] =
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)CPUMenu_Country, "&Country");
	
	// Create the country code entries.
	for (int i = 0; i < 5; i++)
	{
		InsertMenu(CPUMenu_Country, i, MF_BYPOSITION | MF_STRING,
			   IDM_CPU_COUNTRY + i, CountryCodes[i]);
	}
	
	// Separator
	InsertMenu(CPUMenu_Country, 5, MF_SEPARATOR, NULL, NULL);
	
	// Add the Auto-Detection Order configuration option.
	InsertMenu(CPUMenu_Country, 6, MF_BYPOSITION | MF_STRING, IDM_CPU_COUNTRY_ORDER, "Auto-Detection Order...");
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
	InsertMenu(parent, 3, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)SoundMenu, "&Sound");
	
	InsertMenu(SoundMenu, 0, flags, IDM_SOUND_ENABLE, "&Enable");
	
	InsertMenu(SoundMenu, 1, MF_SEPARATOR, NULL, NULL);
	
	create_gens_window_SoundMenu_Rate(SoundMenu, 2);
	InsertMenu(SoundMenu, 3, flags, IDM_SOUND_STEREO, "&Stereo");
	
	InsertMenu(SoundMenu, 4, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(SoundMenu, 5, flags, IDM_SOUND_Z80, "&Z80");
	
	InsertMenu(SoundMenu, 6, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(SoundMenu, 7, flags, IDM_SOUND_YM2612, "&YM2612");
	InsertMenu(SoundMenu, 8, flags, IDM_SOUND_YM2612_IMPROVED, "YM2612 Improved");
	InsertMenu(SoundMenu, 9, flags, IDM_SOUND_DAC, "&DAC");
	InsertMenu(SoundMenu, 10, flags, IDM_SOUND_DAC_IMPROVED, "DAC Improved");
	InsertMenu(SoundMenu, 11, flags, IDM_SOUND_PSG, "&PSG");
	InsertMenu(SoundMenu, 12, flags, IDM_SOUND_PSG_IMPROVED, "PSG Improved");
	InsertMenu(SoundMenu, 13, flags, IDM_SOUND_PCM, "P&CM");
	InsertMenu(SoundMenu, 14, flags, IDM_SOUND_PWM, "P&WM");
	InsertMenu(SoundMenu, 15, flags, IDM_SOUND_CDDA, "CDDA");
	
	InsertMenu(SoundMenu, 16, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(SoundMenu, 17, flags, IDM_SOUND_WAVDUMP, "Start WAV Dump");
	InsertMenu(SoundMenu, 18, flags, IDM_SOUND_GYMDUMP, "Start GYM Dump");
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)SoundMenu_Rate, "&Rate");
	
	// Create the rate entries.
	for (i = 0; i < 6; i++)
	{
		sprintf(SndName, "%d Hz", SndRates[i][1]);
		InsertMenu(SoundMenu_Rate, i, MF_BYPOSITION | MF_STRING,
			   IDM_SOUND_RATE + SndRates[i][0], &SndName[0]);
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)OptionsMenu, "&Options");
	
	InsertMenu(OptionsMenu, 0, flags, IDM_OPTIONS_GENERAL, "&General Options...");
	InsertMenu(OptionsMenu, 1, flags, IDM_OPTIONS_JOYPADS, "&Joypads...");
	InsertMenu(OptionsMenu, 2, flags, IDM_OPTIONS_DIRECTORIES, "&Directories...");
	InsertMenu(OptionsMenu, 3, flags, IDM_OPTIONS_BIOSMISCFILES, "&BIOS/Misc Files...");
	
	InsertMenu(OptionsMenu, 4, MF_SEPARATOR, NULL, NULL);
	
#ifdef GENS_CDROM
	InsertMenu(OptionsMenu, 5, flags, IDM_OPTIONS_CURRENT_CD_DRIVE, "Current &CD Drive...");
#endif /* GENS_CDROM */
	create_gens_window_OptionsMenu_SegaCDSRAMSize(OptionsMenu, 6);
	
	InsertMenu(OptionsMenu, 7, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(OptionsMenu, 8, flags, IDM_OPTIONS_LOADCONFIG, "&Load Config...");
	InsertMenu(OptionsMenu, 9, flags, IDM_OPTIONS_SAVECONFIGAS, "&Save Config As...");
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)OptionsMenu_SegaCDSRAMSize, "SegaCD S&RAM Size");
	
	// Create the rate entries.
	for (i = -1; i <= 3; i++)
	{
		if (i == -1)
			strcpy(SRAMName, "None");
		else
			sprintf(SRAMName, "%d KB", 8 << i);
		
		InsertMenu(OptionsMenu_SegaCDSRAMSize, i + 1, MF_BYPOSITION | MF_STRING,
			   IDM_OPTIONS_SEGACDSRAMSIZE + (i + 1), SRAMName);
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
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING,
		   (UINT_PTR)HelpMenu, "&Help");
	
	InsertMenu(HelpMenu, 0, flags, IDM_HELP_ABOUT, "&About");
}
#endif
