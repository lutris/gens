/***************************************************************************
 * Gens: (GTK+) Main Window - Synchronization Functions.                   *
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
#include "gens_window_sync.h"
#include "gens_window_callbacks.h"

#include "gtk-misc.h"

#include "g_main.h"
#include "g_sdldraw.h"
#include "vdp_rend.h"
#include "gens.h"
#include "vdp_io.h"
#include "mem_m68k.h"
#include "g_sdlsound.h"
#include "ym2612.h"
#include "psg.h"
#include "pcm.h"
#include "pwm.h"
#include "cd_sys.h"
#include "gym.h"

/**
 * Sync_Gens_Window(): Synchronize the GENS Main Window.
 */
void Sync_Gens_Window(void)
{
	// Synchronize all menus.
	Sync_Gens_Window_FileMenu();
	Sync_Gens_Window_GraphicsMenu();
	Sync_Gens_Window_CPUMenu();
	Sync_Gens_Window_SoundMenu();
	Sync_Gens_Window_OptionsMenu();
}


/**
 * Sync_Gens_Window_FileMenu(): Synchronize the File Menu.
 */
void Sync_Gens_Window_FileMenu(void)
{
	GtkWidget *MItem_ROMHistory, *MItem_ROMHistory_SubMenu;
	GtkWidget *MItem_ROMHistory_SubMenu_Item, *MItem_SaveState;
	char ROM_Name[GENS_PATH_MAX];
	
	// ROM Format prefixes
	// TODO: Move this somewhere else.
	const char* ROM_Format_Prefix[5] = {"[----]", "[MD]", "[32X]", "[SCD]", "[SCDX]"};
	
	// Temporary variables for ROM History.
	int i, romFormat;
	// Number of ROMs found for ROM History.
	int romsFound = 0;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// ROM History
	MItem_ROMHistory = lookup_widget(gens_window, "FileMenu_ROMHistory");
	gtk_menu_item_remove_submenu(GTK_MENU_ITEM(MItem_ROMHistory));
	MItem_ROMHistory_SubMenu = gtk_menu_new();
	gtk_widget_set_name(MItem_ROMHistory_SubMenu, "FileMenu_ROMHistory_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(MItem_ROMHistory), MItem_ROMHistory_SubMenu);
	
	for (i = 0; i < 9; i++)
	{
		// Make sure this Recent ROM entry actually has an entry.
		if (strlen(Recent_Rom[i]) == 0)
			continue;
		
		// Increment the ROMs Found counter.
		romsFound++;
		
		// Determine the ROM format.
		// TODO: Improve the return variable from Detect_Format()
		romFormat = Detect_Format(Recent_Rom[i]) >> 1;
		if (romFormat >= 1 && romFormat <= 4)
			strcpy(ROM_Name, ROM_Format_Prefix[romFormat]);
		else
			strcpy(ROM_Name, ROM_Format_Prefix[0]);
		
		// Add a tab, a dash, and a space.
		strcat(ROM_Name, "\t- ");
		
		// Get the ROM filename.
		Get_Name_From_Path(Recent_Rom[i], Str_Tmp);
		strcat(ROM_Name, Str_Tmp);
		
		// Add the ROM item to the ROM History submenu.
		MItem_ROMHistory_SubMenu_Item = gtk_menu_item_new_with_label(ROM_Name);
		gtk_widget_show(MItem_ROMHistory_SubMenu_Item);
		gtk_container_add(GTK_CONTAINER(MItem_ROMHistory_SubMenu),
				  MItem_ROMHistory_SubMenu_Item);
		g_signal_connect((gpointer)MItem_ROMHistory_SubMenu_Item, "activate",
				 G_CALLBACK(on_FileMenu_ROMHistory_activate),
				 GINT_TO_POINTER(i));
	}
	
	// If no recent ROMs were found, disable the ROM History menu.
	gtk_widget_set_sensitive(MItem_ROMHistory, romsFound);
	
	// Current savestate
	sprintf(Str_Tmp, "FileMenu_ChangeState_SubMenu_%d", Current_State);
	MItem_SaveState = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_SaveState), TRUE);
	
	// TODO: Disable Close ROM if no ROM is loaded.
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	GtkWidget *MItem_VSync, *MItem_Stretch, *MItem_OpenGL, *MItem_SpriteLimit;
	GtkWidget *MItem_OpenGL_Resolution, *MItem_OpenGL_Resolution_Custom;
	GtkWidget *MItem_bpp, *MItem_Render, *MItem_FrameSkip;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Simple checkbox items
	MItem_VSync = lookup_widget(gens_window, "GraphicsMenu_VSync");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_VSync), W_VSync);
	MItem_Stretch = lookup_widget(gens_window, "GraphicsMenu_Stretch");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Stretch), Stretch);
	MItem_OpenGL = lookup_widget(gens_window, "GraphicsMenu_OpenGL");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_OpenGL), Opengl);
	MItem_SpriteLimit = lookup_widget(gens_window, "GraphicsMenu_SpriteLimit");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_SpriteLimit), Sprite_Over);
	
	// OpenGL Resolution
	
	// Get the Custom resolution menu item.
	MItem_OpenGL_Resolution_Custom = lookup_widget(gens_window, "GraphicsMenu_OpenGLRes_SubMenu_Custom");
	
	// Check if the current GL resolution is a custom resolution.
	// TODO: Make an array with predefined resolutions somewhere.
	if ((Width_gl == 320 && Height_gl == 240) ||
	    (Width_gl == 640 && Height_gl == 480) ||
	    (Width_gl == 800 && Height_gl == 600) ||
	    (Width_gl == 1024 && Height_gl == 768))
	{
		// Predefined resolution.
		sprintf(Str_Tmp, "GraphicsMenu_OpenGLRes_SubMenu_%dx%d", Width_gl, Height_gl);
		MItem_OpenGL_Resolution = lookup_widget(gens_window, Str_Tmp);
		// Set the text of the Custom entry to "Custom..."
		gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_OpenGL_Resolution_Custom)->child), "Custom...");
	}
	else
	{
		// Custom resolution.
		MItem_OpenGL_Resolution = MItem_OpenGL_Resolution_Custom;
		// Set the text of the Custom entry to "Custom... (wxh)"
		sprintf(Str_Tmp, "Custom... (%dx%d)", Width_gl, Height_gl);
		gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_OpenGL_Resolution_Custom)->child), Str_Tmp);
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_OpenGL_Resolution), TRUE);
	
	// Bits per pixel
	sprintf(Str_Tmp, "GraphicsMenu_bpp_SubMenu_%d", Bpp);
	MItem_bpp = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_bpp), TRUE);
	
	// Render
	sprintf(Str_Tmp, "GraphicsMenu_Render_SubMenu_%d", Render_Mode);
	MItem_Render = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Render), TRUE);
	
	// Frame Skip
	if (Frame_Skip == -1)
		strcpy(Str_Tmp, "GraphicsMenu_FrameSkip_SubMenu_Auto");
	else
		sprintf(Str_Tmp, "GraphicsMenu_FrameSkip_SubMenu_%d", Frame_Skip);
	MItem_FrameSkip = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_FrameSkip), TRUE);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
