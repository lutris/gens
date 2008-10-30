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

#include <string.h>

#include "gens_window.hpp"
#include "gens_window_sync.hpp"
#include "ui/common/gens/gens_menu.h"

#include "gtk-misc.h"

#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "segacd/cd_sys.hpp"
#include "util/sound/gym.hpp"

// Renderer / Blitter selection stuff.
#include "gens_core/gfx/renderers.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/misc.h"

// C++ includes
#include <string>
using std::string;


// Macro to create a menu item with radio buttons.
#define NewMenuItem_Radio(MenuItemWidget, MenuItemCaption, MenuItemName, Container, State, RadioGroup)	\
{												\
	MenuItemWidget = gtk_radio_menu_item_new_with_mnemonic(RadioGroup, (MenuItemCaption));	\
	RadioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(MenuItemWidget));	\
	gtk_widget_set_name(MenuItemWidget, MenuItemName);					\
	gtk_widget_show(MenuItemWidget);							\
	gtk_container_add(GTK_CONTAINER(Container), MenuItemWidget);				\
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MenuItemWidget), State);		\
	GLADE_HOOKUP_OBJECT(gens_window, MenuItemWidget, MenuItemName);				\
}


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
	// ROM Format prefixes
	// TODO: Move this somewhere else.
	const char* ROM_Format_Prefix[5] = {"[----]", "[MD]", "[32X]", "[SCD]", "[SCDX]"};
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// ROM History
	GtkWidget *mnuROMHistory = findMenuItem(IDM_FILE_ROMHISTORY);
	
	// Check if the ROM History submenu already exists.
	GtkWidget *mnuROMHistory_sub = gtk_menu_item_get_submenu(GTK_MENU_ITEM(mnuROMHistory));
	if (mnuROMHistory_sub)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuROMHistory_sub);
	}
	
	// Create a new submenu.
	mnuROMHistory_sub = gtk_menu_new();
	gtk_widget_set_name(mnuROMHistory_sub, "FileMenu_ROMHistory_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mnuROMHistory), mnuROMHistory_sub);
	
	g_object_set_data_full(G_OBJECT(mnuROMHistory), "FileMenu_ROMHistory_SubMenu",
			       g_object_ref(mnuROMHistory_sub),
			       (GDestroyNotify)g_object_unref);
	
	GtkWidget *mnuROMHistory_item;
	string sROMHistoryEntry;
	char sTmpROMFilename[GENS_PATH_MAX];
	char sMenuKey[24];
	int romFormat;
	int romsFound = 0;
	for (unsigned short i = 0; i < 9; i++)
	{
		// Make sure this Recent ROM entry actually has an entry.
		if (strlen(Recent_Rom[i]) == 0)
			continue;
		
		// Increment the ROMs Found counter.
		romsFound++;
		
		// Determine the ROM format.
		// TODO: Improve the return variable from Detect_Format()
		romFormat = ROM::detectFormat_fopen(Recent_Rom[i]) >> 1;
		if (romFormat >= 1 && romFormat <= 4)
			sROMHistoryEntry = ROM_Format_Prefix[romFormat];
		else
			sROMHistoryEntry = ROM_Format_Prefix[0];
		
		// Add a tab, a dash, and a space.
		sROMHistoryEntry += "\t- ";
		
		// Get the ROM filename.
		ROM::getNameFromPath(Recent_Rom[i], sTmpROMFilename);
		sROMHistoryEntry += sTmpROMFilename;
		
		// Add the ROM item to the ROM History submenu.
		mnuROMHistory_item = gtk_menu_item_new_with_label(sROMHistoryEntry.c_str());
		gtk_widget_show(mnuROMHistory_item);
		gtk_container_add(GTK_CONTAINER(mnuROMHistory_sub), mnuROMHistory_item);
		
		// Make sure the menu item is deleted when the submenu is deleted.
		sprintf(sMenuKey, "ROMHistory_Sub_%d", i);
		g_object_set_data_full(G_OBJECT(mnuROMHistory_sub), sMenuKey,
				       g_object_ref(mnuROMHistory_item),
				       (GDestroyNotify)g_object_unref);
		
		// Connect the signal.
		g_signal_connect((gpointer)mnuROMHistory_item, "activate",
				 G_CALLBACK(GensWindow_GTK_MenuItemCallback),
					    GINT_TO_POINTER(IDM_FILE_ROMHISTORY_0 + i));
	}
	
	// If no recent ROMs were found, disable the ROM History menu.
	gtk_widget_set_sensitive(mnuROMHistory, romsFound);
	
	// Savestate menu items
	gboolean saveStateEnable = (Game != NULL);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_LOADSTATE), saveStateEnable);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_SAVESTATE), saveStateEnable);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_QUICKLOAD), saveStateEnable);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_QUICKSAVE), saveStateEnable);
	
	// Current savestate
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_FILE_CHANGESTATE_0 + Current_State)), TRUE);
	
	// Disable "Close ROM" if no ROM is loaded.
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_CLOSEROM), (Game != NULL));
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	uint16_t id;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Simple checkbox items
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_VSYNC)), Video.VSync_W);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_STRETCH)), draw->stretch());
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_SPRITELIMIT)), Sprite_Over);
	
	// Bits per pixel
	switch (bpp)
	{
		case 15:
			id = IDM_GRAPHICS_BPP_15;
			break;
		case 16:
			id = IDM_GRAPHICS_BPP_16;
			break;
		case 32:
			id = IDM_GRAPHICS_BPP_32;
			break;
		default:
			id = 0;
			break;
	}
	
	if (id != 0)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(id)), TRUE);
	
	// Rebuild the Render submenu
	Sync_Gens_Window_GraphicsMenu_Render(findMenuItem(IDM_GRAPHICS_RENDER));
	
	// Selected Render Mode
	int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_RENDER_NORMAL + rendMode)), TRUE);
	
	// Frame Skip
	id = (IDM_GRAPHICS_FRAMESKIP_AUTO + 1) + Frame_Skip;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(id)), TRUE);
	
	// Screen Shot
	gtk_widget_set_sensitive(findMenuItem(IDM_GRAPHICS_STRETCH), (Game != NULL));
	
