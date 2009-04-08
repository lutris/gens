/***************************************************************************
 * Gens: [MDP] Sonic Gens. (Window Code) (GTK+)                            *
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

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// C includes.
#include <stdint.h>
#include <stdio.h>

#include "sgens_window.h"
#include "sgens_plugin.h"
#include "sgens.h"

// sGens ROM type information and widget information.
#include "sgens_rom_type.h"
#include "sgens_widget_info.h"

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// Window.
static GtkWidget *sgens_window = NULL;

// Widgets.
static GtkWidget *lblLoadedGame;
static GtkWidget *lblLevelInfo_Zone;
static GtkWidget *lblLevelInfo_Act;

static GtkWidget *lblLevelInfo_Desc[LEVEL_INFO_COUNT];
static GtkWidget *lblLevelInfo[LEVEL_INFO_COUNT];

static GtkWidget *lblPlayerInfo_Desc[PLAYER_INFO_COUNT];
static GtkWidget *lblPlayerInfo[PLAYER_INFO_COUNT];

// Widget creation functions.
static void	sgens_window_create_level_info_frame(GtkWidget *container);
static void	sgens_window_create_player_info_frame(GtkWidget *container);

// Callbacks.
static gboolean	sgens_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	sgens_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);


/**
 * sgens_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void MDP_FNCALL sgens_window_show(void *parent)
{
	if (sgens_window)
	{
		// Sonic Gens window is already visible. Set focus.
		gtk_widget_grab_focus(sgens_window);
		return;
	}
	
	// Create the window.
	sgens_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(sgens_window), 4);
	gtk_window_set_title(GTK_WINDOW(sgens_window), "Sonic Gens");
	gtk_window_set_position(GTK_WINDOW(sgens_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(sgens_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(sgens_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(sgens_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)sgens_window, "delete_event",
			 G_CALLBACK(sgens_window_callback_close), NULL);
	g_signal_connect((gpointer)sgens_window, "destroy_event",
			 G_CALLBACK(sgens_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)sgens_window, "response",
			 G_CALLBACK(sgens_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(sgens_window));
	gtk_box_set_spacing(GTK_BOX(vboxDialog), 8);
	gtk_widget_show(vboxDialog);
	
	// Create the label for the loaded game.
	lblLoadedGame = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(lblLoadedGame), 0.5f, 0.0f);
	gtk_label_set_justify(GTK_LABEL(lblLoadedGame), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblLoadedGame);
	gtk_box_pack_start(GTK_BOX(vboxDialog), lblLoadedGame, FALSE, TRUE, 0);
	
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
	
	// Show the window.
	gtk_widget_show_all(sgens_window);
	
	// Register the window with MDP Host Services.
	sgens_host_srv->window_register(&mdp, sgens_window);
}


/**
 * sgens_window_create_level_info_frame(): Create the "Level Information" frame.
 * @param container Container for the frame.
 */
