/***************************************************************************
 * Gens: (Win32) Main Window - Callback Functions.                         *
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

#include "emulator/g_main.hpp"
#include "gens_window.h"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"
#if 0
#include "game_genie/game_genie_window_misc.h"
#include "controller_config/controller_config_window_misc.hpp"
#include "bios_misc_files/bios_misc_files_window_misc.hpp"
#include "directory_config/directory_config_window_misc.hpp"
#include "general_options/general_options_window_misc.hpp"
#include "about/about_window.h"
#include "color_adjust/color_adjust_window_misc.h"
#include "country_code/country_code_window_misc.h"

#ifdef GENS_OPENGL
#include "opengl_resolution/opengl_resolution_window_misc.h"
#endif /* GENS_OPENGL */

#ifdef GENS_CDROM
#include "select_cdrom/select_cdrom_window_misc.h"
#endif /* GENS_CDROM */
#endif

#include "emulator/ui_proxy.hpp"
#include "util/file/config_file.hpp"

#include "ui/gens_ui.hpp"

#include "util/sound/gym.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/save.hpp"
#include "gens_core/cpu/z80/z80.h"
#include "util/gfx/scrshot.h"

// Sega CD
#include "emulator/g_mcd.hpp"

// 32X
#include "gens_core/cpu/sh2/sh2.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// C++ includes
#include <string>
using std::string;


// For some reason, these aren't extern'd anywhere...
extern "C"
{
	void main68k_reset();
	void sub68k_reset();
}


// Menu identifier definitions
#include "gens_window_menu.h"


#include "video/v_draw_ddraw.hpp"
static bool PaintsEnabled = true;


static void on_gens_window_close(void);
static void on_gens_window_FileMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK Gens_Window_WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	int t;
	
	switch(message)
	{
		case WM_CLOSE:
			on_gens_window_close();
			return 0;
		
		case WM_CREATE:
			Active = 1;
			break;
		
		case WM_PAINT:
			HDC hDC;
			PAINTSTRUCT ps;
			
			hDC = BeginPaint(hWnd, &ps);
			
			if (PaintsEnabled)
			{
				((VDraw_DDraw*)draw)->clearPrimaryScreen();
				draw->flip();
			}
			
			EndPaint(hWnd, &ps);
			break;
		
		case WM_MENUSELECT:
		case WM_ENTERSIZEMOVE:
			// Prevent audio stuttering when a menu is opened or the window is resized.
			audio->clearSoundBuffer();
			break;
		
		case WM_COMMAND:
			// Menu item.
			switch (LOWORD(wParam) & 0xF000)
			{
				case ID_FILE_MENU:
					on_gens_window_FileMenu(hWnd, message, wParam, lParam);
					break;
			}
			break;
		
#if 0
#ifdef GENS_DEBUGGER
		case WM_KEYDOWN:
			// TODO: Make sure this is correct.
			if (Debug)
				Debug_Event(wParam, 0);
			break;
#endif /* GENS_DEBUGGER */
#endif
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * Window is closed.
 */
static void on_gens_window_close(void)
{
	//Modif N - making sure sound doesn't stutter on exit
	if (audio->soundInitialized())
		audio->clearSoundBuffer();
	
	close_gens();
}


static void on_gens_window_FileMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case ID_FILE_OPENROM:
			if (audio->playingGYM())
				Stop_Play_GYM();
			if (Get_Rom() != -1)
				Sync_Gens_Window();
			break;
		
		case ID_FILE_CLOSEROM:
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
			
			Free_Rom(Game);
			Sync_Gens_Window();
			break;
	}
}


#if 0
#ifdef GENS_CDROM
/**
 * File, Boot CD
 */
void on_FileMenu_BootCD_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	if (!Num_CD_Drive)
		return;
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	if (audio->playingGYM())
		Stop_Play_GYM();
	
	Free_Rom(Game); // Don't forget it !
	SegaCD_Started = Init_SegaCD(NULL);
	Sync_Gens_Window();
}
#endif


