/**
 * GENS: Main Window Callbacks. (GTK+)
 */
 

#include "gens.h"
#include "g_main.h"
#include "genswindow.h"
#include "genswindow_callbacks.h"
#include "ui_proxy.h"
#include "ui-common.h"

#include "g_sdlsound.h"
#include "g_sdldraw.h"
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
	fprintf(stderr, "TODO: STUB: %s()\n", __func__);
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
	printf("%d\n", Full_Screen);
}


/**
 * Graphics, VSync
 */
void on_GraphicsMenu_VSync_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	
	Change_VSync(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));
}


/**
 * Graphics, Stretch
 */
void on_GraphicsMenu_Stretch_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	
	Change_Stretch(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));
}


/**
 * Graphics, OpenGL
 */
void on_GraphicsMenu_OpenGL_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	
	Change_OpenGL(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));
}


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
	// TODO: Color Adjust window.
	fprintf(stderr, "TODO: STUB: %s()\n", __func__);
}


/**
 * Graphics, Sprite Limit
 */
void on_GraphicsMenu_SpriteLimit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	
	Set_Sprite_Limit(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)));
}


/**
 * Graphics, Render, #
 */
void on_GraphicsMenu_Render_SubMenu_RenderItem_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	int renderMode = GPOINTER_TO_INT(user_data);
	
	if (!do_callbacks)
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
