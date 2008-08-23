/**
 * GENS: Main Window Callbacks. (GTK+)
 */
 

#include "gens.h"
#include "g_main.h"
#include "genswindow_callbacks.h"
#include "ui-common.h"

#include "g_sdlsound.h"
#include "gym.h"
#include "rom.h"
#include "vdp_io.h"
#include "save.h"

// Sega CD
#include "cd_aspi.h"
#include "g_mcd.h"


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
		sync_gens_ui(UPDATE_GTK);
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
}


/**
 * File, Game Genie
 */
void on_FileMenu_GameGenie_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	// TODO: Reimplement Game Genie.
	fprintf(stderr, "TODO: STUB: on_FileMenu_GameGenie_activate()\n");
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
#define CHANGE_SAVE_SLOT(function, slot)					\
void function(GtkMenuItem *menuitem, gpointer user_data)			\
{										\
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))	\
		Set_Current_State(slot);					\
}
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_0_activate, 0);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_1_activate, 1);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_2_activate, 2);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_3_activate, 3);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_4_activate, 4);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_5_activate, 5);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_6_activate, 6);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_7_activate, 7);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_8_activate, 8);
CHANGE_SAVE_SLOT(on_FileMenu_ChangeState_SubMenu_9_activate, 9);


/**
 * File, Quit
 */
void on_FileMenu_Quit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	close_gens();
}