/**
 * File, ROM History, #
 */
void on_FileMenu_ROMHistory_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	
	if (audio->playingGYM())
		Stop_Play_GYM();
	Open_Rom(Recent_Rom[GPOINTER_TO_INT(user_data)]);
	Sync_Gens_Window();
}


/**
 * File, Game Genie
 */
void on_FileMenu_GameGenie_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_Game_Genie();
}


/**
 * File, Load State...
 */
void on_FileMenu_LoadState_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	Str_Tmp[0] = 0;
	if (Change_File_L(Str_Tmp, State_Dir) == 1)
		Load_State(Str_Tmp);
}


/**
 * File, Save State As...
 */
void on_FileMenu_SaveState_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	Str_Tmp[0] = 0;
	if (Change_File_S(Str_Tmp, State_Dir) == 1)
		Save_State(Str_Tmp);
}


/**
 * File, Quick Load
 */
void on_FileMenu_QuickLoad_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	char SaveFile[GENS_PATH_MAX] = "";
	Get_State_File_Name(SaveFile);
	Load_State(SaveFile);
}


/**
 * File, Quick Save
 */
void on_FileMenu_QuickSave_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	Str_Tmp[0] = 0;
	Get_State_File_Name(Str_Tmp);
	Save_State(Str_Tmp);
}


/**
 * File, Change State, #
 */
void on_FileMenu_ChangeState_SubMenu_SlotItem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int slot = GPOINTER_TO_INT(user_data);
	
	if (!do_callbacks)
		return;
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		Set_Current_State(slot);
}


/**
 * File, Quit
 */
void on_FileMenu_Quit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	close_gens();
}


/**
 * Graphics, Full Screen
 */
void on_GraphicsMenu_FullScreen_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	if (!do_callbacks)
		return;

	/*
	if (Full_Screen)
		Set_Render(0, -1, 1);
	else
		Set_Render(1, Render_FS, 1);
	*/
	
	draw->setFullScreen(!draw->fullScreen());
	// TODO: See if draw->setRender() is still needed.
	//draw->setRender(Video.Render_Mode);
}


/**
 * Various items in the Graphics menu.
 */
CHECK_MENU_ITEM_CALLBACK(on_GraphicsMenu_VSync_activate, Change_VSync);
CHECK_MENU_ITEM_CALLBACK(on_GraphicsMenu_Stretch_activate, Change_Stretch);
#ifdef GENS_OPENGL
CHECK_MENU_ITEM_CALLBACK(on_GraphicsMenu_OpenGL_activate, Change_OpenGL);

void on_GraphicsMenu_OpenGLFilter_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(user_data);
	
	Video.glLinearFilter = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	
	if (Video.glLinearFilter)
		MESSAGE_L("Enabled OpenGL Linear Filter", "Enabled OpenGL Linear Filter", 1500);
	else
		MESSAGE_L("Disabled OpenGL Linear Filter", "Disabled OpenGL Linear Filter", 1500);
}
#endif


#ifdef GENS_OPENGL
/**
 * Graphics, Resolution, #x# or Custom
 * TODO: Use this for SDL mode too, not just for OpenGL mode.
 */
void on_GraphicsMenu_OpenGLRes_SubMenu_ResItem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int resValue, w, h;
	
	if (!do_callbacks)
		return;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	
	resValue = GPOINTER_TO_INT(user_data);
	if (resValue == 0)
	{
		// Custom Resolution.
		Open_OpenGL_Resolution();
		return;
	}
	
	// Get the resolution.
	w = (resValue >> 16);
	h = (resValue & 0xFFFF);
	
	// Set the resolution.
	Set_GL_Resolution(w, h);
}
#endif


/**
 * Graphics, Bits per pixel, #
 * TODO: Use this for SDL mode too, not just for OpenGL mode.
 */