static void sgens_window_create_level_info_frame(GtkWidget *container)
{
	// "Level Information" frame.
	GtkWidget *fraLevelInfo = gtk_frame_new("Level Information");
	gtk_frame_set_shadow_type(GTK_FRAME(fraLevelInfo), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(fraLevelInfo);
	gtk_box_pack_start(GTK_BOX(container), fraLevelInfo, TRUE, TRUE, 0);
	
	// Create a VBox for the frame.
	GtkWidget *vboxLevelInfo = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vboxLevelInfo), 0);
	gtk_widget_show(vboxLevelInfo);
	gtk_container_add(GTK_CONTAINER(fraLevelInfo), vboxLevelInfo);
	
	// "Zone" information label.
	lblLevelInfo_Zone = gtk_label_new("Zone");
	gtk_misc_set_alignment(GTK_MISC(lblLevelInfo_Zone), 0.5f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblLevelInfo_Zone), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblLevelInfo_Zone);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), lblLevelInfo_Zone, TRUE, TRUE, 0);
	
	// "Act" information label.
	lblLevelInfo_Act = gtk_label_new("Act");
	gtk_misc_set_alignment(GTK_MISC(lblLevelInfo_Act), 0.5f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblLevelInfo_Act), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblLevelInfo_Act);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), lblLevelInfo_Act, TRUE, TRUE, 0);
	
	// Table for level information.
	GtkWidget *tblLevelInfo = gtk_table_new(5, 4, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(tblLevelInfo), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tblLevelInfo), 16);
	gtk_table_set_col_spacing(GTK_TABLE(tblLevelInfo), 3, 8);
	gtk_widget_show(tblLevelInfo);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), tblLevelInfo, TRUE, TRUE, 0);
	
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
		gtk_misc_set_alignment(GTK_MISC(lblLevelInfo_Desc[i]), 0.0f, 0.5f);
		gtk_widget_show(lblLevelInfo_Desc[i]);
		gtk_table_attach(GTK_TABLE(tblLevelInfo), lblLevelInfo_Desc[i],
				 start_col, end_col,
				 level_info[i].row, level_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		
		// Information label.
		sprintf(tmp, "<tt>%s</tt>", level_info[i].initial);
		lblLevelInfo[i] = gtk_label_new(tmp);
		gtk_misc_set_alignment(GTK_MISC(lblLevelInfo[i]), 1.0f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblLevelInfo[i]), GTK_JUSTIFY_RIGHT);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[i]), TRUE);
		gtk_widget_show(lblLevelInfo[i]);
		gtk_table_attach(GTK_TABLE(tblLevelInfo), lblLevelInfo[i],
				 start_col + 1, end_col + 1,
				 level_info[i].row, level_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
	}
}


/**
 * sgens_window_create_player_info_frame(): Create the "Player Information" frame.
 * @param container Container for the frame.
 */
static void sgens_window_create_player_info_frame(GtkWidget *container)
{
	// "Player Information" frame.
	GtkWidget *fraPlayerInfo = gtk_frame_new("Player Information");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPlayerInfo), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(fraPlayerInfo);
	gtk_box_pack_start(GTK_BOX(container), fraPlayerInfo, TRUE, TRUE, 0);
	
	// Table for player information.
	GtkWidget *tblPlayerInfo = gtk_table_new(3, 2, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(tblPlayerInfo), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tblPlayerInfo), 16);
	gtk_widget_show(tblPlayerInfo);
	gtk_container_add(GTK_CONTAINER(fraPlayerInfo), tblPlayerInfo);
	
	// Add the player information widgets.
	unsigned int i;
	char tmp[64];
	for (i = 0; i < PLAYER_INFO_COUNT; i++)
	{
		// Description label.
		lblPlayerInfo_Desc[i] = gtk_label_new(player_info[i].description);
		gtk_misc_set_alignment(GTK_MISC(lblPlayerInfo_Desc[i]), 0.0f, 0.5f);
		gtk_widget_show(lblPlayerInfo_Desc[i]);
		gtk_table_attach(GTK_TABLE(tblPlayerInfo), lblPlayerInfo_Desc[i],
				 0, 1,
				 player_info[i].row, player_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		
		// Information label.
		sprintf(tmp, "<tt>%s</tt>", player_info[i].initial);
		lblPlayerInfo[i] = gtk_label_new(tmp);
		gtk_misc_set_alignment(GTK_MISC(lblPlayerInfo[i]), 1.0f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblPlayerInfo[i]), GTK_JUSTIFY_RIGHT);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[i]), TRUE);
		gtk_widget_show(lblPlayerInfo[i]);
		gtk_table_attach(GTK_TABLE(tblPlayerInfo), lblPlayerInfo[i],
				 1, 2,
				 player_info[i].row, player_info[i].row + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
	}
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
	gtk_widget_destroy(sgens_window);
	sgens_window = NULL;
}


