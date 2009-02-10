/***************************************************************************
 * Gens: (GTK+) Controller Configuration Window.                           *
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

#include "cc_window.h"

// C includes.
#include <stdio.h>
#include <string.h>

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Input Handler.
#include "input/input.h"

// Unused Parameter macro.
#include "macros/unused.h"


// Window.
GtkWidget *cc_window = NULL;
extern int cc_window_is_configuring = FALSE;

// Internal key configuration, which is copied when Save is clicked.
static input_keymap_t cc_key_config[8];

// Miscellaneous functions.
static void	cc_window_create_controller_port_frame(GtkWidget *container, int port);

// Callbacks.
static gboolean	cc_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	cc_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);


/**
 * cc_window_show(): Show the Controller Configuration window.
 * @param parent Parent window.
 */
void cc_window_show(GtkWindow *parent)
{
	if (cc_window)
	{
		// Controller Configuration window is already visible.
		// Set focus.
		gtk_widget_grab_focus(cc_window);
		return;
	}
	
	// Create the Controller Configuration window.
	cc_window = gtk_dialog_new();
	gtk_widget_set_name(cc_window, "cc_window");
	gtk_container_set_border_width(GTK_CONTAINER(cc_window), 4);
	gtk_window_set_title(GTK_WINDOW(cc_window), "Controller Configuration");
	gtk_window_set_position(GTK_WINDOW(cc_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(cc_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(cc_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(cc_window), FALSE);
	g_object_set_data(G_OBJECT(cc_window), "cc_window", cc_window);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)cc_window, "delete_event",
			 G_CALLBACK(cc_window_callback_close), NULL);
	g_signal_connect((gpointer)cc_window, "destroy_event",
			 G_CALLBACK(cc_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)cc_window, "response",
			 G_CALLBACK(cc_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(cc_window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(cc_window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the main HBox.
	GtkWidget *hboxMain = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hboxMain, "hboxMain");
	gtk_widget_show(hboxMain);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxMain, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(cc_window), "hboxMain",
			       g_object_ref(hboxMain), (GDestroyNotify)g_object_unref);
	
	// Controller port VBox.
	GtkWidget *vboxControllerPorts = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxControllerPorts, "vboxControllerPorts");
	gtk_widget_show(vboxControllerPorts);
	gtk_box_pack_start(GTK_BOX(hboxMain), vboxControllerPorts, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(cc_window), "vboxControllerPorts",
			       g_object_ref(vboxControllerPorts), (GDestroyNotify)g_object_unref);
	
	// Create the controller port frames.
	cc_window_create_controller_port_frame(vboxControllerPorts, 1);
	cc_window_create_controller_port_frame(vboxControllerPorts, 2);
	
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(cc_window), GTK_WINDOW(parent));
	
	// Show the window.
	gtk_widget_show_all(cc_window);
}


/**
 * cc_window_create_controller_port_frame(): Create a controller port frame.
 * @param container Container for the frame.
 * @param port Port number.
 */
static void cc_window_create_controller_port_frame(GtkWidget *container, int port)
{
	// TODO: Store the widget pointers.
	
	char tmp[32];
	
	// Create the frame.
	GtkWidget *fraPort = gtk_frame_new(NULL);
	sprintf(tmp, "fraPort_%d", port);
	gtk_widget_set_name(fraPort, tmp);
	gtk_frame_set_shadow_type(GTK_FRAME(fraPort), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraPort), 4);
	gtk_widget_show(fraPort);
	gtk_box_pack_start(GTK_BOX(container), fraPort, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(fraPort), (GDestroyNotify)g_object_unref);
	
	// Port label.
	sprintf(tmp, "<b><i>Port %d</i></b>", port);
	GtkWidget *lblPort = gtk_label_new(tmp);
	sprintf(tmp, "lblPort_%d", port);
	gtk_widget_set_name(lblPort, tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblPort), TRUE);
	gtk_widget_show(lblPort);
	gtk_frame_set_label_widget(GTK_FRAME(fraPort), lblPort);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(fraPort), (GDestroyNotify)g_object_unref);
	
	// VBox for the controller frame.
	GtkWidget *vboxController = gtk_vbox_new(FALSE, 0);
	sprintf(tmp, "vboxController_%d", port);
	gtk_widget_set_name(vboxController, tmp);
	gtk_container_set_border_width(GTK_CONTAINER(vboxController), 8);
	gtk_widget_show(vboxController);
	gtk_container_add(GTK_CONTAINER(fraPort), vboxController);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(vboxController), (GDestroyNotify)g_object_unref);
	
	// Checkbox for enabling teamplayer.
	GtkWidget *chkTeamplayer = gtk_check_button_new_with_label("Use Teamplayer");
	sprintf(tmp, "chkTeamplayer_%d", port);
	gtk_widget_set_name(chkTeamplayer, tmp);
	gtk_widget_show(chkTeamplayer);
	gtk_box_pack_start(GTK_BOX(vboxController), chkTeamplayer, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(chkTeamplayer), (GDestroyNotify)g_object_unref);
	// TODO: Connect the "clicked" signal (or "toggled"?).
	
	// Table for the player controls.
	GtkWidget *tblPlayers = gtk_table_new(4, 3, FALSE);
	sprintf(tmp, "tblPlayers_%d", port);
	gtk_widget_set_name(tblPlayers, tmp);
	gtk_container_set_border_width(GTK_CONTAINER(tblPlayers), 0);
	gtk_table_set_col_spacings(GTK_TABLE(tblPlayers), 12);
	gtk_widget_show(tblPlayers);
	gtk_box_pack_start(GTK_BOX(vboxController), tblPlayers, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(tblPlayers), (GDestroyNotify)g_object_unref);
	
	// Player inputs.
	unsigned int i;
	char player[4];
	for (i = 0; i < 4; i++)
	{
		if (i == 0)
			sprintf(player, "%d", port);
		else
			sprintf(player, "%d%c", port, 'A' + (char)i);
		
		// Player label.
		sprintf(tmp, "Player %s", player);
		GtkWidget *lblPlayer = gtk_label_new(tmp);
		sprintf(tmp, "lblPlayer_%s", player);
		gtk_widget_set_name(lblPlayer, tmp);
		gtk_misc_set_alignment(GTK_MISC(lblPlayer), 0, 0.5);
		gtk_widget_show(lblPlayer);
		gtk_table_attach(GTK_TABLE(tblPlayers), lblPlayer,
				 0, 1, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblPlayer), (GDestroyNotify)g_object_unref);
		
		// Pad type.
		GtkWidget *cboPadType = gtk_combo_box_new_text();
		sprintf(tmp, "cboPadType_%s", player);
		gtk_widget_set_name(cboPadType, tmp);
		gtk_widget_show(cboPadType);
		gtk_table_attach(GTK_TABLE(tblPlayers), cboPadType,
				 1, 2, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(cboPadType), (GDestroyNotify)g_object_unref);
		
		// Pad type dropdown.
		gtk_combo_box_append_text(GTK_COMBO_BOX(cboPadType), "3 buttons");
		gtk_combo_box_append_text(GTK_COMBO_BOX(cboPadType), "6 buttons");
		
		// "Configure" button.
		GtkWidget *btnConfigure = gtk_button_new_with_label("Configure");
		sprintf(tmp, "btnConfigure_%s", player);
		gtk_widget_set_name(btnConfigure, tmp);
		gtk_widget_show(btnConfigure);
		gtk_table_attach(GTK_TABLE(tblPlayers), btnConfigure,
				 2, 3, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(btnConfigure), (GDestroyNotify)g_object_unref);
		
		// Determine the port number to use for the callback.
		unsigned int callbackPort;
		if (i == 0)
			callbackPort = port - 1;
		else
		{
			if (port == 1)
				callbackPort = i + 1;
			else //if (port == 2)
				callbackPort = i + 4;
		}
		// TODO: "clicked" signal for the "Configure" button.
	}
}


/**
 * cc_window_close(): Close the Controller Configuration window.
 */
void cc_window_close(void)
{
	if (!cc_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(cc_window);
	cc_window = NULL;
}


/**
 * cc_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean cc_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	cc_window_close();
	return FALSE;
}


/**
 * cc_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void cc_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(dialog);
	GENS_UNUSED_PARAMETER(user_data);
	
	// TODO
}