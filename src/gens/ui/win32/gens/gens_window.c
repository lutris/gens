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


// Menu objects
HMENU MainMenu;
HMENU FileMenu;
HMENU GraphicsMenu;
HMENU CPUMenu;
HMENU SoundMenu;
HMENU OptionsMenu;
HMENU HelpMenu;


static void create_gens_window_menubar(void);
static void create_gens_window_FileMenu(HMENU parent);
#if 0
static void create_gens_window_FileMenu_ChangeState_SubMenu(GtkWidget *container);
static void create_gens_window_GraphicsMenu(GtkWidget *container);
#ifdef GENS_OPENGL
static void create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(GtkWidget *container);
#endif
static void create_gens_window_GraphicsMenu_bpp_SubMenu(GtkWidget *container);
static void create_gens_window_GraphicsMenu_FrameSkip_SubMenu(GtkWidget *container);
static void create_gens_window_CPUMenu(GtkWidget *container);
#ifdef GENS_DEBUGGER
static void create_gens_window_CPUMenu_Debug_SubMenu(GtkWidget *container);
#endif /* GENS_DEBUGGER */
static void create_gens_window_CPUMenu_Country_SubMenu(GtkWidget *container);
static void create_gens_window_SoundMenu(GtkWidget *container);
static void create_gens_window_SoundMenu_Rate_SubMenu(GtkWidget *container);
static void create_gens_window_OptionsMenu(GtkWidget *container);
static void create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(GtkWidget *container);
static void create_gens_window_HelpMenu(GtkWidget *container);
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
	if (!Gens_hWnd)
		return NULL;
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
	create_gens_window_FileMenu(MainMenu);
	/*
	create_gens_window_GraphicsMenu(MenuBar);
	create_gens_window_CPUMenu(MenuBar);
	create_gens_window_SoundMenu(MenuBar);
	create_gens_window_OptionsMenu(MenuBar);
	create_gens_window_HelpMenu(MenuBar);
	*/
}


/**
 * create_gens_window_FileMenu(): Create the File menu.
 * @param parent Parent menu.
 */
static void create_gens_window_FileMenu(HMENU parent)
{
	unsigned int flags;
	int i = 0;
	
	// File
	FileMenu = CreatePopupMenu();
	InsertMenu(parent, 0, MF_BYPOSITION | MF_POPUP | MF_STRING, FileMenu, "&File");
	
	flags = MF_BYPOSITION | MF_STRING;
	
	InsertMenu(FileMenu, i++, flags, /*ID_MENU_OPENROM*/ 0, "&Open ROM...");
#ifdef GENS_CDROM
	InsertMenu(FileMenu, i++, flags, /*ID_MENU_BOOTCD*/ 0, "&Boot CD");
#endif /* GENS_CDROM */
	InsertMenu(FileMenu, i++, flags, /*ID_MENU_NETPLAY*/ 0, "&Netplay");
	InsertMenu(FileMenu, i++, flags, /*ID_MENU_ROMHISTORY*/ 0, "ROM &History");
	InsertMenu(FileMenu, i++, flags, /*ID_MENU_CLOSEROM*/ 0, "&Close ROM");
	
	InsertMenu(FileMenu, i++, MF_SEPARATOR, NULL, NULL);
	
#if 0
	// Game Genie
	NewMenuItem_Icon(FileMenu_GameGenie, "_Game Genie", "FileMenu_GameGenie", FileMenu,
			 FileMenu_GameGenie_Icon, "password.png");
	AddMenuCallback(FileMenu_GameGenie, on_FileMenu_GameGenie_activate);
	
	// Separator
	NewMenuSeparator(FileMenu_Separator2, "FileMenu_Separator2", FileMenu);
	
	// Load State...
	NewMenuItem_StockIcon(FileMenu_LoadState, "_Load State...", "FileMenu_LoadState", FileMenu,
			      FileMenu_LoadState_Icon, "gtk-open");
	AddMenuAccelerator(FileMenu_LoadState, GDK_F8, GDK_SHIFT_MASK);
	AddMenuCallback(FileMenu_LoadState, on_FileMenu_LoadState_activate);
	
	// Save State As...
	NewMenuItem_StockIcon(FileMenu_SaveState, "_Save State As...", "FileMenu_SaveState", FileMenu,
			      FileMenu_SaveState_Icon, "gtk-save-as");
	AddMenuAccelerator(FileMenu_SaveState, GDK_F5, GDK_SHIFT_MASK);
	AddMenuCallback(FileMenu_SaveState, on_FileMenu_SaveState_activate);
	
	// Quick Load
	NewMenuItem_StockIcon(FileMenu_QuickLoad, "Quick Load", "FileMenu_QuickLoad", FileMenu,
			      FileMenu_QuickLoad_Icon, "gtk-refresh");
	AddMenuAccelerator(FileMenu_QuickLoad, GDK_F8, 0);
	AddMenuCallback(FileMenu_QuickLoad, on_FileMenu_QuickLoad_activate);
	
	// Quick Save
	NewMenuItem_StockIcon(FileMenu_QuickSave, "Quick Save", "FileMenu_QuickSave", FileMenu,
			      FileMenu_QuickSave_Icon, "gtk-save");
	AddMenuAccelerator(FileMenu_QuickSave, GDK_F5, 0);
	AddMenuCallback(FileMenu_QuickSave, on_FileMenu_QuickSave_activate);
	
	// Change State
	NewMenuItem_StockIcon(FileMenu_ChangeState, "Change State", "FileMenu_ChangeState", FileMenu,
			      FileMenu_ChangeState_Icon, "gtk-revert-to-saved");
	// Change State submenu
	create_gens_window_FileMenu_ChangeState_SubMenu(FileMenu_ChangeState);
	
	// Quit
	NewMenuItem_StockIcon(FileMenu_Quit, "_Quit", "FileMenu_Quit", FileMenu,
			      FileMenu_Quit_Icon, "gtk-quit");
	AddMenuAccelerator(FileMenu_Quit, GDK_Q, GDK_CONTROL_MASK);
	AddMenuCallback(FileMenu_Quit, on_FileMenu_Quit_activate);
#endif
}


