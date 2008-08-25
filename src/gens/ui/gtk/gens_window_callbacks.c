/**
 * GENS: (GTK+) Main Window Callbacks.
 */
 

#include "gens.h"
#include "g_main.h"
#include "gens_window.h"
#include "gens_window_callbacks.h"
#include "gens_window_sync.h"
#include "game_genie_window_misc.h"
#include "controller_config_window.h"

#include "ui_proxy.h"
#include "ui-common.h"
#include "config_file.h"

#include "g_sdlsound.h"
#include "g_sdldraw.h"
#include "gym.h"
#include "rom.h"
#include "vdp_io.h"
#include "save.h"
#include "z80.h"

// Sega CD
#include "cd_aspi.h"
#include "g_mcd.h"

// 32X
#include "sh2.h"


// For some reason, these aren't extern'd anywhere...
void main68k_reset();
void sub68k_reset();


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
	if (!do_callbacks)								\
		return;									\
	uiProxyFunction(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));	\
}


/**
 * STUB: Indicates that this function is a stub.
 */
#define STUB fprintf(stderr, "TODO: STUB: %s()\n", __func__);


/**
 * Window is closed.
 */
gboolean on_gens_window_close(GtkMenuItem *menuitem, GdkEvent *event, gpointer user_data)
{
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
	/*
	if ((Check_If_Kaillera_Running()))
		return 0;
	*/
	if (GYM_Playing)
		Stop_Play_GYM();
	if (Get_Rom() != -1)
		Sync_Gens_Window();
	else
		UI_MsgBox("Error opening ROM.", "Error");
}


/**
 * File, Boot CD
 */
void on_FileMenu_BootCD_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!Num_CD_Drive)
		return;
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	if (GYM_Playing)
		Stop_Play_GYM();
	
	Free_Rom(Game); // Don't forget it !
	SegaCD_Started = Init_SegaCD(NULL);
	Sync_Gens_Window();
}


/**
 * File, ROM History, #
 */
void on_FileMenu_ROMHistory_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (GYM_Playing)
		Stop_Play_GYM();
	Open_Rom(Recent_Rom[GPOINTER_TO_INT(user_data)]);
	Sync_Gens_Window();
}


/**
 * File, Close ROM
 */
void on_FileMenu_CloseROM_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (Sound_Initialised)
		Clear_Sound_Buffer();
	Debug = 0;
	if (Net_Play)
	{
		if (Full_Screen)
			Set_Render( 0, -1, 1);
	}
	Free_Rom(Game);
	Sync_Gens_Window();
}


/**
 * File, Game Genie
 */
void on_FileMenu_GameGenie_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	Open_Game_Genie();
}


/**
 * File, Load State...
 */
void on_FileMenu_LoadState_activate(GtkMenuItem *menuitem, gpointer user_data)
{
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
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	Str_Tmp[0] = 0;
	Get_State_File_Name(Str_Tmp);
	Load_State(Str_Tmp);
}


/**
 * File, Quick Save
 */
void on_FileMenu_QuickSave_activate(GtkMenuItem *menuitem, gpointer user_data)
{
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
	close_gens();
}


/**
 * Graphics, Full Screen
 */
void on_GraphicsMenu_FullScreen_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;

	/*
	if (Full_Screen)
		Set_Render(0, -1, 1);
	else
		Set_Render(1, Render_FS, 1);
	*/
	
	Full_Screen = !Full_Screen;
	Set_Render(Full_Screen, Render_Mode, 0);
}


/**
 * Various items in the Graphics menu.
 */
CHECK_MENU_ITEM_CALLBACK(on_GraphicsMenu_VSync_activate, Change_VSync);
CHECK_MENU_ITEM_CALLBACK(on_GraphicsMenu_Stretch_activate, Change_Stretch);
CHECK_MENU_ITEM_CALLBACK(on_GraphicsMenu_OpenGL_activate, Change_OpenGL);


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
		// TODO: Custom Resolution window.
		fprintf(stderr, "TODO: %s() - Custom Resolution Window\n", __func__);
		return;
	}
	
	// Get the resolution.
	w = (resValue >> 16);
	h = (resValue & 0xFFFF);
	
	// Set the resolution.
	Set_GL_Resolution(w, h);
	MESSAGE_NUM_L("Seleted %dx[TODO] Resolution", "Selected %dx[TODO] Resolution", (w), 1500);
}


/**
 * Graphics, Bits per pixel, #
 * TODO: Use this for SDL mode too, not just for OpenGL mode.
 */
void on_GraphicsMenu_bpp_SubMenu_bppItem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int bpp = GPOINTER_TO_INT(user_data);
	
	if (!do_callbacks || !Opengl)
		return;
	if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
		return;
	
	// Set the bits per pixel.
	Set_Bpp(bpp);
	MESSAGE_NUM_L("Selected %d-bit color depth", "Selected %d-bit color depth", (bpp), 1500);
}


/**
 * Graphics, Color Adjust...
 */
void on_GraphicsMenu_ColorAdjust_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	STUB;
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
	Set_Render(Full_Screen, renderMode, 0);
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
	Clear_Sound_Buffer();
	Take_Shot();
}


#ifdef GENS_DEBUG
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
#endif


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
	STUB;
}


/**
 * CPU, Hard Reset
 */
void on_CPUMenu_HardReset_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	system_reset();
}


/**
 * CPU, Reset Main 68000
 */
void on_CPUMenu_ResetMain68000_activate(GtkMenuItem *menuitem, gpointer user_data)
{
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
	gchar* label;
	
	// Change WAV dump status.
	if (!WAV_Dumping)
		Start_WAV_Dump();
	else
		Stop_WAV_Dump();
	
	// Check the status again to determine what to set the label to.
	label = (WAV_Dumping ? "Stop WAV Dump" : "Start WAV Dump");
	
	// Set the text of the WAV dump menu item.
	gtk_label_set_text(GTK_LABEL(GTK_BIN(menuitem)->child), label);
}


/**
 * Sound, GYM Dump
 */
void on_SoundMenu_GYMDump_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gchar* label;
	
	// Change GYM dump status.
	if (!GYM_Dumping)
		Start_GYM_Dump();
	else
		Stop_GYM_Dump();
	
	// Check the status again to determine what to set the label to.
	label = (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump");
	
	// Set the text of the GYM dump menu item.
	gtk_label_set_text(GTK_LABEL(GTK_BIN(menuitem)->child), label);
}


/**
 * Options, General Options...
 */
void on_OptionsMenu_GeneralOptions_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	STUB;
}


/**
 * Options, Joypads...
 */
void on_OptionsMenu_Joypads_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	Open_Controller_Config();
}


/**
 * Options, Directories...
 */
void on_OptionsMenu_Directories_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	STUB;
}


/**
 * Options, BIOS/Misc Files...
 */
void on_OptionsMenu_BIOSMiscFiles_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	STUB;
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


/**
 * Options, Current CD Drive...
 */
void on_OptionsMenu_CurrentCDDrive_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	STUB;
}


/**
 * Options, Load Config...
 */
void on_OptionsMenu_LoadConfig_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	Load_As_Config(Game);
	Sync_Gens_Window();
}


/**
 * Options, Save Config As...
 */
void on_OptionsMenu_SaveConfigAs_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	Save_As_Config();
}


/**
 * Help, About
 */
void on_HelpMenu_About_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	STUB;
}
