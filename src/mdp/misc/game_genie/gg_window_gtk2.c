/***************************************************************************
 * Gens: [MDP] Game Genie. (Window Code) (GTK+)                            *
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

#include "gg_window.h"
#include "gg_plugin.h"
#include "gg.hpp"

// GTK includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Window.
static GtkWidget *gg_window = NULL;

// Widgets.
static GtkWidget *txtEntry_Code;
static GtkWidget *txtEntry_Name;
static GtkWidget *lstCodes;

// List model for the code listing.
static GtkListStore *lmCodes = NULL;

// Widget creation functions.
static void gg_window_create_lstCodes(GtkWidget *container);

// Custom response IDs.
#define GG_RESPONSE_DELETE		1
#define GG_RESPONSE_DEACTIVATE_ALL	2

// Callbacks.
static gboolean gg_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
//static void gg_window_callback_button(GtkButton *button, gpointer user_data);
static void gg_window_callback_btnAddCode(GtkButton *button, gpointer user_data);


/**
 * gg_window_show(): Show the Game Genie window.
 * @param parent Parent window.
 */
void gg_window_show(void *parent)
{
	if (gg_window)
	{
		// Game Genie window is already visible.
		// Set focus.
		gtk_widget_grab_focus(gg_window);
		return;
	}
	
	// Create the Game Genie window.
	gg_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(gg_window), 4);
	gtk_window_set_title(GTK_WINDOW(gg_window), "Game Genie");
	gtk_window_set_position(GTK_WINDOW(gg_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(gg_window), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(gg_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(gg_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)gg_window, "delete_event",
			 G_CALLBACK(gg_window_callback_close), NULL);
	g_signal_connect((gpointer)gg_window, "destroy_event",
			 G_CALLBACK(gg_window_callback_close), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(gg_window)->vbox;
	gtk_widget_show(vboxDialog);
	
	// Create the main frame.
	GtkWidget *fraMain = gtk_frame_new(NULL);
	gtk_widget_show(fraMain);
	gtk_box_pack_start(GTK_BOX(vboxDialog), fraMain, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraMain), GTK_SHADOW_NONE);
	
	// Strings.
	static const char* const strInfoTitle = "<b><i>Information about Game Genie / Patch codes</i></b>";
	static const char* const strInfo =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA-DDDD (address-data)";
	
	// Header label.
	GtkWidget *lblInfoTitle = gtk_label_new(strInfoTitle);
	gtk_label_set_use_markup(GTK_LABEL(lblInfoTitle), TRUE);
	gtk_widget_show(lblInfoTitle);
	gtk_frame_set_label_widget(GTK_FRAME(fraMain), lblInfoTitle);
	
	// Description label.
	GtkWidget *lblInfo = gtk_label_new(strInfo);
	gtk_widget_show(lblInfo);
	gtk_container_add(GTK_CONTAINER(fraMain), lblInfo);
	gtk_misc_set_alignment(GTK_MISC(lblInfo), 0.02f, 0.0f);
	
	// VBox for table layout.
	GtkWidget *vboxTable = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vboxTable);
	gtk_box_pack_start(GTK_BOX(vboxDialog), vboxTable, FALSE, TRUE, 0);
	
	// Table layout.
	GtkWidget *tblEntry = gtk_table_new(2, 3, FALSE);
	gtk_widget_show(tblEntry);
	gtk_table_set_col_spacings(GTK_TABLE(tblEntry), 10);
	gtk_box_pack_start(GTK_BOX(vboxTable), tblEntry, FALSE, TRUE, 0);
	
	// Code label and entry widgets.
	GtkWidget *lblEntry_Code = gtk_label_new("Code");
	gtk_misc_set_alignment(GTK_MISC(lblEntry_Code), 0.0f, 0.5f);
	gtk_widget_show(lblEntry_Code);
	gtk_table_attach(GTK_TABLE(tblEntry), lblEntry_Code, 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	txtEntry_Code = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(txtEntry_Code), 11);
	gtk_widget_show(txtEntry_Code);
	gtk_table_attach(GTK_TABLE(tblEntry), txtEntry_Code, 1, 2, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	// TODO
	/*
	g_signal_connect((gpointer)txtEntry_Code, "key-press-event",
			  G_CALLBACK(on_entry_gg_keypress), NULL);
	*/
	
	// Name label and entry widgets.
	GtkWidget *lblEntry_Name = gtk_label_new("Name");
	gtk_misc_set_alignment(GTK_MISC(lblEntry_Name), 0.0f, 0.5f);
	gtk_widget_show(lblEntry_Name);
	gtk_table_attach(GTK_TABLE(tblEntry), lblEntry_Name, 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	txtEntry_Name = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(txtEntry_Name), 11);
	gtk_widget_show(txtEntry_Name);
	gtk_table_attach(GTK_TABLE(tblEntry), txtEntry_Name, 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	// TODO
	/*
	g_signal_connect((gpointer)txtEntry_Code, "key-press-event",
			  G_CALLBACK(on_entry_gg_keypress), NULL);
	*/
	
	// "Add Code" button.
	GtkWidget *btnAddCode = gtk_button_new_with_mnemonic("Add C_ode");
	gtk_widget_show(btnAddCode);
	gtk_table_attach(GTK_TABLE(tblEntry), btnAddCode, 2, 3, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// Set the "Add Code" button's icon to "gtk-add".
	GtkWidget *btnAddCode_icon = gtk_image_new_from_stock("gtk-add", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(btnAddCode_icon);
	gtk_button_set_image(GTK_BUTTON(btnAddCode), btnAddCode_icon);
	
	// Connect the "clicked" signal for the "Add Code" button.
	g_signal_connect((gpointer)btnAddCode, "clicked",
			 G_CALLBACK(gg_window_callback_btnAddCode), NULL);
	
	// Set the focus chain for the entry boxes.
	GList *lFocusChain = NULL;
	lFocusChain = g_list_append(lFocusChain, txtEntry_Code);
	lFocusChain = g_list_append(lFocusChain, txtEntry_Name);
	lFocusChain = g_list_append(lFocusChain, btnAddCode);
	lFocusChain = g_list_first(lFocusChain);
	gtk_container_set_focus_chain(GTK_CONTAINER(tblEntry), lFocusChain);
	g_list_free(lFocusChain);
	
	// HBox for the code list.
	GtkWidget *hboxList = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxList);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxList, TRUE, TRUE, 0);
	
	// Scrolled Window for the code list.
	GtkWidget *scrlList = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlList), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlList),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrlList);
	gtk_box_pack_start(GTK_BOX(hboxList), scrlList, TRUE, TRUE, 0);
	
	// Create the treeview for the Game Genie codes.
	gg_window_create_lstCodes(scrlList);
	
	// Create the dialog buttons.
	
	// "Delete" button.
	gtk_dialog_add_button(GTK_DIALOG(gg_window),
			      GTK_STOCK_DELETE,
			      GG_RESPONSE_DELETE);
	
	// "Deactivate All" button.
	GtkWidget *btnDeactivateAll = gtk_dialog_add_button(
					GTK_DIALOG(gg_window),
					"Deac_tivate All",
					GG_RESPONSE_DEACTIVATE_ALL);
	
	// Create the icon for the "Deactivate All" button.
	GtkWidget *btnDeactivateAll_icon = gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(btnDeactivateAll_icon);
	gtk_button_set_image(GTK_BUTTON(btnDeactivateAll), btnDeactivateAll_icon);
	
	// Add the rest of the buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(gg_window),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
			       GTK_STOCK_SAVE, GTK_RESPONSE_OK,
			       NULL);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(gg_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_APPLY,
						GTK_RESPONSE_CANCEL,
						GG_RESPONSE_DEACTIVATE_ALL,
						GG_RESPONSE_DELETE,
						-1);