#if 0
/**
 * gens_window_FileMenu_ChangeState_SubMenu(): Create the File, Change State submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_FileMenu_ChangeState_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *SlotItem;
	GSList *SlotGroup = NULL;
	
	int i;
	char ObjName[64];
	char SlotName[8];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "FileMenu_ChangeState_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the save slot entries.
	for (i = 0; i < 10; i++)
	{
		sprintf(SlotName, "%d", i);
		sprintf(ObjName, "FileMenu_ChangeState_SubMenu_%s", SlotName);
		NewMenuItem_Radio(SlotItem, SlotName, ObjName, SubMenu, (i == 0 ? TRUE : FALSE), SlotGroup);
		g_signal_connect((gpointer)SlotItem, "activate",
				 G_CALLBACK(on_FileMenu_ChangeState_SubMenu_SlotItem_activate),
				 GINT_TO_POINTER(i));
	}
}


/**
 * create_gens_window_GraphicsMenu(): Create the Graphics menu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu(GtkWidget *container)
{
	GtkWidget *Graphics;			GtkWidget *Graphics_Icon;
	GtkWidget *GraphicsMenu;
	GtkWidget *GraphicsMenu_FullScreen;	GtkWidget *GraphicsMenu_FullScreen_Icon;
	GtkWidget *GraphicsMenu_VSync;
	GtkWidget *GraphicsMenu_Stretch;
	GtkWidget *GraphicsMenu_Separator1;
#ifdef GENS_OPENGL
	GtkWidget *GraphicsMenu_OpenGL;
	GtkWidget *GraphicsMenu_OpenGLFilter;
	GtkWidget *GraphicsMenu_OpenGLRes;
#endif
	GtkWidget *GraphicsMenu_bpp;
	GtkWidget *GraphicsMenu_Separator2;
	GtkWidget *GraphicsMenu_ColorAdjust;	GtkWidget *GraphicsMenu_ColorAdjust_Icon;
	GtkWidget *GraphicsMenu_Render;		GtkWidget *GraphicsMenu_Render_Icon;
	GtkWidget *GraphicsMenu_Separator3;
	GtkWidget *GraphicsMenu_SpriteLimit;
	GtkWidget *GraphicsMenu_Separator4;
	GtkWidget *GraphicsMenu_FrameSkip;	GtkWidget *GraphicsMenu_FrameSkip_Icon;
	GtkWidget *GraphicsMenu_Separator5;
	GtkWidget *GraphicsMenu_ScreenShot;	GtkWidget *GraphicsMenu_ScreenShot_Icon;
	
	// Graphics
	NewMenuItem_Icon(Graphics, "_Graphics", "Graphics", container, Graphics_Icon, "xpaint.png");
	
	// Menu object for the GraphicsMenu
	GraphicsMenu = gtk_menu_new();
	gtk_widget_set_name(GraphicsMenu, "GraphicsMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Graphics), GraphicsMenu);
	
	// Full Screen
	NewMenuItem_StockIcon(GraphicsMenu_FullScreen, "_Full Screen", "GraphicsMenu_FullScreen", GraphicsMenu,
			      GraphicsMenu_FullScreen_Icon, "gtk-fullscreen");
	AddMenuAccelerator(GraphicsMenu_FullScreen, GDK_Return, GDK_MOD1_MASK);
	AddMenuCallback(GraphicsMenu_FullScreen, on_GraphicsMenu_FullScreen_activate);
	
	// VSync
	NewMenuItem_Check(GraphicsMenu_VSync, "_VSync", "GraphicsMenu_VSync", GraphicsMenu, FALSE);
	AddMenuAccelerator(GraphicsMenu_VSync, GDK_F3, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_VSync, on_GraphicsMenu_VSync_activate);
	
	// Stretch
	NewMenuItem_Check(GraphicsMenu_Stretch, "_Stretch", "GraphicsMenu_Stretch", GraphicsMenu, FALSE);
	AddMenuAccelerator(GraphicsMenu_Stretch, GDK_F2, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_Stretch, on_GraphicsMenu_Stretch_activate);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator1, "GraphicsMenu_Separator1", GraphicsMenu);
	
#ifdef GENS_OPENGL
	// OpenGL
	NewMenuItem_Check(GraphicsMenu_OpenGL, "Open_GL", "GraphicsMenu_OpenGL", GraphicsMenu, FALSE);
	AddMenuAccelerator(GraphicsMenu_OpenGL, GDK_r, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_OpenGL, on_GraphicsMenu_OpenGL_activate);
	
	// OpenGL Linear Filter
	NewMenuItem_Check(GraphicsMenu_OpenGLFilter, "OpenGL _Linear Filter", "GraphicsMenu_OpenGLFilter", GraphicsMenu, FALSE);
	AddMenuCallback(GraphicsMenu_OpenGLFilter, on_GraphicsMenu_OpenGLFilter_activate);
	
	// OpenGL Resolution
	NewMenuItem(GraphicsMenu_OpenGLRes, "OpenGL Resolution", "GraphicsMenu_OpenGLRes", GraphicsMenu);
	// OpenGL Resolution submenu
	create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(GraphicsMenu_OpenGLRes);
#endif
	
	// Bits per pixel (OpenGL mode) [TODO: Where is this value actually used?]
	NewMenuItem(GraphicsMenu_bpp, "Bits per pixel", "GraphicsMenu_bpp", GraphicsMenu);
	// Bits per pixel submenu
	create_gens_window_GraphicsMenu_bpp_SubMenu(GraphicsMenu_bpp);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator2, "GraphicsMenu_Separator2", GraphicsMenu);
	
	// Color Adjust
	NewMenuItem_StockIcon(GraphicsMenu_ColorAdjust, "Color Adjust...", "GraphicsMenu_ColorAdjust", GraphicsMenu,
			      GraphicsMenu_ColorAdjust_Icon, "gtk-select-color");
	AddMenuCallback(GraphicsMenu_ColorAdjust, on_GraphicsMenu_ColorAdjust_activate);
	
	// Render
	NewMenuItem_Icon(GraphicsMenu_Render, "_Render", "GraphicsMenu_Render", GraphicsMenu,
			 GraphicsMenu_Render_Icon, "viewmag.png");
	// Render submenu
	Sync_Gens_Window_GraphicsMenu_Render_SubMenu(GraphicsMenu_Render);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator3, "GraphicsMenu_Separator3", GraphicsMenu);
	
	// Sprite Limit
	NewMenuItem_Check(GraphicsMenu_SpriteLimit, "Sprite Limit", "GraphicsMenu_SpriteLimit", GraphicsMenu, TRUE);
	AddMenuCallback(GraphicsMenu_SpriteLimit, on_GraphicsMenu_SpriteLimit_activate);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator4, "GraphicsMenu_Separator4", GraphicsMenu);
	
	// Frame Skip
	NewMenuItem_Icon(GraphicsMenu_FrameSkip, "Frame Skip", "GraphicsMenu_FrameSkip", GraphicsMenu,
			 GraphicsMenu_FrameSkip_Icon, "2rightarrow.png");
	// Frame Skip submenu
	create_gens_window_GraphicsMenu_FrameSkip_SubMenu(GraphicsMenu_FrameSkip);
	
	// Separator
	NewMenuSeparator(GraphicsMenu_Separator5, "GraphicsMenu_Separator5", GraphicsMenu);
	
	// Screen Shot
	NewMenuItem_StockIcon(GraphicsMenu_ScreenShot, "Screen Shot", "GraphicsMenu_ScreenShot", GraphicsMenu,
			      GraphicsMenu_ScreenShot_Icon, "gtk-copy");
	AddMenuAccelerator(GraphicsMenu_ScreenShot, GDK_BackSpace, GDK_SHIFT_MASK);
	AddMenuCallback(GraphicsMenu_ScreenShot, on_GraphicsMenu_ScreenShot_activate);
}


#ifdef GENS_OPENGL
/**
 * create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(): Create the Graphics, OpenGL Resolution submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu_OpenGLRes_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *ResItem;
	GSList *ResGroup = NULL;
	int resValue;	// 0xWWWWHHHH
	
	// TODO: Move this array somewhere else.
	int resolutions[5][2] =
	{
		{320, 240},
		{640, 480},
		{800, 600},
		{1024, 768},
		{-1, -1}, // Custom
	};
	
	int i;
	char ObjName[64];
	char ResName[16];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_OpenGLRes_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the resolution entries.
	for (i = 0; i < 5; i++)
	{
		if (resolutions[i][0] > 0)
			sprintf(ResName, "%dx%d", resolutions[i][0], resolutions[i][1]);
		else
			strcpy(ResName, "Custom");
		
		sprintf(ObjName, "GraphicsMenu_OpenGLRes_SubMenu_%s", ResName);
		NewMenuItem_Radio(ResItem, ResName, ObjName, SubMenu, (i == 0 ? TRUE : FALSE), ResGroup);
		resValue = (resolutions[i][0] == -1 ? 0 : (resolutions[i][0] << 16 | resolutions[i][1]));
		g_signal_connect((gpointer)ResItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_OpenGLRes_SubMenu_ResItem_activate),
				 GINT_TO_POINTER(resValue));
	}
}
#endif


/**
 * create_gens_window_GraphicsMenu_bpp_SubMenu(): Create the Graphics, Bits per pixel submenu.
 * @param container Container for this menu.
 */