#ifdef GENS_OPENGL
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_OPENGL)), Video.OpenGL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_OPENGL_FILTER)), Video.glLinearFilter);
	
	// OpenGL Resolution
	
	if (Video.Width_GL == 320 && Video.Height_GL == 240)
		id = IDM_GRAPHICS_OPENGL_RES_320;
	else if (Video.Width_GL == 640 && Video.Height_GL == 480)
		id = IDM_GRAPHICS_OPENGL_RES_640;
	else if (Video.Width_GL == 800 && Video.Height_GL == 600)
		id = IDM_GRAPHICS_OPENGL_RES_800;
	else if (Video.Width_GL == 1024 && Video.Height_GL == 768)
		id = IDM_GRAPHICS_OPENGL_RES_1024;
	else
		id = IDM_GRAPHICS_OPENGL_RES_CUSTOM;
	
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(id)), TRUE);
	
	// Set the text of the custom resolution item.
	GtkWidget *mnuGLResCustom = findMenuItem(IDM_GRAPHICS_OPENGL_RES_CUSTOM);
	if (id == IDM_GRAPHICS_OPENGL_RES_CUSTOM)
	{
		// Custom resolution. Set the text.
		char sCustomRes[32];
		sprintf(sCustomRes, "Custom... (%dx%d)", Video.Width_GL, Video.Height_GL);
		gtk_label_set_text(GTK_LABEL(GTK_BIN(mnuGLResCustom)->child), sCustomRes);
	}
	else
	{
		// Predefined resolution.
		gtk_label_set_text(GTK_LABEL(GTK_BIN(mnuGLResCustom)->child), "Custom...");
	}
#endif
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * @param container Container for this menu.
 */
void Sync_Gens_Window_GraphicsMenu_Render(GtkWidget *container)
{
	if (!container)
		return;
	
	GtkWidget *mnuItem;
	GSList *radioGroup = NULL;
	gboolean showRenderer;
	
	int i;
	char sObjName[64];
	
	// Check if the Render submenu already exists.
	GtkWidget *mnuSubMenu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(container));
	if (mnuSubMenu)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuSubMenu);
	}
	
	// Create a new submenu.
	mnuSubMenu = gtk_menu_new();
	gtk_widget_set_name(mnuSubMenu, "GraphicsMenu_Render_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), mnuSubMenu);
	
	g_object_set_data_full(G_OBJECT(container), "GraphicsMenu_Render_SubMenu",
			       g_object_ref(mnuSubMenu),
			       (GDestroyNotify)g_object_unref);
	
	// Create the render entries.
	i = 0;
	while (Renderers[i].name)
	{
		// Delete the menu item from the map, if it exists.
		gensMenuMap.erase(IDM_GRAPHICS_RENDER_NORMAL + i);
		
		// Check if the current blitter exists for this video mode.
		showRenderer = FALSE;
		if (bpp == 32)
		{
			// 32-bit
			if (Have_MMX && Renderers[i].blit_32_mmx)
				showRenderer = TRUE;
			else if (Renderers[i].blit_32)
				showRenderer = TRUE;
		}
		else // if (bpp == 15 || bpp == 16)
		{
			// 15/16-bit
			if (Have_MMX && Renderers[i].blit_16_mmx)
				showRenderer = TRUE;
			else if (Renderers[i].blit_16)
				showRenderer = TRUE;
		}
		
		if (showRenderer)
		{
			sprintf(sObjName, "GraphicsMenu_Render_SubMenu_%d", i);
			
			mnuItem = gtk_radio_menu_item_new_with_mnemonic(radioGroup, Renderers[i].name);
			radioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mnuItem));
			gtk_widget_set_name(mnuItem, sObjName);
			gtk_widget_show(mnuItem);
			gtk_container_add(GTK_CONTAINER(mnuSubMenu), mnuItem);
			
			// Make sure the menu item is deleted when the submenu is deleted.
			g_object_set_data_full(G_OBJECT(mnuSubMenu), sObjName,
					       g_object_ref(mnuItem),
					       (GDestroyNotify)g_object_unref);
			
			// Connect the signal.
			g_signal_connect((gpointer)mnuItem, "activate",
					  G_CALLBACK(GensWindow_GTK_MenuItemCallback),
					  GINT_TO_POINTER(IDM_GRAPHICS_RENDER_NORMAL + i));
			
			// Add the menu item to the map.
			gensMenuMap.insert(gtkMenuMapItem(IDM_GRAPHICS_RENDER_NORMAL + i, mnuItem));
		}
		
		// Check the next renderer.
		i++;
	}
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
#if 0
#ifdef GENS_DEBUGGER
	GtkWidget *MItem_Debug;
	GtkWidget *MItem_Debug_Item;
	int i, checkDebug;