#endif
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(gg_window), GTK_WINDOW(parent));
	
	// Show the window.
	gtk_widget_show_all(gg_window);
	
	// Register the window with MDP Host Services.
	gg_host_srv->window_register(&mdp, gg_window);
}


/**
 * gg_window_create_lstCodes(): Create the treeview for the Game Genie codes.
 * @param container Container for the treeview.
 */
static void gg_window_create_lstCodes(GtkWidget *container)
{
	// Create the list model.
	if (lmCodes)
	{
		// List model already exists. Clear it.
		gtk_list_store_clear(GTK_LIST_STORE(lmCodes));
	}
	else
	{
		lmCodes = gtk_list_store_new(3,
				G_TYPE_BOOLEAN,		// Enabled
				G_TYPE_STRING,		// Code
				G_TYPE_STRING);		// Name
	}
	
	// Treeview containing the Game Genie codes.
	lstCodes = gtk_tree_view_new_with_model(GTK_TREE_MODEL(lmCodes));
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstCodes), TRUE);
	gtk_widget_set_size_request(lstCodes, -1, 160);
	gtk_widget_show(lstCodes);
	gtk_container_add(GTK_CONTAINER(container), lstCodes);
	
	// Create the renderers and columns for the treeview.
	GtkCellRenderer  *rendEnabled = gtk_cell_renderer_toggle_new();
	GtkTreeViewColumn *colEnabled = gtk_tree_view_column_new_with_attributes("Enabled", rendEnabled, "active", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colEnabled);
	
	GtkCellRenderer  *rendCode = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colCode = gtk_tree_view_column_new_with_attributes("Code", rendCode, "active", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colCode);
	
	GtkCellRenderer  *rendName = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colName = gtk_tree_view_column_new_with_attributes("Name", rendName, "active", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colName);
}


/**
 * gg_window_close(): Close the Game Genie window.
 */
void gg_window_close(void)
{
	if (!gg_window)
		return;
	
	// Unregister the window from MDP Host Services.
	gg_host_srv->window_unregister(&mdp, gg_window);
	
	// Destroy the window.
	gtk_widget_destroy(gg_window);
	gg_window = NULL;
}


/**
 * gg_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 */
static gboolean gg_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	gg_window_close();
	return FALSE;
}


/**
 * gg_window_callback_btnAddCode(): "Add Code" button was clicked.
 * @param button Button widget.
 * @param user_data User data.
 */
static void gg_window_callback_btnAddCode(GtkButton *button, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(button);
	MDP_UNUSED_PARAMETER(user_data);
	
	// TODO: Add the code.
	fprintf(stderr, "Code entered: %s\n", gtk_entry_get_text(GTK_ENTRY(txtEntry_Code)));
}