#ifdef GENS_DEBUG
	GtkWidget *MItem_Debug;
	GtkWidget *MItem_Debug_Item;
	int i, checkDebug;
#endif
	GtkWidget *MItem_Country;
	GtkWidget *MItem_Reset68K, *MItem_ResetM68K, *MItem_ResetS68K;
	GtkWidget *MItem_ResetMSH2, *MItem_ResetSSH2;
	GtkWidget *MItem_SegaCD_PerfectSync;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
#ifdef GENS_DEBUG
	MItem_Debug = lookup_widget(gens_window, "CPUMenu_Debug");
	gtk_widget_set_sensitive(MItem_Debug, (Genesis_Started || SegaCD_Started || _32X_Started));
	
	// Hide/Show debug entries depending on the active console.
	
	if (Genesis_Started || SegaCD_Started || _32X_Started)
	{
		for (i = 1; i <= 9; i++)
		{
			sprintf(Str_Tmp, "CPUMenu_Debug_SubMenu_%d", i);
			MItem_Debug_Item = lookup_widget(gens_window, Str_Tmp);
			// TODO: Use debug constants instead?
			if (i >= 1 && i <= 3)
				checkDebug = 1;
			else if (i >= 4 && i <= 6)
				checkDebug = SegaCD_Started;
			else if (i >= 7 && i <= 9)
				checkDebug = _32X_Started;
			else
			{
				// Shouldn't happen...
				fprintf(stderr, "%s: ERROR: i == %d\n", __func__, i);
				checkDebug = 0;
			}
			
			if (checkDebug)
				gtk_widget_show(MItem_Debug_Item);
			else
				gtk_widget_hide(MItem_Debug_Item);
			
			// Make sure the check state for this debug item is correct.
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Debug_Item), (Debug == i));
		}
		// Separators
		MItem_Debug_Item = lookup_widget(gens_window, "CPUMenu_Debug_SubMenu_Sep1");
		if (SegaCD_Started)
			gtk_widget_show(MItem_Debug_Item);
		else
			gtk_widget_hide(MItem_Debug_Item);
		
		MItem_Debug_Item = lookup_widget(gens_window, "CPUMenu_Debug_SubMenu_Sep2");
		if (_32X_Started)
			gtk_widget_show(MItem_Debug_Item);
		else
			gtk_widget_hide(MItem_Debug_Item);
	}
