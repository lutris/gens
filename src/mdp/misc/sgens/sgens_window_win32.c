/***************************************************************************
 * Gens: [MDP] Sonic Gens. (Window Code) (Win32)                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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

#include <stdint.h>
#include <stdio.h>

#include "sgens_window.h"
#include "sgens_plugin.h"
#include "sgens.hpp"

// sGens ROM type information and widget information.
#include "sgens_rom_type.h"
#include "sgens_widget_info.h"

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_win32.h"

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// Window.
static HWND	sgens_window = NULL;

// Window class.
static WNDCLASS	sgens_wndclass;

// Window size.
#define SGENS_WINDOW_WIDTH  640
#define SGENS_WINDOW_HEIGHT 480

// Instance and font.
static HINSTANCE sgens_hinstance;
static HFONT sgens_hfont = NULL;

// Widgets.
static HWND	lblLoadedGame;
static HWND	lblLevelInfo_Zone;
static HWND	lblLevelInfo_Act;

static HWND	lblLevelInfo_Desc[LEVEL_INFO_COUNT];
static HWND	lblLevelInfo[LEVEL_INFO_COUNT];

static HWND	lblPlayerInfo_Desc[PLAYER_INFO_COUNT];
static HWND	lblPlayerInfo[PLAYER_INFO_COUNT];

// Window procedure.
static LRESULT CALLBACK sgens_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widget creation functions.
static void	sgens_window_create_child_windows(HWND hWnd);
static void	sgens_window_create_level_info_frame(HWND container);
static void	sgens_window_create_player_info_frame(HWND container);


/**
 * sgens_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void MDP_FNCALL sgens_window_show(void *parent)
{
	if (sgens_window)
	{
		// Sonic Gens window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(sgens_window, SW_SHOW);
		return;
	}
	
	// Get the HINSTANCE.
	sgens_hinstance = GetModuleHandle(NULL);
	
	if (sgens_wndclass.lpfnWndProc != sgens_window_wndproc)
	{
		// Create the window class.
		sgens_wndclass.style = 0;
		sgens_wndclass.lpfnWndProc = sgens_window_wndproc;
		sgens_wndclass.cbClsExtra = 0;
		sgens_wndclass.cbWndExtra = 0;
		sgens_wndclass.hInstance = sgens_hinstance;
		sgens_wndclass.hIcon = NULL;
		sgens_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		sgens_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		sgens_wndclass.lpszMenuName = NULL;
		sgens_wndclass.lpszClassName = "mdp_misc_sgens_window";
		
		RegisterClass(&sgens_wndclass);
	}
	
	// Create the font.
	sgens_hfont = mdp_win32_get_message_font();
	
	// Create the window.
	sgens_window = CreateWindow("mdp_misc_sgens_window", "Sonic Gens",
				    WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    SGENS_WINDOW_WIDTH, SGENS_WINDOW_HEIGHT,
				    (HWND)parent, NULL, sgens_hinstance, NULL);
	
	// Window adjustment.
	mdp_win32_set_actual_window_size(sgens_window, SGENS_WINDOW_WIDTH, SGENS_WINDOW_HEIGHT);
	mdp_win32_center_on_window(sgens_window, (HWND)parent);
	
	UpdateWindow(sgens_window);
	ShowWindow(sgens_window, TRUE);
	
	// Register the window with MDP Host Services.
	sgens_host_srv->window_register(&mdp, sgens_window);
}


/**
 * sgens_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void sgens_window_create_child_windows(HWND hWnd)
{
#if 0
	// Create the label for the loaded game.
	lblLoadedGame = gtk_label_new(NULL);
	gtk_widget_set_name(lblLoadedGame, "lblLoadedGame");
	gtk_misc_set_alignment(GTK_MISC(lblLoadedGame), 0.5f, 0.0f);
	gtk_label_set_justify(GTK_LABEL(lblLoadedGame), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblLoadedGame);
	gtk_box_pack_start(GTK_BOX(vboxDialog), lblLoadedGame, FALSE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(sgens_window), "lblLoadedGame",
			       g_object_ref(lblLoadedGame), (GDestroyNotify)g_object_unref);
	
	// Create the "Level Information" frame.
	sgens_window_create_level_info_frame(vboxDialog);
	
	// Create the "Player Information" frame.
	sgens_window_create_player_info_frame(vboxDialog);
	
	// Create the dialog buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(sgens_window),
			       "gtk-close", GTK_RESPONSE_CLOSE,
			       NULL);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(sgens_window), GTK_WINDOW(parent));
	
	// Update the current ROM type and information display.
	sgens_window_update_rom_type();
	sgens_window_update();
#endif
}


/**
 * sgens_window_create_level_info_frame(): Create the "Level Information" frame.
 * @param container Container for the frame.
 */
