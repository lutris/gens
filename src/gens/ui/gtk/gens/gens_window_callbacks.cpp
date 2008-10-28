/***************************************************************************
 * Gens: (GTK+) Main Window - Callback Functions.                          *
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
#include "gens_window.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"
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

#include "ui/gens_ui.hpp"
#include "gtk-misc.h"

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// C++ includes
#include <string>
using std::string;


// For some reason, these aren't extern'd anywhere...
extern "C"
{
	void main68k_reset();
	void sub68k_reset();
}


/**
 * CHECK_MENU_ITEM_CALLBACK(): Macro for simple callbacks that merely check
 * if callbacks are enabled and run a function.
 * These callbacks are usually used for menu item that act as checkboxes.
 * @param callbackFunction: Name of the callback function.
 * @param uiProxyFunction: Function in ui_proxy.c to call.
 */
#define CHECK_MENU_ITEM_CALLBACK(callbackFunction, uiProxyFunction)			\
void callbackFunction(GtkMenuItem *menuitem, gpointer user_data)			\
{											\
	GENS_UNUSED_PARAMETER(user_data);						\
											\
	if (!do_callbacks)								\
		return;									\
	uiProxyFunction(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));	\
}


/**
 * Window is closed.
 */
gboolean on_gens_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	close_gens();
	
	// TRUE tells GTK+ not to close the window. This is needed
	// in order to prevent an X11 error from occurring due to
	// the embedded SDL window.
	return TRUE;
}


/**
 * File, Open ROM...
 */
void on_FileMenu_OpenROM_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	/*
	if ((Check_If_Kaillera_Running()))
		return 0;
	*/
	if (audio->playingGYM())
		Stop_Play_GYM();
	if (ROM::getROM() != -1)
		Sync_Gens_Window();
	/* Removed: Get_Rom() and related show an error by themselves.
	else
		GensUI::msgBox("Error opening ROM.", "Error");
	*/
}


#ifdef GENS_CDROM
/**
 * File, Boot CD
 */
void on_FileMenu_BootCD_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	if (!ASPI_Initialized || !Num_CD_Drive)
	{
		printf("ASPI not initialized and/or no CD-ROM drive(s) detected.\n");
		return;
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
	ROM::openROM(Recent_Rom[GPOINTER_TO_INT(user_data)]);
	Sync_Gens_Window();
}


/**
 * File, Close ROM
 */
void on_FileMenu_CloseROM_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
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
	
	string filename = Savestate::selectFile(false, State_Dir);
	if (!filename.empty())
		Savestate::loadState(filename.c_str());
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
	
	string filename = Savestate::selectFile(true, State_Dir);
	if (!filename.empty())
		Savestate::saveState(filename);
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
	
	string filename = Savestate::getStateFilename();
	Savestate::loadState(filename.c_str());
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
	
	string filename = Savestate::getStateFilename();
	Savestate::saveState(filename);
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
	int newBpp = GPOINTER_TO_INT(user_data);
	
	if (!do_callbacks)
		return;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	
	// Set the bits per pixel.
	draw->setBpp(newBpp);
	MESSAGE_NUM_L("Selected %d-bit color depth", "Selected %d-bit color depth", newBpp, 1500);
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
	ImageUtil::screenShot();
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
 * Sound, Rate, #
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
	
	Config::loadAs(Game);
	Sync_Gens_Window();
}


/**
 * Options, Save Config As...
 */
void on_OptionsMenu_SaveConfigAs_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	Config::saveAs();
}


/**
 * Help, About
 */
void on_HelpMenu_About_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(menuitem);
	GENS_UNUSED_PARAMETER(user_data);
	
	AboutWindow::Instance(GTK_WINDOW(gens_window));
}


/** Drag & Drop callbacks **/


void gens_window_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
				    GtkSelectionData *selection_data, guint target_type, guint time,
				    gpointer data)
{
	gboolean dnd_success = FALSE;
	gboolean delete_selection_data = FALSE;
	
	if ((selection_data != NULL) && (selection_data->length >= 0))
	{
		string filename;
		dnd_success = TRUE;
		
		filename = string((gchar*)(selection_data->data));
		
		if (filename.length() >= 8 && filename.substr(0, 8) == "file:///")
		{
			// "file:///" prefix. Remove the prefix.
			filename = filename.substr(7);
		}
		else if (filename.length() >= 7 && filename.substr(0, 7) == "file://")
		{
			// "file://" prefix. Remove the prefix.
			filename = filename.substr(6);
		}
		else if (filename.length() >= 6 && filename.substr(0, 6) == "file:/")
		{
			// "file:/" prefix. Remove the prefix.
			filename = filename.substr(5);
		}
		else if (filename.length() >= 10 && filename.substr(0, 9) == "desktop:/")
		{
			// "desktop:/" prefix. Remove the prefix and prepend the user's desktop directory.
			filename = string(getenv("HOME")) + "/Desktop/" + filename.substr(9);
		}
		
		// Unescape the URI.
		char* unescaped = g_uri_unescape_string(filename.c_str(), NULL);
		filename = string(unescaped);
		g_free(unescaped);
		
		// Check that the file actually exists.
		struct stat sbuf;
		if (!stat(filename.c_str(), &sbuf))
		{
			// File exists. Open it as a ROM image.
			ROM::openROM(filename.c_str());
		}
		else
		{
			// File does not exist.
			dnd_success = FALSE;
		}
	}
	
	gtk_drag_finish(context, dnd_success, delete_selection_data, time);
}


gboolean gens_window_drag_drop(GtkWidget *widget, GdkDragContext *context,
			       gint x, gint y, guint time, gpointer user_data)
{
	if (context->targets)
	{
		GdkAtom target_type = GDK_POINTER_TO_ATOM(g_list_nth_data(context->targets, 0));
		gtk_drag_get_data(widget, context, target_type, time);
		return TRUE;
	}
	return FALSE;
}