/**
 * sgens_window_update_rom_type(): Update the current ROM type.
 */
void MDP_FNCALL sgens_window_update_rom_type(void)
{
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
}


/**
 * sgens_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean sgens_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	sgens_window_close();
	return FALSE;
}


/**
 * sgens_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void sgens_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(dialog);
	MDP_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CLOSE:
			// Close.
			sgens_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other response ID. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * sgens_window_update(): Update the information display.
 */
void MDP_FNCALL sgens_window_update(void)
{
	if (!sgens_window)
		return;
	
	if (sgens_current_rom_type <= SGENS_ROM_TYPE_UNSUPPORTED)
		return;
	
	char tmp[64];
	
	// Values common to all supported Sonic games.
	
	// Score.
	sprintf(tmp, "<tt>%d</tt>", (sgens_host_srv->mem_read_32(MDP_MEM_MD_RAM, 0xFE26) * 10));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_SCORE]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_SCORE]), TRUE);
	
	// Time.
	sprintf(tmp, "<tt>%02d:%02d:%02d</tt>",
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE23),
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE24),
		sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE25));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_TIME]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_TIME]), TRUE);
	
	// Rings.
	sprintf(tmp, "<tt>%d</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFE20));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS]), TRUE);
	
	// Lives.
	sprintf(tmp, "<tt>%d</tt>", sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE12));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_LIVES]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_LIVES]), TRUE);
	
	// Continues.
	sprintf(tmp, "<tt>%d</tt>", sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE18));
	gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CONTINUES]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CONTINUES]), TRUE);
	
	// Rings remaining for Perfect Bonus.
	// This is only applicable for Sonic 2.
	if (sgens_current_rom_type >= SGENS_ROM_TYPE_SONIC2_REV00 &&
	    sgens_current_rom_type <= SGENS_ROM_TYPE_SONIC2_REV02)
	{
		sprintf(tmp, "<tt>%d</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFF40));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_RINGS_PERFECT]), TRUE);
	}
	
	// Water status.
	uint16_t water_level = sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xF648);
	sprintf(tmp, "<tt>%s</tt>", (sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xF648) != 0 ? "ON" : "OFF"));
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
		sprintf(tmp, "<tt>%d</tt>", sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFE57));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), TRUE);
		
		// Camera X position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFF10));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), TRUE);
		
		// Camera Y position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xFF14));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), TRUE);
		
		// Player angle.
		uint16_t angle = (sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xD026) |
				  (sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xD027) << 8));
		sprintf(tmp, "<tt>%0.02f°</tt>", ((double)(angle) * 1.40625));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), TRUE);
		
		// Player X position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xD008));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), TRUE);
		
		// Player Y position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xD00C));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), TRUE);
	}
	else
	{
		// Information for games other than S1.
		
		// Number of emeralds.
		sprintf(tmp, "<tt>%d</tt>", sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xFEB1));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_EMERALDS]), TRUE);
		
		// Camera X position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xEE00));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_X]), TRUE);
		
		// Camera Y position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xEE04));
		gtk_label_set_text(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblLevelInfo[LEVEL_INFO_CAMERA_Y]), TRUE);
		
		// Player angle.
		uint16_t angle = (sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xB026) |
				  (sgens_host_srv->mem_read_8(MDP_MEM_MD_RAM, 0xB027) << 8));
		sprintf(tmp, "<tt>%0.02f°</tt>", ((double)(angle) * 1.40625));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_ANGLE]), TRUE);
		
		// Player X position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xB008));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_X]), TRUE);
		
		// Player Y position.
		sprintf(tmp, "<tt>%04X</tt>", sgens_host_srv->mem_read_16(MDP_MEM_MD_RAM, 0xB00C));
		gtk_label_set_text(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblPlayerInfo[PLAYER_INFO_Y]), TRUE);
	}
	
	// sGens window has been updated.
	return;
}
