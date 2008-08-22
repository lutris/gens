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

// Sega CD
#include "cd_aspi.h"
#include "g_mcd.h"


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