#endif

	// Country code
	if (Country == -1)
		MItem_Country = lookup_widget(gens_window, "CPUMenu_Country_SubMenu_Auto");
	else
	{
		sprintf(Str_Tmp, "CPUMenu_Country_SubMenu_%d", Country);
		MItem_Country = lookup_widget(gens_window, Str_Tmp);
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Country), TRUE);
	
	// TODO: Country order (maybe?)
	
	// Hide and show appropriate RESET items.
	MItem_Reset68K = lookup_widget(gens_window, "CPUMenu_Reset68000");
	MItem_ResetM68K = lookup_widget(gens_window, "CPUMenu_ResetMain68000");
	MItem_ResetS68K = lookup_widget(gens_window, "CPUMenu_ResetSub68000");
	MItem_ResetMSH2 = lookup_widget(gens_window, "CPUMenu_ResetMainSH2");
	MItem_ResetSSH2 = lookup_widget(gens_window, "CPUMenu_ResetSubSH2");
	
	if (SegaCD_Started)
	{
		// SegaCD: Hide regular 68000; show Main 68000 and Sub 68000.
		gtk_widget_hide(MItem_Reset68K);
		gtk_widget_show(MItem_ResetM68K);
		gtk_widget_show(MItem_ResetS68K);
	}
	else
	{
		// No SegaCD: Show regular 68000; hide Main 68000 and Sub 68000;
		gtk_widget_show(MItem_Reset68K);
		gtk_widget_hide(MItem_ResetM68K);
		gtk_widget_hide(MItem_ResetS68K);
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		gtk_widget_show(MItem_ResetMSH2);
		gtk_widget_show(MItem_ResetSSH2);
	}
	else
	{
		// 32X: Hide Main SH2 and Sub SH2.
		gtk_widget_hide(MItem_ResetMSH2);
		gtk_widget_hide(MItem_ResetSSH2);
	}
	
	// SegaCD Perfect Sync
	MItem_SegaCD_PerfectSync = lookup_widget(gens_window, "CPUMenu_SegaCD_PerfectSync");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_SegaCD_PerfectSync), SegaCD_Accurate);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
	GtkWidget *MItem_Enable, *MItem_Rate, *MItem_Stereo, *MItem_Z80;
	GtkWidget *MItem_YM2612, *MItem_YM2612_Improved;
	GtkWidget *MItem_DAC, *MItem_DAC_Improved;
	GtkWidget *MItem_PSG, *MItem_PSG_Improved;
	GtkWidget *MItem_PCM, *MItem_PWM, *MItem_CDDA;
	
	GtkWidget *MItem_GYMDump, *MItem_WAVDump;
	gchar *label; int allowGYMDump;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Simple checkbox items
	MItem_Enable = lookup_widget(gens_window, "SoundMenu_Enable");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Enable), Sound_Enable);
	MItem_Stereo = lookup_widget(gens_window, "SoundMenu_Stereo");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Stereo), Sound_Stereo);
	MItem_Z80 = lookup_widget(gens_window, "SoundMenu_Z80");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Z80), Z80_State & 1);
	MItem_YM2612 = lookup_widget(gens_window, "SoundMenu_YM2612");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_YM2612), YM2612_Enable);
	MItem_YM2612_Improved = lookup_widget(gens_window, "SoundMenu_YM2612_Improved");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_YM2612_Improved), YM2612_Improv);
	MItem_DAC = lookup_widget(gens_window, "SoundMenu_DAC");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_DAC), DAC_Enable);
	MItem_DAC_Improved = lookup_widget(gens_window, "SoundMenu_DAC_Improved");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_DAC_Improved), DAC_Improv);
	MItem_PSG = lookup_widget(gens_window, "SoundMenu_PSG");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PSG), PSG_Enable);
	MItem_PSG_Improved = lookup_widget(gens_window, "SoundMenu_PSG_Improved");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PSG_Improved), PSG_Improv);
	MItem_PCM = lookup_widget(gens_window, "SoundMenu_PCM");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PCM), PCM_Enable);
	MItem_PWM = lookup_widget(gens_window, "SoundMenu_PWM");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_PWM), PWM_Enable);
	MItem_CDDA = lookup_widget(gens_window, "SoundMenu_CDDA");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_CDDA), CDDA_Enable);
	
	// Rate
	sprintf(Str_Tmp, "SoundMenu_Rate_SubMenu_%d", Sound_Rate);
	MItem_Rate = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Rate), TRUE);
	
	// GYM dumping
	label = (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump");
	MItem_GYMDump = lookup_widget(gens_window, "SoundMenu_GYMDump");
	gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_GYMDump)->child), label);
	
	// WAV dumping
	label = (WAV_Dumping ? "Stop WAV Dump" : "Start WAV Dump");
	MItem_WAVDump = lookup_widget(gens_window, "SoundMenu_WAVDump");
	gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_WAVDump)->child), label);
	
	// Enable or disable items, depending on the Enable state.
	gtk_widget_set_sensitive(MItem_Stereo, Sound_Enable);
	gtk_widget_set_sensitive(MItem_YM2612, Sound_Enable);
	gtk_widget_set_sensitive(MItem_YM2612_Improved, Sound_Enable);
	gtk_widget_set_sensitive(MItem_DAC, Sound_Enable);
	gtk_widget_set_sensitive(MItem_DAC_Improved, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PSG, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PSG_Improved, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PCM, Sound_Enable);
	gtk_widget_set_sensitive(MItem_PWM, Sound_Enable);
	gtk_widget_set_sensitive(MItem_CDDA, Sound_Enable);
	gtk_widget_set_sensitive(MItem_GYMDump, Sound_Enable);
	// TODO: Uncomment this line after WAV dumping has been reimplemented.
	//gtk_widget_set_sensitive(MItem_WAVDump, Sound_Enable);
	
	// Enable or disable GYM/WAV dumping, depending on if a game is running or not.
	// Also, don't enable this if sound is disabled.
	allowGYMDump = (Genesis_Started || SegaCD_Started || _32X_Started) && Sound_Enable;
	gtk_widget_set_sensitive(MItem_GYMDump, allowGYMDump);
	gtk_widget_set_sensitive(MItem_WAVDump, allowGYMDump);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
	GtkWidget *SRAMSize;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	if (BRAM_Ex_State & 0x100)
	{
		// RAM cart selected.
		sprintf(Str_Tmp, "OptionsMenu_SegaCDSRAMSize_SubMenu_%d", BRAM_Ex_Size);
	}
	else
	{
		// No RAM cart selected.
		strcpy(Str_Tmp, "OptionsMenu_SegaCDSRAMSize_SubMenu_None");
	}
	SRAMSize = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(SRAMSize), TRUE);
	
	// Enable callbacks.
	do_callbacks = 1;
}