void on_GraphicsMenu_bpp_SubMenu_bppItem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int bpp = GPOINTER_TO_INT(user_data);
	
	if (!do_callbacks)
		return;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	
	// Set the bits per pixel.
	draw->setBpp(bpp);
	MESSAGE_NUM_L("Selected %d-bit color depth", "Selected %d-bit color depth", (bpp), 1500);
}


/**
 * Graphics, Color Adjust...
 */
void on_GraphicsMenu_ColorAdjust_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_Color_Adjust();
}


/**
 * Graphics, Sprite Limit
 */
CHECK_MENU_ITEM_CALLBACK(on_GraphicsMenu_SpriteLimit_activate, Set_Sprite_Limit);


/**
 * Graphics, Render, #
 */
void on_GraphicsMenu_Render_SubMenu_RenderItem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int renderMode = GPOINTER_TO_INT(user_data);
	
	if (!do_callbacks)
		return;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	
	// Set the render mode.
	draw->setRender(renderMode);
}


/**
 * Graphics, Frame Skip, #
 */
void on_GraphicsMenu_FrameSkip_SubMenu_FSItem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int fs = GPOINTER_TO_INT(user_data);
	
	if (!do_callbacks)
		return;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	
	// Set the frame skip value.
	Set_Frame_Skip(fs);
}


/**
 * Graphics, Screen Shot
 */
void on_GraphicsMenu_ScreenShot_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	audio->clearSoundBuffer();
	Save_Shot();
}


#ifdef GENS_DEBUGGER
/**
 * CPU, Debug, #
 */
void on_CPUMenu_Debug_SubMenu_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int newDebug = GPOINTER_TO_INT(user_data);
	int i;
	
	if (!do_callbacks)
		return;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
	{
		// Debug mode is unchecked.
		if (newDebug == Debug)
		{
			// This debugging mode is being turned off.
			Change_Debug(newDebug);
		}
		return;
	}
	
	// Set the debug mode.
	Change_Debug(newDebug);
	
	// Uncheck all other Debug items.
	for (i = 0; i < 9; i++)
	{
		if (i + 1 != newDebug)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(debugMenuItems[i]), FALSE);
	}	
}
#endif /* GENS_DEBUGGER */


/**
 * CPU, Country, #
 */
void on_CPUMenu_Country_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		Change_Country(GPOINTER_TO_INT(user_data));
}


/**
 * CPU, Country, Auto-Detect Order...
 */
void on_CPUMenu_Country_SubMenu_AutoDetectOrder_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_Country_Code();
}


/**
 * CPU, Hard Reset
 */
void on_CPUMenu_HardReset_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	system_reset();
}


/**
 * CPU, Reset Main 68000
 */
void on_CPUMenu_ResetMain68000_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	
	if (!Game)
		return;
	
	Paused = 0;
	main68k_reset();
	if (Genesis_Started || _32X_Started)
	{
		MESSAGE_L("68000 CPU reset", "68000 CPU reset", 1000);
	}
	else if(SegaCD_Started)
	{
		MESSAGE_L("Main 68000 CPU reset", "Main 68000 CPU reset", 1000);
	}
}


/**
 * CPU, Reset Sub 68000
 */
void on_CPUMenu_ResetSub68000_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	
	if (!Game || !SegaCD_Started)
		return;
	
	Paused = 0;
	sub68k_reset();
	MESSAGE_L("Sub 68000 CPU reset", "Sub 68000 CPU reset", 1000);
}


/**
 * CPU, Reset Main SH2
 */
void on_CPUMenu_ResetMainSH2_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	
	if (!Game || !_32X_Started)
		return;
	
	Paused = 0;
	SH2_Reset(&M_SH2, 1);
	MESSAGE_L("Master SH2 reset", "Master SH2 reset", 1000);
}


/**
 * CPU, Reset Sub SH2
 */
void on_CPUMenu_ResetSubSH2_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	
	if (!Game || !_32X_Started)
		return;
	
	Paused = 0;
	SH2_Reset(&S_SH2, 1);
	MESSAGE_L("Slave SH2 reset", "Slave SH2 reset", 1000);
}