static void create_gens_window_GraphicsMenu_bpp_SubMenu(GtkWidget *container)
{
	GtkWidget *SubMenu;
	GtkWidget *bppItem;
	GSList *bppGroup = NULL;
	
	// TODO: Move this array somewhere else.
	int bpp[3] = {15, 16, 32};
	const char* bppStr[3] = {"15 (555)", "16 (565)", "32"};
	int i;
	char ObjName[64];
	
	// Create the submenu.
	SubMenu = gtk_menu_new();
	gtk_widget_set_name(SubMenu, "GraphicsMenu_bpp_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), SubMenu);
	
	// Create the bits per pixel entries.
	for (i = 0; i < 3; i++)
	{
		sprintf(ObjName, "GraphicsMenu_bpp_SubMenu_%d", bpp[i]);
		NewMenuItem_Radio(bppItem, bppStr[i], ObjName, SubMenu, (i == 1 ? TRUE : FALSE), bppGroup);
		g_signal_connect((gpointer)bppItem, "activate",
				 G_CALLBACK(on_GraphicsMenu_bpp_SubMenu_bppItem_activate),
				 GINT_TO_POINTER(bpp[i]));
	}
}


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


/**
 * create_gens_window_CPUMenu(): Create the CPU menu.
 * @param container Container for this menu.
 */
static void create_gens_window_CPUMenu(GtkWidget *container)
{
	GtkWidget *CPU;				GtkWidget *CPU_Icon;
	GtkWidget *CPUMenu;
#ifdef GENS_DEBUGGER
	GtkWidget *CPUMenu_Debug;
	GtkWidget *CPUMenu_Separator1;
#endif /* GENS_DEBUGGER */
	GtkWidget *CPUMenu_Country;
	GtkWidget *CPUMenu_Separator2;
	GtkWidget *CPUMenu_HardReset;		GtkWidget *CPUMenu_HardReset_Icon;
	GtkWidget *CPUMenu_Reset68000;
	GtkWidget *CPUMenu_ResetMain68000;
	GtkWidget *CPUMenu_ResetSub68000;
	GtkWidget *CPUMenu_ResetMainSH2;
	GtkWidget *CPUMenu_ResetSubSH2;
	GtkWidget *CPUMenu_ResetZ80;
	GtkWidget *CPUMenu_Separator3;
	GtkWidget *CPUMenu_SegaCD_PerfectSync;
	
	// CPU
	NewMenuItem_Icon(CPU, "_CPU", "CPU", container, CPU_Icon, "memory.png");
	
	// Menu object for the CPUMenu
	CPUMenu = gtk_menu_new();
	gtk_widget_set_name(CPUMenu, "CPUMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(CPU), CPUMenu);
	
#ifdef GENS_DEBUGGER
	// Debug
	NewMenuItem(CPUMenu_Debug, "_Debug", "CPUMenu_Debug", CPUMenu);
	// Debug submenu
	create_gens_window_CPUMenu_Debug_SubMenu(CPUMenu_Debug);
	
	// Separator
	NewMenuSeparator(CPUMenu_Separator1, "CPUMenu_Separator1", CPUMenu);
#endif /* GENS_DEBUGGER */
	
	// Country
	NewMenuItem(CPUMenu_Country, "_Country", "CPUMenu_Country", CPUMenu);
	// Country submenu
	create_gens_window_CPUMenu_Country_SubMenu(CPUMenu_Country);
	
	// Separator
	NewMenuSeparator(CPUMenu_Separator2, "CPUMenu_Separator2", CPUMenu);
	
	// Hard Reset
	NewMenuItem_StockIcon(CPUMenu_HardReset, "Hard _Reset", "CPUMenu_HardReset", CPUMenu,
			      CPUMenu_HardReset_Icon, "gtk-refresh");
	// Tab doesn't seem to work here...
	// Leaving this line in anyway so the accelerator is displayed in the menu.
	AddMenuAccelerator(CPUMenu_HardReset, GDK_Tab, 0);
	AddMenuCallback(CPUMenu_HardReset, on_CPUMenu_HardReset_activate);
	
	// Reset 68000 (same as Reset Main 68000, but shows up if the Sega CD isn't enabled.)
	NewMenuItem(CPUMenu_Reset68000, "Reset 68000", "CPUMenu_Reset68000", CPUMenu);
	AddMenuCallback(CPUMenu_Reset68000, on_CPUMenu_ResetMain68000_activate);
	
	// Reset Main 68000
	NewMenuItem(CPUMenu_ResetMain68000, "Reset Main 68000", "CPUMenu_ResetMain68000", CPUMenu);
	AddMenuCallback(CPUMenu_ResetMain68000, on_CPUMenu_ResetMain68000_activate);

	// Reset Sub 68000
	NewMenuItem(CPUMenu_ResetSub68000, "Reset Sub 68000", "CPUMenu_ResetSub68000", CPUMenu);
	AddMenuCallback(CPUMenu_ResetSub68000, on_CPUMenu_ResetSub68000_activate);
	
	// Reset Main SH2
	NewMenuItem(CPUMenu_ResetMainSH2, "Reset Main SH2", "CPUMenu_ResetMainSH2", CPUMenu);
	AddMenuCallback(CPUMenu_ResetMainSH2, on_CPUMenu_ResetMainSH2_activate);
	
	// Reset Sub SH2
	NewMenuItem(CPUMenu_ResetSubSH2, "Reset Sub SH2", "CPUMenu_ResetSubSH2", CPUMenu);
	AddMenuCallback(CPUMenu_ResetSubSH2, on_CPUMenu_ResetSubSH2_activate);
	
	// Reset Z80
	NewMenuItem(CPUMenu_ResetZ80, "Reset Z80", "CPUMenu_ResetZ80", CPUMenu);
	AddMenuCallback(CPUMenu_ResetZ80, on_CPUMenu_ResetZ80_activate);
	
	// Separator
	NewMenuSeparator(CPUMenu_Separator3, "CPUMenu_Separator3", CPUMenu);
	
	// SegaCD Perfect Sync
	NewMenuItem_Check(CPUMenu_SegaCD_PerfectSync, "SegaCD Perfect Sync (SLOW)", "CPUMenu_SegaCD_PerfectSync", CPUMenu, FALSE);
	AddMenuCallback(CPUMenu_SegaCD_PerfectSync, on_CPUMenu_SegaCD_PerfectSync_activate);
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
 * create_gens_window_SoundMenu(): Create the Sound menu.
 * @param container Container for this menu.
 */
static void create_gens_window_SoundMenu(GtkWidget *container)
{
	GtkWidget *Sound;			GtkWidget *Sound_Icon;
	GtkWidget *SoundMenu;
	GtkWidget *SoundMenu_Enable;
	GtkWidget *SoundMenu_Separator1;
	GtkWidget *SoundMenu_Rate;
	GtkWidget *SoundMenu_Stereo;
	GtkWidget *SoundMenu_Separator2;
	GtkWidget *SoundMenu_Z80;
	GtkWidget *SoundMenu_Separator3;
	GtkWidget *SoundMenu_YM2612;
	GtkWidget *SoundMenu_YM2612_Improved;
	GtkWidget *SoundMenu_DAC;
	GtkWidget *SoundMenu_DAC_Improved;
	GtkWidget *SoundMenu_PSG;
	GtkWidget *SoundMenu_PSG_Improved;
	GtkWidget *SoundMenu_PCM;
	GtkWidget *SoundMenu_PWM;
	GtkWidget *SoundMenu_CDDA;
	GtkWidget *SoundMenu_Separator4;
	GtkWidget *SoundMenu_WAVDump;
	GtkWidget *SoundMenu_GYMDump;
	
	// Sound
	NewMenuItem_Icon(Sound, "_Sound", "Sound", container, Sound_Icon, "kmix.png");
	
	// Menu object for the SoundMenu
	SoundMenu = gtk_menu_new();
	gtk_widget_set_name(SoundMenu, "SoundMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Sound), SoundMenu);
	
	// Enable
	NewMenuItem_Check(SoundMenu_Enable, "_Enable", "SoundMenu_Enable", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_Enable, on_SoundMenu_Enable_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator1, "SoundMenu_Separator1", SoundMenu);

	// Rate
	NewMenuItem(SoundMenu_Rate, "_Rate", "SoundMenu_Rate", SoundMenu);
	// Rate submenu
	create_gens_window_SoundMenu_Rate_SubMenu(SoundMenu_Rate);
	
	// Stereo
	NewMenuItem_Check(SoundMenu_Stereo, "_Stereo", "SoundMenu_Stereo", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_Stereo, on_SoundMenu_Stereo_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator2, "SoundMenu_Separator2", SoundMenu);
	
	// Z80
	NewMenuItem_Check(SoundMenu_Z80, "_Z80", "SoundMenu_Z80", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_Z80, on_SoundMenu_Z80_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator3, "SoundMenu_Separator3", SoundMenu);
	
	// YM2612
	NewMenuItem_Check(SoundMenu_YM2612, "_YM2612", "SoundMenu_YM2612", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_YM2612, on_SoundMenu_YM2612_activate);
	
	// YM2612 Improved
	NewMenuItem_Check(SoundMenu_YM2612_Improved, "YM2612 Improved", "SoundMenu_YM2612_Improved", SoundMenu, FALSE);
	AddMenuCallback(SoundMenu_YM2612_Improved, on_SoundMenu_YM2612_Improved_activate);
	
	// DAC
	NewMenuItem_Check(SoundMenu_DAC, "_DAC", "SoundMenu_DAC", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_DAC, on_SoundMenu_DAC_activate);
	
	// DAC Improved
	NewMenuItem_Check(SoundMenu_DAC_Improved, "DAC Improved", "SoundMenu_DAC_Improved", SoundMenu, FALSE);
	AddMenuCallback(SoundMenu_DAC_Improved, on_SoundMenu_DAC_Improved_activate);
	
	// PSG
	NewMenuItem_Check(SoundMenu_PSG, "_PSG", "SoundMenu_PSG", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_PSG, on_SoundMenu_PSG_activate);
	
	// PSG Improved
	NewMenuItem_Check(SoundMenu_PSG_Improved, "PSG Improved", "SoundMenu_PSG_Improved", SoundMenu, FALSE);
	AddMenuCallback(SoundMenu_PSG_Improved, on_SoundMenu_PSG_Improved_activate);
	
	// PCM
	NewMenuItem_Check(SoundMenu_PCM, "P_CM", "SoundMenu_PCM", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_PCM, on_SoundMenu_PCM_activate);
	
	// PWM
	NewMenuItem_Check(SoundMenu_PWM, "P_WM", "SoundMenu_PWM", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_PWM, on_SoundMenu_PWM_activate);
	
	// CDDA
	NewMenuItem_Check(SoundMenu_CDDA, "CDD_A (CD Audio)", "SoundMenu_CDDA", SoundMenu, TRUE);
	AddMenuCallback(SoundMenu_CDDA, on_SoundMenu_CDDA_activate);
	
	// Separator
	NewMenuSeparator(SoundMenu_Separator4, "SoundMenu_Separator4", SoundMenu);
	
	// WAV Dump
	NewMenuItem(SoundMenu_WAVDump, "Start WAV Dump", "SoundMenu_WAVDump", SoundMenu);
	AddMenuCallback(SoundMenu_WAVDump, on_SoundMenu_WAVDump_activate);
	
	// GYM Dump
	NewMenuItem(SoundMenu_GYMDump, "Start GYM Dump", "SoundMenu_GYMDump", SoundMenu);
	AddMenuCallback(SoundMenu_GYMDump, on_SoundMenu_GYMDump_activate);
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
 * create_gens_window_OptionsMenu(): Create the Options menu.
 * @param container Container for this menu.
 */
static void create_gens_window_OptionsMenu(GtkWidget *container)
{
	GtkWidget *Options;			GtkWidget *Options_Icon;
	GtkWidget *OptionsMenu;
	GtkWidget *OptionsMenu_GeneralOptions;	GtkWidget *OptionsMenu_GeneralOptions_Icon;
	GtkWidget *OptionsMenu_Joypads;		GtkWidget *OptionsMenu_Joypads_Icon;
	GtkWidget *OptionsMenu_Directories;	GtkWidget *OptionsMenu_Directories_Icon;
	GtkWidget *OptionsMenu_BIOSMiscFiles;	GtkWidget *OptionsMenu_BIOSMiscFiles_Icon;
	GtkWidget *OptionsMenu_Separator1;
#ifdef GENS_CDROM
	GtkWidget *OptionsMenu_CurrentCDDrive;	GtkWidget *OptionsMenu_CurrentCDDrive_Icon;
#endif
	GtkWidget *OptionsMenu_SegaCDSRAMSize;	GtkWidget *OptionsMenu_SegaCDSRAMSize_Icon;
	GtkWidget *OptionsMenu_Separator2;
	GtkWidget *OptionsMenu_LoadConfig;	GtkWidget *OptionsMenu_LoadConfig_Icon;
	GtkWidget *OptionsMenu_SaveConfigAs;	GtkWidget *OptionsMenu_SaveConfigAs_Icon;
	
	// Options
	NewMenuItem_Icon(Options, "_Options", "Options", container, Options_Icon, "package_settings.png");
	
	// Menu object for the OptionsMenu
	OptionsMenu = gtk_menu_new();
	gtk_widget_set_name(OptionsMenu, "OptionsMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Options), OptionsMenu);
	
	// General Options
	NewMenuItem_Icon(OptionsMenu_GeneralOptions, "_General Options...", "OptionsMenu_GeneralOptions", OptionsMenu,
			 OptionsMenu_GeneralOptions_Icon, "ksysguard.png");
	AddMenuCallback(OptionsMenu_GeneralOptions, on_OptionsMenu_GeneralOptions_activate);
	
	// Joypads
	NewMenuItem_Icon(OptionsMenu_Joypads, "_Joypads...", "OptionsMenu_Joypads", OptionsMenu,
			 OptionsMenu_Joypads_Icon, "package_games.png");
	AddMenuCallback(OptionsMenu_Joypads, on_OptionsMenu_Joypads_activate);
	
	// Directories
	NewMenuItem_Icon(OptionsMenu_Directories, "_Directories...", "OptionsMenu_Directories", OptionsMenu,
			 OptionsMenu_Directories_Icon, "folder_slin_open.png");
	AddMenuCallback(OptionsMenu_Directories, on_OptionsMenu_Directories_activate);
	
	// BIOS/Misc Files...
	NewMenuItem_Icon(OptionsMenu_BIOSMiscFiles, "_BIOS/Misc Files...", "OptionsMenu_BIOSMiscFiles", OptionsMenu,
			 OptionsMenu_BIOSMiscFiles_Icon, "binary.png");
	AddMenuCallback(OptionsMenu_BIOSMiscFiles, on_OptionsMenu_BIOSMiscFiles_activate);
	
	// Separator
	NewMenuSeparator(OptionsMenu_Separator1, "OptionsMenu_Separator1", OptionsMenu);
	
#ifdef GENS_CDROM
	// Current CD Drive...
	NewMenuItem_StockIcon(OptionsMenu_CurrentCDDrive, "Current _CD Drive...", "OptionsMenu_CurrentCDDrive", OptionsMenu,
			      OptionsMenu_CurrentCDDrive_Icon, "gtk-cdrom");
	AddMenuCallback(OptionsMenu_CurrentCDDrive, on_OptionsMenu_CurrentCDDrive_activate);
#endif
	
	// Sega CD SRAM Size
	NewMenuItem_Icon(OptionsMenu_SegaCDSRAMSize, "Sega CD S_RAM Size", "OptionsMenu_SegaCDSRAMSize", OptionsMenu,
			 OptionsMenu_SegaCDSRAMSize_Icon, "memory.png");
	// Sega CD SRAM Size submenu
	create_gens_window_OptionsMenu_SegaCDSRAMSize_SubMenu(OptionsMenu_SegaCDSRAMSize);
	
	// Separator
	NewMenuSeparator(OptionsMenu_Separator2, "OptionsMenu_Separator2", OptionsMenu);
	
	// Load Config...
	NewMenuItem_StockIcon(OptionsMenu_LoadConfig, "_Load Config...", "OptionsMenu_LoadConfig", OptionsMenu,
			      OptionsMenu_LoadConfig_Icon, "gtk-open");
	AddMenuCallback(OptionsMenu_LoadConfig, on_OptionsMenu_LoadConfig_activate);
	
	// Save Config As...
	NewMenuItem_StockIcon(OptionsMenu_SaveConfigAs, "_Save Config As...", "OptionsMenu_SaveConfigAs", OptionsMenu,
			      OptionsMenu_SaveConfigAs_Icon, "gtk-save-as");
	AddMenuCallback(OptionsMenu_SaveConfigAs, on_OptionsMenu_SaveConfigAs_activate);
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


/**
 * create_gens_window_HelpMenu(): Create the Help menu.
 * @param container Container for this menu.
 */
static void create_gens_window_HelpMenu(GtkWidget *container)
{
	GtkWidget *Help;			GtkWidget *Help_Icon;
	GtkWidget *HelpMenu;
	GtkWidget *HelpMenu_About;		GtkWidget *HelpMenu_About_Icon;
	
	// Help
	NewMenuItem_StockIcon(Help, "_Help", "Help", container, Help_Icon, "gtk-help");
	
	// Menu object for the HelpMenu
	HelpMenu = gtk_menu_new();
	gtk_widget_set_name(HelpMenu, "HelpMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Help), HelpMenu);
	
	// About
	NewMenuItem_StockIcon(HelpMenu_About, "_About", "About", HelpMenu, HelpMenu_About_Icon, "gtk-help");
	AddMenuCallback(HelpMenu_About, on_HelpMenu_About_activate);
}
#endif