static void sgens_window_create_level_info_frame(HWND container)
{
#if 0
	// "Level Information" frame.
	HWND fraLevelInfo = gtk_frame_new("Level Information");
	gtk_widget_set_name(fraLevelInfo, "fraLevelInfo");
	gtk_frame_set_shadow_type(GTK_FRAME(fraLevelInfo), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(fraLevelInfo);
	gtk_box_pack_start(GTK_BOX(container), fraLevelInfo, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "fraLevelInfo",
			       g_object_ref(fraLevelInfo), (GDestroyNotify)g_object_unref);
	
	// Create a VBox for the frame.
	HWND vboxLevelInfo = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_name(vboxLevelInfo, "vboxLevelInfo");
	gtk_container_set_border_width(GTK_CONTAINER(vboxLevelInfo), 0);
	gtk_widget_show(vboxLevelInfo);
	gtk_container_add(GTK_CONTAINER(fraLevelInfo), vboxLevelInfo);
	g_object_set_data_full(G_OBJECT(container), "vboxLevelInfo",
			       g_object_ref(vboxLevelInfo), (GDestroyNotify)g_object_unref);
	
	// "Zone" information label.
	lblLevelInfo_Zone = gtk_label_new("Zone");
	gtk_widget_set_name(lblLevelInfo_Zone, "lblLevelInfo_Zone");
	gtk_misc_set_alignment(GTK_MISC(lblLevelInfo_Zone), 0.5f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblLevelInfo_Zone), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblLevelInfo_Zone);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), lblLevelInfo_Zone, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "lblLevelInfo_Zone",
			       g_object_ref(lblLevelInfo_Zone), (GDestroyNotify)g_object_unref);
	
	// "Act" information label.
	lblLevelInfo_Act = gtk_label_new("Act");
	gtk_widget_set_name(lblLevelInfo_Act, "lblLevelInfo_Act");
	gtk_misc_set_alignment(GTK_MISC(lblLevelInfo_Act), 0.5f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblLevelInfo_Act), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblLevelInfo_Act);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), lblLevelInfo_Act, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "lblLevelInfo_Act",
			       g_object_ref(lblLevelInfo_Act), (GDestroyNotify)g_object_unref);
	
	// Table for level information.
	HWND tblLevelInfo = gtk_table_new(5, 4, FALSE);
	gtk_widget_set_name(tblLevelInfo, "tblLevelInfo");
	gtk_container_set_border_width(GTK_CONTAINER(tblLevelInfo), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tblLevelInfo), 16);
	gtk_table_set_col_spacing(GTK_TABLE(tblLevelInfo), 3, 8);
	gtk_widget_show(tblLevelInfo);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), tblLevelInfo, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "tblLevelInfo",
			       g_object_ref(tblLevelInfo), (GDestroyNotify)g_object_unref);
	
	// Add the level information widgets.
	unsigned int i;
	char tmp[64];
	for (i = 0; i < LEVEL_INFO_COUNT; i++)
	{
		// Determine the column starting and ending positions.
		int start_col, end_col;
		if (level_info[i].fill_all_cols)
		{
			start_col = 0;
			end_col = 3;
		}
		else
		{
			start_col = (level_info[i].column * 2);
			end_col = (level_info[i].column * 2) + 1;
		}
		
		// Description label.
		lblLevelInfo_Desc[i] = gtk_label_new(level_info[i].description);
		sprintf(tmp, "lblLevelInfo_Desc_%d", i);
		gtk_widget_set_name(lblLevelInfo_Desc[i], tmp);
		gtk_misc_set_alignment(GTK_MISC(lblLevelInfo_Desc[i]), 0.0f, 0.5f);
		gtk_widget_show(lblLevelInfo_Desc[i]);
		gtk_table_attach(GTK_TABLE(tblLevelInfo), lblLevelInfo_Desc[i],
				 start_col, end_col,
				 level_info[i].row, level_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblLevelInfo_Desc[i]), (GDestroyNotify)g_object_unref);
		
		// Information label.
		sprintf(tmp, "<tt>%s</tt>", level_info[i].initial);
		lblLevelInfo[i] = gtk_label_new(tmp);
		sprintf(tmp, "lblLevelInfo_%d", i);
		gtk_widget_set_name(lblLevelInfo[i], tmp);
		gtk_misc_set_alignment(GTK_MISC(lblLevelInfo[i]), 1.0f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblLevelInfo[i]), GTK_JUSTIFY_RIGHT);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[i]), TRUE);
		gtk_widget_show(lblLevelInfo[i]);
		gtk_table_attach(GTK_TABLE(tblLevelInfo), lblLevelInfo[i],
				 start_col + 1, end_col + 1,
				 level_info[i].row, level_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblLevelInfo[i]), (GDestroyNotify)g_object_unref);
	}
