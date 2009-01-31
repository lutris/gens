/***************************************************************************
 * Gens: [MDP] VDP Layer Options. (Window Code) (GTK+)                     *
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

#include "vlopt_window.h"
#include "vlopt_plugin.h"
#include "vlopt.hpp"

// GTK includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Window.
static GtkWidget *vlopt_window = NULL;
static GtkAccelGroup *vlopt_accel_group;

// Widgets.
static GtkWidget *txtEntry_Code;
static GtkWidget *txtEntry_Name;
static GtkWidget *lstCodes;

// Callbacks.
static gboolean vlopt_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static gboolean vlopt_window_callback_button(GtkButton *button, gpointer user_data);


/**
 * vlopt_window_show(): Show the Game Genie window.
 * @param parent Parent window.
 */
void vlopt_window_show(void *parent)
{
	if (vlopt_window)
	{
		// Game Genie window is already visible.
		// Set focus.
		gtk_widget_grab_focus(vlopt_window);
		return;
	}
	
	// Create the Game Genie window.
	vlopt_window = gtk_dialog_new();
	gtk_widget_set_name(vlopt_window, "vlopt_window");
	gtk_container_set_border_width(GTK_CONTAINER(vlopt_window), 4);
	gtk_window_set_title(GTK_WINDOW(vlopt_window), "Game Genie");
	gtk_window_set_position(GTK_WINDOW(vlopt_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(vlopt_window), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(vlopt_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(vlopt_window), FALSE);
	
	// Create the accelerator group.
	vlopt_accel_group = gtk_accel_group_new();
	g_object_set_data_full(G_OBJECT(vlopt_window), "vlopt_accel_group",
			       g_object_ref(vlopt_accel_group), (GDestroyNotify)g_object_unref);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(vlopt_window), "vlopt_window", vlopt_window);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)vlopt_window, "delete_event",
			 G_CALLBACK(vlopt_window_callback_close), NULL);
	g_signal_connect((gpointer)vlopt_window, "destroy_event",
			 G_CALLBACK(vlopt_window_callback_close), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(vlopt_window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(vlopt_window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the main VBox.
	GtkWidget *vboxMain = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vboxMain, "vboxMain");
	gtk_widget_show(vboxMain);
	gtk_container_add(GTK_CONTAINER(vboxDialog), vboxMain);
	g_object_set_data_full(G_OBJECT(vlopt_window), "vboxMain",
			       g_object_ref(vboxMain), (GDestroyNotify)g_object_unref);
	
	// Create the main frame.
	GtkWidget *fraMain = gtk_frame_new(NULL);
	gtk_widget_set_name(fraMain, "fraMain");
	gtk_widget_show(fraMain);
	gtk_box_pack_start(GTK_BOX(vboxMain), fraMain, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraMain), GTK_SHADOW_NONE);
	g_object_set_data_full(G_OBJECT(vlopt_window), "fraMain",
			       g_object_ref(fraMain), (GDestroyNotify)g_object_unref);
	
	// Strings.
	static const char* const strInfoTitle = "<b><i>Information about Game Genie / Patch codes</i></b>";
	static const char* const strInfo =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA-DDDD (address-data)";
	
	// Header label.
	GtkWidget *lblInfoTitle = gtk_label_new(strInfoTitle);
	gtk_widget_set_name(lblInfoTitle, "lblInfoTitle");
	gtk_label_set_use_markup(GTK_LABEL(lblInfoTitle), TRUE);
	gtk_widget_show(lblInfoTitle);
	gtk_frame_set_label_widget(GTK_FRAME(fraMain), lblInfoTitle);
	g_object_set_data_full(G_OBJECT(vlopt_window), "lblInfoTitle",
			       g_object_ref(lblInfoTitle), (GDestroyNotify)g_object_unref);
	
	// Description label.
	GtkWidget *lblInfo = gtk_label_new(strInfo);
	gtk_widget_set_name(lblInfo, "lblInfo");
	gtk_widget_show(lblInfo);
	gtk_container_add(GTK_CONTAINER(fraMain), lblInfo);
	gtk_misc_set_alignment(GTK_MISC(lblInfo), 0.02f, 0.0f);
	g_object_set_data_full(G_OBJECT(vlopt_window), "lblInfo",
			       g_object_ref(lblInfo), (GDestroyNotify)g_object_unref);
	
	// VBox for table layout.
	GtkWidget *vboxTable = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxTable, "vboxTable");
	gtk_widget_show(vboxTable);
	gtk_box_pack_start(GTK_BOX(vboxMain), vboxTable, FALSE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "vboxMain",
			       g_object_ref(vboxMain), (GDestroyNotify)g_object_unref);
	
	// Table layout.
	GtkWidget *tblEntry = gtk_table_new(2, 3, FALSE);
	gtk_widget_set_name(tblEntry, "tblEntry");
	gtk_widget_show(tblEntry);
	gtk_table_set_col_spacings(GTK_TABLE(tblEntry), 10);
	gtk_box_pack_start(GTK_BOX(vboxTable), tblEntry, FALSE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "tblEntry",
			       g_object_ref(tblEntry), (GDestroyNotify)g_object_unref);
	
	// Code label and entry widgets.
	GtkWidget *lblEntry_Code = gtk_label_new("Code");
	gtk_widget_set_name(lblEntry_Code, "lblEntry_Code");
	gtk_misc_set_alignment(GTK_MISC(lblEntry_Code), 0.0f, 0.5f);
	gtk_widget_show(lblEntry_Code);
	gtk_table_attach(GTK_TABLE(tblEntry), lblEntry_Code, 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "lblEntry_Code",
			       g_object_ref(lblEntry_Code), (GDestroyNotify)g_object_unref);
	
	txtEntry_Code = gtk_entry_new();
	gtk_widget_set_name(txtEntry_Code, "txtEntry_Code");
	gtk_entry_set_max_length(GTK_ENTRY(txtEntry_Code), 11);
	gtk_widget_show(txtEntry_Code);
	gtk_table_attach(GTK_TABLE(tblEntry), txtEntry_Code, 1, 2, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "txtEntry_Code",
			       g_object_ref(txtEntry_Code), (GDestroyNotify)g_object_unref);
	// TODO
	/*
	g_signal_connect((gpointer)txtEntry_Code, "key-press-event",
			  G_CALLBACK(on_entry_vlopt_keypress), NULL);
	*/
	
	// Name label and entry widgets.
	GtkWidget *lblEntry_Name = gtk_label_new("Name");
	gtk_widget_set_name(lblEntry_Name, "lblEntry_Name");
	gtk_misc_set_alignment(GTK_MISC(lblEntry_Name), 0.0f, 0.5f);
	gtk_widget_show(lblEntry_Name);
	gtk_table_attach(GTK_TABLE(tblEntry), lblEntry_Name, 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "lblEntry_Name",
			       g_object_ref(lblEntry_Name), (GDestroyNotify)g_object_unref);
	
	txtEntry_Name = gtk_entry_new();
	gtk_widget_set_name(txtEntry_Name, "txtEntry_Name");
	gtk_entry_set_max_length(GTK_ENTRY(txtEntry_Name), 11);
	gtk_widget_show(txtEntry_Name);
	gtk_table_attach(GTK_TABLE(tblEntry), txtEntry_Name, 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "txtEntry_Name",
			       g_object_ref(txtEntry_Name), (GDestroyNotify)g_object_unref);
	// TODO
	/*
	g_signal_connect((gpointer)txtEntry_Code, "key-press-event",
			  G_CALLBACK(on_entry_vlopt_keypress), NULL);
	*/
	
	// Add Code button.
	GtkWidget *btnAddCode = gtk_button_new_with_mnemonic("_Add Code");
	gtk_widget_set_name(btnAddCode, "btnAddCode");
	gtk_widget_show(btnAddCode);
	gtk_table_attach(GTK_TABLE(tblEntry), btnAddCode, 2, 3, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "btnAddCode",
			       g_object_ref(btnAddCode), (GDestroyNotify)g_object_unref);
	
	// Set the button's icon to "gtk-add".
	GtkWidget *btnAddCode_icon = gtk_image_new_from_stock("gtk-add", GTK_ICON_SIZE_BUTTON);
	gtk_widget_set_name(btnAddCode_icon, "btnAddCode_icon");
	gtk_widget_show(btnAddCode_icon);
	gtk_button_set_image(GTK_BUTTON(btnAddCode), btnAddCode_icon);
	g_object_set_data_full(G_OBJECT(vlopt_window), "btnAddCode_icon",
			       g_object_ref(btnAddCode_icon), (GDestroyNotify)g_object_unref);
	
	// Set the focus chain for the entry boxes.
	GList *lFocusChain = NULL;
	lFocusChain = g_list_append(lFocusChain, txtEntry_Code);
	lFocusChain = g_list_append(lFocusChain, txtEntry_Name);
	lFocusChain = g_list_append(lFocusChain, btnAddCode);
	lFocusChain = g_list_first(lFocusChain);
	gtk_container_set_focus_chain(GTK_CONTAINER(tblEntry), lFocusChain);
	g_list_free(lFocusChain);
	
	// Add the accel group to the window.
	gtk_window_add_accel_group(GTK_WINDOW(vlopt_window), vlopt_accel_group);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(vlopt_window), GTK_WINDOW(parent));
	
	// Show the window.
	gtk_widget_show_all(vlopt_window);
	
	// Register the window with MDP Host Services.
	vlopt_host_srv->window_register(&mdp, vlopt_window);
}


/**
 * vlopt_window_close(): Close the Game Genie window.
 */
void vlopt_window_close(void)
{
	if (!vlopt_window)
		return;
	
	gtk_widget_destroy(vlopt_window);
	
	// Unregister the window from MDP Host Services.
	vlopt_host_srv->window_unregister(&mdp, vlopt_window);
	
	// Set the window pointer to NULL to indicate that it's closed.
	vlopt_window = NULL;
}


/**
 * vlopt_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 */
static gboolean vlopt_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	vlopt_window_close();
	return FALSE;
}
