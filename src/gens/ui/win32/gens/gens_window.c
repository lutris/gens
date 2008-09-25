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
HMENU FileMenu_ChangeState;
HMENU GraphicsMenu;
HMENU CPUMenu;
HMENU SoundMenu;
HMENU OptionsMenu;
HMENU HelpMenu;


static void create_gens_window_menubar(void);
static void create_gens_window_FileMenu(HMENU parent, int position);
static void create_gens_window_FileMenu_ChangeState(HMENU parent, int position);
static void create_gens_window_GraphicsMenu(HMENU parent, int position);
static void create_gens_window_CPUMenu(HMENU parent, int position);
static void create_gens_window_SoundMenu(HMENU parent, int position);
static void create_gens_window_OptionsMenu(HMENU parent, int position);
static void create_gens_window_HelpMenu(HMENU parent, int position);
#if 0
static void create_gens_window_GraphicsMenu_FrameSkip_SubMenu(GtkWidget *container);
#ifdef GENS_DEBUGGER
static void create_gens_window_CPUMenu_Debug_SubMenu(GtkWidget *container);
#endif /* GENS_DEBUGGER */
static void create_gens_window_CPUMenu_Country_SubMenu(GtkWidget *container);
static void create_gens_window_SoundMenu_Rate_SubMenu(GtkWidget *container);
static void create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(GtkWidget *container);
#endif


// Set to 0 to temporarily disable callbacks.
int do_callbacks = 1;


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
	DestroyMenu(FileMenu);
	FileMenu = CreatePopupMenu();
	RemoveMenu(parent, position, MF_BYPOSITION);
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, FileMenu, "&File");
	
	InsertMenu(FileMenu, 0, flags, ID_FILE_OPENROM, "&Open ROM...");
#ifdef GENS_CDROM
	InsertMenu(FileMenu, 1, flags, ID_FILE_BOOTCD, "&Boot CD");
#endif /* GENS_CDROM */
	InsertMenu(FileMenu, 2, flags | MF_GRAYED, ID_FILE_NETPLAY, "&Netplay");
	InsertMenu(FileMenu, 3, flags, ID_FILE_ROMHISTORY, "ROM &History");
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
	DestroyMenu(FileMenu_ChangeState);
	FileMenu_ChangeState = CreatePopupMenu();
	RemoveMenu(parent, position, MF_BYPOSITION);
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, FileMenu_ChangeState, "Change State");
	
	MENUITEMINFO mnuStateItem;
	char mnuTitle[2];
	int i;
	
	memset(&mnuStateItem, 0x00, sizeof(mnuStateItem));
	mnuStateItem.cbSize = sizeof(mnuStateItem);
	mnuStateItem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
	mnuStateItem.fType = MFT_RADIOCHECK | MFT_STRING;
	mnuStateItem.hSubMenu = 0;
	mnuStateItem.hbmpChecked = 0;
	mnuStateItem.hbmpUnchecked = 0;
	
	// Create the save slot entries.
	for (i = 0; i < 10; i++)
	{
		mnuStateItem.wID = ID_FILE_CHANGESTATE + i;
		mnuStateItem.fState = (Current_State == i ? MFS_CHECKED : MFS_UNCHECKED);
		mnuTitle[0] = '0' + i;
		mnuTitle[1] = 0;
		mnuStateItem.dwTypeData = &mnuTitle;
		mnuStateItem.cch = 1;
		InsertMenuItem(FileMenu_ChangeState, i, TRUE, &mnuStateItem);
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
	DestroyMenu(GraphicsMenu);
	GraphicsMenu = CreatePopupMenu();
	RemoveMenu(parent, position, MF_BYPOSITION);
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, GraphicsMenu, "&Graphics");
	
	InsertMenu(GraphicsMenu, 0, flags, ID_GRAPHICS_FULLSCREEN, "&Full Screen");
	InsertMenu(GraphicsMenu, 1, flags, ID_GRAPHICS_VSYNC, "&VSync");
	InsertMenu(GraphicsMenu, 2, flags, ID_GRAPHICS_STRETCH, "&Stretch");
	
	InsertMenu(GraphicsMenu, 3, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 4, flags, ID_GRAPHICS_COLORADJUST, "&Color Adjust...");
	InsertMenu(GraphicsMenu, 5, flags, ID_GRAPHICS_RENDER, "&Render");
	
	InsertMenu(GraphicsMenu, 6, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 7, flags, ID_GRAPHICS_SPRITELIMIT, "Sprite Limit");
	
	InsertMenu(GraphicsMenu, 8, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 9, flags, ID_GRAPHICS_FRAMESKIP, "Frame Skip");
	
	InsertMenu(GraphicsMenu, 10, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(GraphicsMenu, 11, flags, ID_GRAPHICS_SCREENSHOT, "Screen Shot");
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
	DestroyMenu(CPUMenu);
	CPUMenu = CreatePopupMenu();
	RemoveMenu(parent, position, MF_BYPOSITION);
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, CPUMenu, "&CPU");
	