#endif
}


/**
 * sgens_window_create_player_info_frame(): Create the "Player Information" frame.
 * @param container Container for the frame.
 */
static void sgens_window_create_player_info_frame(HWND container)
{
#if 0
	// "Player Information" frame.
	HWND fraPlayerInfo = gtk_frame_new("Player Information");
	gtk_widget_set_name(fraPlayerInfo, "fraPlayerInfo");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPlayerInfo), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(fraPlayerInfo);
	gtk_box_pack_start(GTK_BOX(container), fraPlayerInfo, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "fraPlayerInfo",
			       g_object_ref(fraPlayerInfo), (GDestroyNotify)g_object_unref);
	
	// Table for player information.
	HWND tblPlayerInfo = gtk_table_new(3, 2, FALSE);
	gtk_widget_set_name(tblPlayerInfo, "tblPlayerInfo");
	gtk_container_set_border_width(GTK_CONTAINER(tblPlayerInfo), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tblPlayerInfo), 16);
	gtk_widget_show(tblPlayerInfo);
	gtk_container_add(GTK_CONTAINER(fraPlayerInfo), tblPlayerInfo);
	g_object_set_data_full(G_OBJECT(container), "tblPlayerInfo",
			       g_object_ref(tblPlayerInfo), (GDestroyNotify)g_object_unref);
	
	// Add the player information widgets.
	unsigned int i;
	char tmp[64];
	for (i = 0; i < PLAYER_INFO_COUNT; i++)
	{
		// Description label.
		lblPlayerInfo_Desc[i] = gtk_label_new(player_info[i].description);
		sprintf(tmp, "lblPlayerInfo_Desc_%d", i);
		gtk_widget_set_name(lblPlayerInfo_Desc[i], tmp);
		gtk_misc_set_alignment(GTK_MISC(lblPlayerInfo_Desc[i]), 0.0f, 0.5f);
		gtk_widget_show(lblPlayerInfo_Desc[i]);
		gtk_table_attach(GTK_TABLE(tblPlayerInfo), lblPlayerInfo_Desc[i],
				 0, 1,
				 player_info[i].row, player_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblPlayerInfo_Desc[i]), (GDestroyNotify)g_object_unref);
		
		// Information label.
		sprintf(tmp, "<tt>%s</tt>", player_info[i].initial);
		lblPlayerInfo[i] = gtk_label_new(tmp);
		sprintf(tmp, "lblPlayerInfo_%d", i);
		gtk_widget_set_name(lblPlayerInfo[i], tmp);
		gtk_misc_set_alignment(GTK_MISC(lblPlayerInfo[i]), 1.0f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblPlayerInfo[i]), GTK_JUSTIFY_RIGHT);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[i]), TRUE);
		gtk_widget_show(lblPlayerInfo[i]);
		gtk_table_attach(GTK_TABLE(tblPlayerInfo), lblPlayerInfo[i],
				 1, 2,
				 player_info[i].row, player_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblPlayerInfo[i]), (GDestroyNotify)g_object_unref);
	}
#endif
}


/**
 * sgens_window_close(): Close the VDP Layer Options window.
 */
void MDP_FNCALL sgens_window_close(void)
{
	if (!sgens_window)
		return;
	
	// Unregister the window from MDP Host Services.
	sgens_host_srv->window_unregister(&mdp, sgens_window);
	
	// Destroy the window.
	DestroyWindow(sgens_window);
	sgens_window = NULL;
}


/**
 * sgens_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK sgens_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			sgens_window_create_child_windows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		
		case WM_COMMAND:
			// TODO
			break;
		
		case WM_DESTROY:
			if (hWnd != sgens_window)
				break;
			
			sgens_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * sgens_window_update_rom_type(): Update the current ROM type.
 */
void MDP_FNCALL sgens_window_update_rom_type(void)
{
#if 0
	if (!sgens_window)
		return;
	
	if (sgens_current_rom_type < SGENS_ROM_TYPE_NONE ||
	    sgens_current_rom_type >= SGENS_ROM_TYPE_MAX)
	{
		// Invalid ROM type. Assume SGENS_ROM_TYPE_UNSUPPORTED.
		sgens_current_rom_type = SGENS_ROM_TYPE_UNSUPPORTED;
	}
	
	// Set the "Loaded Game" label.
	gtk_label_set_text(GTK_LABEL(lblLoadedGame), sgens_ROM_type_name[sgens_current_rom_type]);
	
	// Reset the "Rings for Perfect Bonus" information label.
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT]), "<tt>0</tt>");
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT]), TRUE);
	
	// Enable/Disable the "Rings for Perfect Bonus" labels, depending on ROM type.
	gboolean isS2 = (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC2_REV00 &&
			 sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC2_REV02);
	gtk_widget_set_sensitive(lblLevelInfo_Desc[LEVEL_INFO_RINGS_PERFECT], isS2);
	gtk_widget_set_sensitive(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT], isS2);