/**
 * CPU, Reset Z80
 */
void on_CPUMenu_ResetZ80_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	
	if (!Game)
		return;
	
	z80_Reset(&M_Z80);
	MESSAGE_L("Z80 reset", "Z80 reset", 1000);
}


/**
 * CPU, SegaCD Perfect Sync
 */
CHECK_MENU_ITEM_CALLBACK(on_CPUMenu_SegaCD_PerfectSync_activate, Change_SegaCD_PerfectSync);


/**
 * Sound, Enable
 */
void on_SoundMenu_Enable_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(user_data);
	
	if (!do_callbacks)
		return;
	Change_Sound(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));
	Sync_Gens_Window();
}


/**
 * CPU, Rate, #
 */
void on_SoundMenu_Rate_SubMenu_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		Change_Sample_Rate(GPOINTER_TO_INT(user_data));
}


/**
 * Various items in the Sound menu.
 */
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_Stereo_activate, Change_Sound_Stereo);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_Z80_activate, Change_Z80);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_YM2612_activate, Change_YM2612);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_YM2612_Improved_activate, Change_YM2612_Improved);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_DAC_activate, Change_DAC);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_DAC_Improved_activate, Change_DAC_Improved);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_PSG_activate, Change_PSG);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_PSG_Improved_activate, Change_PSG_Improved);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_PCM_activate, Change_PCM);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_PWM_activate, Change_PWM);
CHECK_MENU_ITEM_CALLBACK(on_SoundMenu_CDDA_activate, Change_CDDA);


/**
 * Sound, WAV Dump
 */
void on_SoundMenu_WAVDump_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(user_data);
	
	string label;
	
	// Change WAV dump status.
	if (!audio->dumpingWAV())
		audio->startWAVDump();
	else
		audio->stopWAVDump();
	
	// Check the status again to determine what to set the label to.
	label = (audio->dumpingWAV() ? "Stop WAV Dump" : "Start WAV Dump");
	
	// Set the text of the WAV dump menu item.
	gtk_label_set_text(GTK_LABEL(GTK_BIN(menuitem)->child), label.c_str());
}


/**
 * Sound, GYM Dump
 */
void on_SoundMenu_GYMDump_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(user_data);
	
	string label;
	
	// Change GYM dump status.
	if (!GYM_Dumping)
		Start_GYM_Dump();
	else
		Stop_GYM_Dump();
	
	// Check the status again to determine what to set the label to.
	label = (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump");
	
	// Set the text of the GYM dump menu item.
	gtk_label_set_text(GTK_LABEL(GTK_BIN(menuitem)->child), label.c_str());
}


/**
 * Options, General Options...
 */
void on_OptionsMenu_GeneralOptions_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_General_Options();
}


/**
 * Options, Joypads...
 */
void on_OptionsMenu_Joypads_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_Controller_Config();
}


/**
 * Options, Directories...
 */
void on_OptionsMenu_Directories_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_Directory_Config();
}


/**
 * Options, BIOS/Misc Files...
 */
void on_OptionsMenu_BIOSMiscFiles_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_BIOS_Misc_Files();
}


/**
 * Options, Sega CD SRAM Size, #
 */
void on_OptionsMenu_SegaCDSRAMSize_SubMenu_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		Change_SegaCD_SRAM_Size(GPOINTER_TO_INT(user_data));
}


#ifdef GENS_CDROM
/**
 * Options, Current CD Drive...
 */
void on_OptionsMenu_CurrentCDDrive_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Open_Select_CDROM();
}
#endif


/**
 * Options, Load Config...
 */
void on_OptionsMenu_LoadConfig_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Load_As_Config(Game);
	Sync_Gens_Window();
}


/**
 * Options, Save Config As...
 */
void on_OptionsMenu_SaveConfigAs_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Save_As_Config();
}


/**
 * Help, About
 */
void on_HelpMenu_About_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	create_about_window();
}
#endif