#endif /* GENS_DEBUGGER */
	GtkWidget *MItem_Country;
	GtkWidget *MItem_Reset68K, *MItem_ResetM68K, *MItem_ResetS68K;
	GtkWidget *MItem_ResetMSH2, *MItem_ResetSSH2;
	GtkWidget *MItem_SegaCD_PerfectSync;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
#ifdef GENS_DEBUGGER
	MItem_Debug = lookup_widget(gens_window, "CPUMenu_Debug");
	gtk_widget_set_sensitive(MItem_Debug, (Game != NULL));
	
	// Hide/Show debug entries depending on the active console.
	
	if (Game != NULL)
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
#endif /* GENS_DEBUGGER */

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
#endif
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
#if 0
	GtkWidget *MItem_Enable, *MItem_Rate, *MItem_Stereo, *MItem_Z80;
	GtkWidget *MItem_YM2612, *MItem_YM2612_Improved;
	GtkWidget *MItem_DAC, *MItem_DAC_Improved;
	GtkWidget *MItem_PSG, *MItem_PSG_Improved;
	GtkWidget *MItem_PCM, *MItem_PWM, *MItem_CDDA;
	
	GtkWidget *MItem_GYMDump, *MItem_WAVDump;
	string label; int allowAudioDump;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Simple checkbox items
	MItem_Enable = lookup_widget(gens_window, "SoundMenu_Enable");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Enable), audio->enabled());
	MItem_Stereo = lookup_widget(gens_window, "SoundMenu_Stereo");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Stereo), audio->stereo());
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
	sprintf(Str_Tmp, "SoundMenu_Rate_SubMenu_%d", audio->soundRate());
	MItem_Rate = lookup_widget(gens_window, Str_Tmp);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(MItem_Rate), TRUE);
	
	// GYM dumping
	label = (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump");
	MItem_GYMDump = lookup_widget(gens_window, "SoundMenu_GYMDump");
	gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_GYMDump)->child), label.c_str());
	
	// WAV dumping
	label = (audio->dumpingWAV() ? "Stop WAV Dump" : "Start WAV Dump");
	MItem_WAVDump = lookup_widget(gens_window, "SoundMenu_WAVDump");
	gtk_label_set_text(GTK_LABEL(GTK_BIN(MItem_WAVDump)->child), label.c_str());
	
	// Enable or disable items, depending on the Enable state.
	gtk_widget_set_sensitive(MItem_Stereo, audio->enabled());
	gtk_widget_set_sensitive(MItem_YM2612, audio->enabled());
	gtk_widget_set_sensitive(MItem_YM2612_Improved, audio->enabled());
	gtk_widget_set_sensitive(MItem_DAC, audio->enabled());
	gtk_widget_set_sensitive(MItem_DAC_Improved, audio->enabled());
	gtk_widget_set_sensitive(MItem_PSG, audio->enabled());
	gtk_widget_set_sensitive(MItem_PSG_Improved, audio->enabled());
	gtk_widget_set_sensitive(MItem_PCM, audio->enabled());
	gtk_widget_set_sensitive(MItem_PWM, audio->enabled());
	gtk_widget_set_sensitive(MItem_CDDA, audio->enabled());
	
	// Enable or disable GYM/WAV dumping, depending on if a game is running or not.
	// Also, don't enable this if sound is disabled.
	allowAudioDump = (Game != NULL) && audio->enabled();
	gtk_widget_set_sensitive(MItem_GYMDump, allowAudioDump);
	// TODO: Change from FALSE to allowAudioDump after WAV dumping has been reimplemented.
	gtk_widget_set_sensitive(MItem_WAVDump, FALSE);
	
	// Enable callbacks.
	do_callbacks = 1;
#endif
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
#if 0
	GtkWidget *SRAMSize;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// SegaCD SRAM Size
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
#endif
}