#endif
}


/**
 * sgens_window_update(): Update the information display.
 */
void MDP_FNCALL sgens_window_update(void)
{
#if 0
	if (!sgens_window)
		return;
	
	if (sgens_current_rom_type <= SGENS_ROM_TYPE_UNSUPPORTED)
		return;
	
	char tmp[64];
	
	// Values common to all supported Sonic games.
	
	// Score.
	sprintf(tmp, "<tt>%d</tt>", (MDP_MEM_32(sgens_md_RAM, 0xFE26) * 10));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_SCORE]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_SCORE]), TRUE);
	
	// Time.
	sprintf(tmp, "<tt>%02d:%02d:%02d</tt>",
		MDP_MEM_BE_8(sgens_md_RAM, 0xFE23),
		MDP_MEM_BE_8(sgens_md_RAM, 0xFE24),
		MDP_MEM_BE_8(sgens_md_RAM, 0xFE25));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_TIME]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_TIME]), TRUE);
	
	// Rings.
	sprintf(tmp, "<tt>%d</tt>", MDP_MEM_16(sgens_md_RAM, 0xFE20));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS]), TRUE);
	
	// Lives.
	sprintf(tmp, "<tt>%d</tt>", MDP_MEM_BE_8(sgens_md_RAM, 0xFE12));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_LIVES]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_LIVES]), TRUE);
	
	// Continues.
	sprintf(tmp, "<tt>%d</tt>", MDP_MEM_BE_8(sgens_md_RAM, 0xFE18));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CONTINUES]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CONTINUES]), TRUE);
	
	// Rings remaining for Perfect Bonus.
	// This is only applicable for Sonic 2.
	if (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC2_REV00 &&
	    sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC2_REV02)
	{
		sprintf(tmp, "<tt>%d</tt>", MDP_MEM_16(sgens_md_RAM, 0xFF40));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT]), TRUE);
	}
	
	// Water status.
	uint16_t water_level = MDP_MEM_16(sgens_md_RAM, 0xF648);
	sprintf(tmp, "<tt>%s</tt>", (MDP_MEM_16(sgens_md_RAM, 0xF648) != 0 ? "ON" : "OFF"));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_WATER_ENABLED]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_WATER_ENABLED]), TRUE);
	
	// Water level.
	sprintf(tmp, "<tt>%04X</tt>", water_level);
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_WATER_LEVEL]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_WATER_LEVEL]), TRUE);
	
	// TODO: Camera position and player position don't seem to be working
	// correctly with Sonic 3, S&K, etc.
	
	if (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC1_REV00 &&
	    sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC1_REVXB)
	{
		// S1-specific information.
		
		// Number of emeralds.
		sprintf(tmp, "<tt>%d</tt>", MDP_MEM_BE_8(sgens_md_RAM, 0xFE57));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), TRUE);
		
		// Camera X position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xFF10));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), TRUE);
		
		// Camera Y position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xFF14));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), TRUE);
		
		// Player angle.
		uint16_t angle = (MDP_MEM_BE_8(sgens_md_RAM, 0xD026) | (MDP_MEM_BE_8(sgens_md_RAM, 0xD027) << 8));
		sprintf(tmp, "<tt>%0.02f°</tt>", ((double)(angle) * 1.40625));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), TRUE);
		
		// Player X position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xD008));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), TRUE);
		
		// Player Y position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xD00C));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), TRUE);
	}
	else
	{
		// Information for games other than S1.
		
		// Number of emeralds.
		sprintf(tmp, "<tt>%d</tt>", MDP_MEM_BE_8(sgens_md_RAM, 0xFEB1));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), TRUE);
		
		// Camera X position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xEE00));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), TRUE);
		
		// Camera Y position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xEE04));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), TRUE);
		
		// Player angle.
		uint16_t angle = (MDP_MEM_BE_8(sgens_md_RAM, 0xB026) | (MDP_MEM_BE_8(sgens_md_RAM, 0xB027) << 8));
		sprintf(tmp, "<tt>%0.02f°</tt>", ((double)(angle) * 1.40625));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), TRUE);
		
		// Player X position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xB008));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), TRUE);
		
		// Player Y position.
		sprintf(tmp, "<tt>%04X</tt>", MDP_MEM_16(sgens_md_RAM, 0xB00C));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), TRUE);
	}
	
	// sGens window has been updated.
	return;
#endif
}