#ifdef GENS_DEBUGGER
	InsertMenu(CPUMenu, 0, flags, ID_CPU_DEBUG, "&Debug");
	InsertMenu(CPUMenu, 1, MF_SEPARATOR, NULL, NULL);
#endif /* GENS_DEBUGGER */
	
	InsertMenu(CPUMenu, 2, flags, ID_CPU_COUNTRY, "&Country");
	
	InsertMenu(CPUMenu, 3, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(CPUMenu, 4, flags, ID_CPU_HARDRESET, "Hard &Reset");
	InsertMenu(CPUMenu, 5, flags, ID_CPU_RESET68K, "Reset 68000");
	InsertMenu(CPUMenu, 6, flags, ID_CPU_RESETMAIN68K, "Reset Main 68000");
	InsertMenu(CPUMenu, 7, flags, ID_CPU_RESETSUB68K, "Reset Sub 68000");
	InsertMenu(CPUMenu, 8, flags, ID_CPU_RESETMAINSH2, "Reset Main SH2");
	InsertMenu(CPUMenu, 9, flags, ID_CPU_RESETSUBSH2, "Reset Sub SH2");
	InsertMenu(CPUMenu, 10, flags, ID_CPU_RESETZ80, "Reset Z80");
	
	InsertMenu(CPUMenu, 11, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(CPUMenu, 12, flags, ID_CPU_SEGACDPERFECTSYNC, "SegaCD Perfect Sync (SLOW)");
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
	DestroyMenu(SoundMenu);
	SoundMenu = CreatePopupMenu();
	RemoveMenu(parent, position, MF_BYPOSITION);
	InsertMenu(parent, 3, MF_BYPOSITION | MF_POPUP | MF_STRING, SoundMenu, "&Sound");
	
	InsertMenu(SoundMenu, 0, flags, ID_SOUND_ENABLE, "&Enable");
	
	InsertMenu(SoundMenu, 1, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(SoundMenu, 2, flags, ID_SOUND_RATE, "&Rate");
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
 * create_gens_window_OptionsMenu(): Create the Options menu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
static void create_gens_window_OptionsMenu(HMENU parent, int position)
{
	unsigned int flags = MF_BYPOSITION | MF_STRING;
	
	// Options
	DestroyMenu(OptionsMenu);
	OptionsMenu = CreatePopupMenu();
	RemoveMenu(parent, position, MF_BYPOSITION);
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, OptionsMenu, "&Options");
	
	InsertMenu(OptionsMenu, 0, flags, ID_OPTIONS_GENERAL, "&General Options...");
	InsertMenu(OptionsMenu, 1, flags, ID_OPTIONS_JOYPADS, "&Joypads...");
	InsertMenu(OptionsMenu, 2, flags, ID_OPTIONS_DIRECTORIES, "&Directories...");
	InsertMenu(OptionsMenu, 3, flags, ID_OPTIONS_BIOS_MISC_FILES, "&BIOS/Misc Files...");
	
	InsertMenu(OptionsMenu, 4, MF_SEPARATOR, NULL, NULL);
	
#ifdef GENS_CDROM
	InsertMenu(OptionsMenu, 5, flags, ID_OPTIONS_CURRENT_CD_DRIVE, "Current &CD Drive...");
#endif /* GENS_CDROM */
	InsertMenu(OptionsMenu, 6, flags, ID_OPTIONS_SEGACD_SRAM_SIZE, "SegaCD S&RAM Size");
	
	InsertMenu(OptionsMenu, 7, MF_SEPARATOR, NULL, NULL);
	
	InsertMenu(OptionsMenu, 8, flags, ID_OPTIONS_LOADCONFIG, "&Load Config...");
	InsertMenu(OptionsMenu, 9, flags, ID_OPTIONS_SAVECONFIGAS, "&Save Config As...");
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
	DestroyMenu(HelpMenu);
	HelpMenu = CreatePopupMenu();
	RemoveMenu(parent, position, MF_BYPOSITION);
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, HelpMenu, "&Help");
	
	InsertMenu(HelpMenu, 0, flags, ID_HELP_ABOUT, "&About");
}


#if 0
/**
 * create_gens_window_GraphicsMenu_FrameSkip_SubMenu(): Create the Graphics, Frame Skip submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu_FrameSkip_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *FSItem;
	GSList *FSGroup = NULL;
	
	int i;
	char FSName[8];
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_FrameSkip_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the frame skip entries.
	for (i = -1; i <= 8; i++)
	{
		if (i >= 0)
			sprintf(FSName, "%d", i);
		else
			strcpy(FSName, "Auto");
		sprintf(ObjName, "GraphicsMenu_FrameSkip_SubMenu_%s", FSName);
		NewMenuItem_Radio(FSItem, FSName, ObjName, SubMenu, (i == -1 ? TRUE : FALSE), FSGroup);
		g_signal_connect((gpointer)FSItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_FrameSkip_SubMenu_FSItem_activate),
				 GINT_TO_POINTER(i));
	}
}


#ifdef GENS_DEBUGGER
/**
 * create_gens_window_CPUMenu_Debug_SubMenu(): Create the CPU, Debug submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_CPUMenu_Debug_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	
	// TODO: Move this array somewhere else.
	const char* DebugStr[9] =
	{
		"_Genesis - 68000",
		"Genesis - _Z80",
		"Genesis - _VDP",
		"_SegaCD - 68000",
		"SegaCD - _CDC",
		"SegaCD - GF_X",
		"32X - Main SH2",
		"32X - Sub SH2",
		"32X - VDP",
	};
	
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "CPUMenu_Debug_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the render entries.
	for (i = 0; i < 9; i++)
	{
		sprintf(ObjName, "CPUMenu_Debug_SubMenu_%d", i + 1);
		NewMenuItem_Check(debugMenuItems[i], DebugStr[i], ObjName, SubMenu, FALSE);
		g_signal_connect((gpointer)debugMenuItems[i], "activate",
				 G_CALLBACK(on_CPUMenu_Debug_SubMenu_activate),
				 GINT_TO_POINTER(i + 1));
		if (i % 3 == 2 && i < 6)
		{
			// Every three entires, add a separator.
			sprintf(ObjName, "CPUMenu_Debug_SubMenu_Sep%d", (i / 3) + 1);
			NewMenuSeparator(debugSeparators[i / 3], ObjName, SubMenu);
		}
	}
}
#endif /* GENS_DEBUGGER */


/**
 * create_gens_window_CPUMenu_Country_SubMenu(): Create the CPU, Country submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_CPUMenu_Country_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *CountryItem;
	GSList *CountryGroup = NULL;
	GtkWidget *CPUMenu_Country_SubMenu_Separator;
	GtkWidget *CPUMenu_Country_SubMenu_AutoDetectOrder;
	
	// TODO: Move this array somewhere else.
	const char* CountryCodes[5] =
	{
		"Auto Detect",
		"Japan (NTSC)",
		"USA (NTSC)",
		"Europe (PAL)",
		"Japan (PAL)",
	};
	
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "CPUMenu_Country_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the bits per pixel entries.
	for (i = 0; i < 5; i++)
	{
		if (i == 0)
			strcpy(ObjName, "CPUMenu_Country_SubMenu_Auto");
		else
			sprintf(ObjName, "CPUMenu_Country_SubMenu_%d", i - 1);
		NewMenuItem_Radio(CountryItem, CountryCodes[i], ObjName, SubMenu, (i == 0 ? TRUE : FALSE), CountryGroup);
		g_signal_connect((gpointer)CountryItem, "activate",
				 G_CALLBACK(on_CPUMenu_Country_activate), GINT_TO_POINTER(i - 1));
	}
	
	// Separator
	NewMenuSeparator(CPUMenu_Country_SubMenu_Separator, "CPUMenu_Country_SubMenu_Separator", SubMenu);
	
	// Add the Auto-Detection Order configuration option.
	NewMenuItem(CPUMenu_Country_SubMenu_AutoDetectOrder, "Auto-Detection Order...", "Auto-Detection Order...", SubMenu);
	AddMenuCallback(CPUMenu_Country_SubMenu_AutoDetectOrder, on_CPUMenu_Country_SubMenu_AutoDetectOrder_activate);	
}


/**
 * create_gens_window_SoundMenu_Rate_SubMenu(): Create the Sound, Rate submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_SoundMenu_Rate_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *SndItem;
	GSList *SndGroup = NULL;
	
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
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "SoundMenu_Rate_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the rate entries.
	for (i = 0; i < 6; i++)
	{
		sprintf(SndName, "%d Hz", SndRates[i][1]);
		sprintf(ObjName, "SoundMenu_Rate_SubMenu_%d", SndRates[i][1]);
		NewMenuItem_Radio(SndItem, SndName, ObjName, SubMenu, (SndRates[i][1] == 22050 ? TRUE : FALSE), SndGroup);
		g_signal_connect((gpointer)SndItem, "activate",
				 G_CALLBACK(on_SoundMenu_Rate_SubMenu_activate),
				 GINT_TO_POINTER(SndRates[i][0]));
	}
}


/**
 * create_gens_window_SoundMenu_Rate_SubMenu(): Create the Options, Sega CD SRAM Size submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *SRAMItem;
	GSList *SRAMGroup = NULL;
	
	int i;
	char SRAMName[16];
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "OptionsMenu_SegaCDSRAMSize_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the rate entries.
	for (i = -1; i <= 3; i++)
	{
		if (i == -1)
		{
			strcpy(SRAMName, "None");
			strcpy(ObjName, "OptionsMenu_SegaCDSRAMSize_SubMenu_None");
		}
		else
		{
			sprintf(SRAMName, "%d KB", 8 << i);	
			sprintf(ObjName, "OptionsMenu_SegaCDSRAMSize_SubMenu_%d", i);
		}
		NewMenuItem_Radio(SRAMItem, SRAMName, ObjName, SubMenu, (i == -1 ? TRUE : FALSE), SRAMGroup);
		g_signal_connect((gpointer)SRAMItem, "activate",
				 G_CALLBACK(on_OptionsMenu_SegaCDSRAMSize_SubMenu_activate),
				 GINT_TO_POINTER(i));
	}
}
#endif
