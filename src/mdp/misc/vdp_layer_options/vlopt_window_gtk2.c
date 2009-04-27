/***************************************************************************
 * Gens: [MDP] VDP Layer Options. (Window Code) (GTK+)                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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
#include <stdio.h>

#include "vlopt_window.h"
#include "vlopt_options.h"
#include "vlopt_plugin.h"
#include "vlopt.h"

// XPM icons.
#include "vlopt_16x16.xpm"
#include "vlopt_32x32.xpm"

// MDP error codes.
#include "mdp/mdp_error.h"

// Window.
static GtkWidget *vlopt_window = NULL;

// Widgets.
static GtkWidget *vlopt_window_checkboxes[VLOPT_OPTIONS_COUNT];

// Custom response IDs.
#define VLOPT_RESPONSE_RESET 1

// Callbacks.
static gboolean vlopt_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void vlopt_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void vlopt_window_callback_checkbox_toggled(GtkToggleButton *togglebutton, gpointer user_data);

// Option handling functions.
static void vlopt_window_load_options(void);
static void vlopt_window_save_options(void);


/**
 * vlopt_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void vlopt_window_show(void *parent)
{
	if (vlopt_window)
	{
		// VDP Layer Options window is already visible.
		// Set focus.
		gtk_widget_grab_focus(vlopt_window);
		return;
	}
	
	// Create the VDP Layer Options window.
	vlopt_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(vlopt_window), 4);
	gtk_window_set_title(GTK_WINDOW(vlopt_window), "VDP Layer Options");
	gtk_window_set_position(GTK_WINDOW(vlopt_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(vlopt_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(vlopt_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(vlopt_window), FALSE);
	
	// Add the window icons.
	GList *icon_list = NULL;
	GdkPixbuf *icon_pixbuf_16, *icon_pixbuf_32;
	
	// Load the 16x16 icon.
	icon_pixbuf_16 = gdk_pixbuf_new_from_xpm_data(vlopt_icon_xpm_16x16);
	icon_list = g_list_append(icon_list, icon_pixbuf_16);
	
	// Load the 32x32 icon.
	icon_pixbuf_32 = gdk_pixbuf_new_from_xpm_data(vlopt_icon_xpm_32x32);
	icon_list = g_list_append(icon_list, icon_pixbuf_32);
	
	// Set the icon list.
	gtk_window_set_icon_list(GTK_WINDOW(vlopt_window), icon_list);
	
	// Unreference everything.
	g_list_free(icon_list);
	g_object_unref(icon_pixbuf_16);
	g_object_unref(icon_pixbuf_32);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)vlopt_window, "delete_event",
			 G_CALLBACK(vlopt_window_callback_close), NULL);
	g_signal_connect((gpointer)vlopt_window, "destroy_event",
			 G_CALLBACK(vlopt_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)vlopt_window, "response",
			 G_CALLBACK(vlopt_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(vlopt_window));
	gtk_widget_show(vboxDialog);
	
	// Create the main VBox.
	GtkWidget *vboxMain = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vboxMain);
	gtk_container_add(GTK_CONTAINER(vboxDialog), vboxMain);
	
	// Create the main frame.
	GtkWidget *fraMain = gtk_frame_new(NULL);
	gtk_widget_show(fraMain);
	gtk_box_pack_start(GTK_BOX(vboxMain), fraMain, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraMain), GTK_SHADOW_ETCHED_IN);
	
	// Main frame label.
	GtkWidget *lblFrameTitle = gtk_label_new("VDP Layer Options");
	gtk_label_set_use_markup(GTK_LABEL(lblFrameTitle), TRUE);
	gtk_widget_show(lblFrameTitle);
	gtk_frame_set_label_widget(GTK_FRAME(fraMain), lblFrameTitle);
	
	// Horizontal padding for the frame VBox.
	GtkWidget *alignVBoxFrame = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignVBoxFrame), 0, 0, 4, 4);
	gtk_container_add(GTK_CONTAINER(fraMain), alignVBoxFrame);
	
	// Create the frame VBox.
	GtkWidget *vboxFrame = gtk_vbox_new(FALSE, 4);
	gtk_widget_show(vboxFrame);
	gtk_container_add(GTK_CONTAINER(alignVBoxFrame), vboxFrame);
	
	// Create the outer table layout for the first 9 layer options.
	GtkWidget *tblOptionsRows = gtk_table_new(4, 2, FALSE);
	gtk_widget_show(tblOptionsRows);
	gtk_box_pack_start(GTK_BOX(vboxFrame), tblOptionsRows, FALSE, FALSE, 0);
	
	// Create a blank label for the first row.
	GtkWidget *lblBlankRow = gtk_label_new(NULL);
	gtk_widget_show(lblBlankRow);
	gtk_table_attach(GTK_TABLE(tblOptionsRows), lblBlankRow,
			 0, 1, 0, 1,
			 (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
	
	// Create the inner table layout for the first 9 layer options.
	GtkWidget *tblOptions = gtk_table_new(4, 3, TRUE);
	gtk_widget_show(tblOptions);
	gtk_table_attach(GTK_TABLE(tblOptionsRows), tblOptions,
			 1, 2, 0, 4,
			 (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
	
	// Create column and row labels.
	for (int i = 0; i < 3; i++)
	{
		// Column label.
		GtkWidget *lblTblColHeader = gtk_label_new(vlopt_options[i].sublayer);
		gtk_misc_set_alignment(GTK_MISC(lblTblColHeader), 0.5f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblTblColHeader), GTK_JUSTIFY_CENTER);
		gtk_widget_show(lblTblColHeader);
		
		gtk_table_attach(GTK_TABLE(tblOptions), lblTblColHeader,
				 i, i + 1, 0, 1,
				 GTK_FILL, (GtkAttachOptions)0, 2, 2);
		
		// Row label.
		GtkWidget *lblTblRowHeader = gtk_label_new(vlopt_options[i * 3].layer);
		gtk_misc_set_alignment(GTK_MISC(lblTblRowHeader), 1.0f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblTblRowHeader), GTK_JUSTIFY_RIGHT);
		gtk_widget_show(lblTblRowHeader);
		
		gtk_table_attach(GTK_TABLE(tblOptionsRows), lblTblRowHeader,
				 0, 1, i + 1, i + 2,
				 GTK_FILL, (GtkAttachOptions)0, 2, 2);
	}
	
	// Create the VDP Layer Options checkboxes.
	uint8_t row = 1, col = 0;
	for (unsigned int i = 0; i < 9; i++)
	{
		vlopt_window_checkboxes[i] = gtk_check_button_new();
		gtk_widget_show(vlopt_window_checkboxes[i]);
		
		gtk_table_attach(GTK_TABLE(tblOptions), vlopt_window_checkboxes[i],
				 col, col + 1, row, row + 1,
				 (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
		
		// Next cell.
		col++;
		if (col >= 3)
		{
			col = 0;
			row++;
		}
		
		// Set the callback.
		g_signal_connect((gpointer)vlopt_window_checkboxes[i], "toggled",
				 G_CALLBACK(vlopt_window_callback_checkbox_toggled),
				 GINT_TO_POINTER(i));
	}
	
	// Create the checkboxes for the remaining VDP Layer Options.
	for (unsigned int i = 9; i < VLOPT_OPTIONS_COUNT; i++)
	{
		vlopt_window_checkboxes[i] = gtk_check_button_new_with_label(vlopt_options[i].layer);
		gtk_widget_show(vlopt_window_checkboxes[i]);
		
		gtk_box_pack_start(GTK_BOX(vboxFrame), vlopt_window_checkboxes[i], FALSE, FALSE, 0);
		
		// Set the callback.
		g_signal_connect((gpointer)vlopt_window_checkboxes[i], "toggled",
				 G_CALLBACK(vlopt_window_callback_checkbox_toggled),
				 GINT_TO_POINTER(i));
	}
	
	// Create the dialog buttons.
	
	// "Reset" button.
	GtkWidget *btnReset = gtk_dialog_add_button(
				GTK_DIALOG(vlopt_window),
				"_Reset", VLOPT_RESPONSE_RESET);
	
	// Create the icon for the "Reset" button.
	GtkWidget *btnReset_icon = gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(btnReset_icon);
	gtk_button_set_image(GTK_BUTTON(btnReset), btnReset_icon);
	
	// "Close" button.
	gtk_dialog_add_button(GTK_DIALOG(vlopt_window),
			      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(vlopt_window), GTK_WINDOW(parent));
	
	// Load the options.
	vlopt_window_load_options();
	
	// Show the window.
	gtk_widget_show_all(vlopt_window);
	
	// Register the window with MDP Host Services.
	vlopt_host_srv->window_register(&mdp, vlopt_window);
}


/**
 * vlopt_window_close(): Close the VDP Layer Options window.
 */
void vlopt_window_close(void)
{
	if (!vlopt_window)
		return;
	
	// Unregister the window from MDP Host Services.
	vlopt_host_srv->window_unregister(&mdp, vlopt_window);
	
	// Destroy the window.
	gtk_widget_destroy(vlopt_window);
	vlopt_window = NULL;
}


/**
 * vlopt_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean vlopt_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	vlopt_window_close();
	return FALSE;
}


/**
 * vlopt_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void vlopt_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(dialog);
	MDP_UNUSED_PARAMETER(user_data);
	
	int rval;
	
	switch (response_id)
	{
		case VLOPT_RESPONSE_RESET:
			// Reset the VDP layer options to the default value.
			rval = vlopt_host_srv->val_set(&mdp, MDP_VAL_VDP_LAYER_OPTIONS, MDP_VDP_LAYER_OPTIONS_DEFAULT);
			if (rval != MDP_ERR_OK)
			{
				fprintf(stderr, "%s(): Error setting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, rval);
			}
			
			// Reload the VDP layer options.
			vlopt_window_load_options();
			break;
		
		case GTK_RESPONSE_CLOSE:
			// Close.
			vlopt_window_close();
			break;
		
		default:
			// Unknown response ID.
			break;
	}
}


/**
 * vlopt_window_load_options(): Load the options from MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_load_options(void)
{
	int vdp_layer_options = vlopt_host_srv->val_get(MDP_VAL_VDP_LAYER_OPTIONS);
	if (vdp_layer_options < 0)
	{
		fprintf(stderr, "%s(): Error getting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, vdp_layer_options);
		return;
	}
	
	// Go through the options.
	for (unsigned int i = 0; i < VLOPT_OPTIONS_COUNT; i++)
	{
		gboolean flag_enabled = (vdp_layer_options & vlopt_options[i].flag);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(vlopt_window_checkboxes[i]), flag_enabled);
	}
}


/**
 * vlopt_window_save_options(): Save the options to MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_save_options(void)
{
	int vdp_layer_options = 0;
	
	// Go through the options.
	for (unsigned int i = 0; i < VLOPT_OPTIONS_COUNT; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(vlopt_window_checkboxes[i])))
			vdp_layer_options |= vlopt_options[i].flag;
	}
	
	// Set the new options.
	int rval = vlopt_host_srv->val_set(&mdp, MDP_VAL_VDP_LAYER_OPTIONS, vdp_layer_options);
	if (rval != MDP_ERR_OK)
	{
		fprintf(stderr, "%s(): Error setting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, vdp_layer_options);
	}
}


/**
 * vlopt_window_callback_checkbox_toggled(): A checkbox was toggled.
 */
static void vlopt_window_callback_checkbox_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(togglebutton);
	MDP_UNUSED_PARAMETER(user_data);
	
	// Save the current layer options.
	vlopt_window_save_options();
}
