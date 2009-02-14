/***************************************************************************
 * Gens: (GTK+) Controller Configuration Window.                           *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cc_window.h"

// C includes.
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// BOOL macros.
#include "macros/bool_m.h"

// Gens input variables.
#include "gens_core/io/io.h"
#include "gens_core/io/io_teamplayer.h"

// Input Handler.
#include "input/input.h"

// TODO: Move SDL-specific code to input_sdl.c.
#include <SDL/SDL.h>

// Unused Parameter macro.
#include "macros/unused.h"


// Window.
GtkWidget *cc_window = NULL;
BOOL cc_window_is_configuring = FALSE;

// Internal key configuration, which is copied when Save is clicked.
static input_keymap_t cc_key_config[8];

// Current player number and button being configured.
static int		cc_cur_player;
static int		cc_cur_player_button;

// Widgets.
static GtkWidget	*chkTeamplayer[2];
static GtkWidget	*lblPlayer[8];
static GtkWidget	*cboPadType[8];
static GtkWidget	*optConfigure[8];

// Widgets: "Input Devices" frame.
static GtkWidget	*lstInputDevices;
static GtkListStore	*lstoreInputDevices;

// Widgets: "Configure Controller" frame.
static GtkWidget	*fraConfigure;
static GtkWidget	*lblButton[12];
static GtkWidget	*lblCurConfig[12];
static GtkWidget	*btnChange[12];

// GSList used to link the "Configure" radio buttons.
static GSList		*gslConfigure;

// Widget creation functions.
static void	cc_window_create_controller_port_frame(GtkWidget *container, int port);
static void	cc_window_create_input_devices_frame(GtkWidget *container);
static void	cc_window_populate_input_devices(GtkListStore *list);
static void	cc_window_create_configure_controller_frame(GtkWidget *container);

// Display key name function.
static inline void cc_window_display_key_name(GtkWidget *label, uint16_t key);

// Configuration load/save functions.
static void	cc_window_init(void);
static void	cc_window_save(void);
static void	cc_window_show_configuration(int player);

// Callbacks.
static gboolean	cc_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	cc_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	cc_window_callback_teamplayer_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	cc_window_callback_padtype_changed(GtkComboBox *widget, gpointer user_data);
static void	cc_window_callback_configure_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	cc_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data);
static void	cc_window_callback_btnChangeAll_clicked(GtkButton *button, gpointer user_data);
static void	cc_window_callback_btnClearAll_clicked(GtkButton *button, gpointer user_data);

// Configure a key.
static void	cc_window_configure_key(int player, int button);

// Blink handler. (Blinks the current button configuration label when configuring.)
static gboolean	cc_window_callback_blink(gpointer data);


/**
 * cc_window_show(): Show the Controller Configuration window.
 * @param parent Parent window.
 */
void cc_window_show(GtkWindow *parent)
{
	if (cc_window)
	{
		// Controller Configuration window is already visible. Set focus.
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
	g_signal_connect((gpointer)(cc_window), "delete_event",
			 G_CALLBACK(cc_window_callback_close), NULL);
	g_signal_connect((gpointer)(cc_window), "destroy_event",
			 G_CALLBACK(cc_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(cc_window), "response",
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
	gslConfigure = NULL;
	cc_window_create_controller_port_frame(vboxControllerPorts, 1);
	cc_window_create_controller_port_frame(vboxControllerPorts, 2);
	
	// Create the "Input Devices" frame and populate the "Input Devices" treeview.
	cc_window_create_input_devices_frame(vboxControllerPorts);
	cc_window_populate_input_devices(lstoreInputDevices);
	
	// "Configure Controller" outer VBox.
	GtkWidget *vboxConfigureOuter = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxConfigureOuter, "vboxConfigureOuter");
	gtk_widget_show(vboxConfigureOuter);
	gtk_box_pack_start(GTK_BOX(hboxMain), vboxConfigureOuter, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(cc_window), "vboxConfigureOuter",
			       g_object_ref(vboxConfigureOuter), (GDestroyNotify)g_object_unref);
	
	// Create the "Configure Controller" frame.
	cc_window_create_configure_controller_frame(vboxConfigureOuter);
	
	// Create the dialog buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(cc_window),
			       "gtk-cancel", GTK_RESPONSE_CANCEL,
			       "gtk-apply", GTK_RESPONSE_APPLY,
			       "gtk-save", GTK_RESPONSE_OK,
			       NULL);
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(cc_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_APPLY,
						GTK_RESPONSE_CANCEL,
						-1);
#endif
	
	// Initialize the internal data variables.
	cc_window_init();
	
	// Show the controller configuration for the first player.
	cc_window_show_configuration(0);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(cc_window), parent);
	
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
	char tmp[32];
	
	// Align the frame to the top of the container.
	GtkWidget *alignPort = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	sprintf(tmp, "alignPort_%d", port);
	gtk_widget_set_name(alignPort, tmp);
	gtk_widget_show(alignPort);
	gtk_box_pack_start(GTK_BOX(container), alignPort, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "alignPort",
			       g_object_ref(alignPort), (GDestroyNotify)g_object_unref);
	
	// Create the frame.
	sprintf(tmp, "<b><i>Port %d</i></b>", port);
	GtkWidget *fraPort = gtk_frame_new(tmp);
	sprintf(tmp, "fraPort_%d", port);
	gtk_widget_set_name(fraPort, tmp);
	gtk_frame_set_shadow_type(GTK_FRAME(fraPort), GTK_SHADOW_ETCHED_IN);
	gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPort))), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(fraPort), 4);
	gtk_widget_show(fraPort);
	gtk_container_add(GTK_CONTAINER(alignPort), fraPort);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(fraPort), (GDestroyNotify)g_object_unref);
	
	// VBox for the controller frame.
	GtkWidget *vboxController = gtk_vbox_new(FALSE, 0);
	sprintf(tmp, "vboxController_%d", port);
	gtk_widget_set_name(vboxController, tmp);
	gtk_container_set_border_width(GTK_CONTAINER(vboxController), 0);
	gtk_widget_show(vboxController);
	gtk_container_add(GTK_CONTAINER(fraPort), vboxController);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(vboxController), (GDestroyNotify)g_object_unref);
	
	// Padding for the teamplayer checkbox.
	GtkWidget *alignTeamplayer = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	sprintf(tmp, "alignTeamplayer_%d", port);
	gtk_widget_set_name(alignTeamplayer, tmp);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignTeamplayer), 4, 4, 8, 8);
	gtk_widget_show(alignTeamplayer);
	gtk_box_pack_start(GTK_BOX(vboxController), alignTeamplayer, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "alignTeamplayer",
			       g_object_ref(alignTeamplayer), (GDestroyNotify)g_object_unref);
	
	// Checkbox for enabling teamplayer.
	chkTeamplayer[port-1] = gtk_check_button_new_with_label("Use Teamplayer");
	sprintf(tmp, "chkTeamplayer_%d", port);
	gtk_widget_set_name(chkTeamplayer[port-1], tmp);
	gtk_widget_show(chkTeamplayer[port-1]);
	gtk_container_add(GTK_CONTAINER(alignTeamplayer), chkTeamplayer[port-1]);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(chkTeamplayer[port-1]), (GDestroyNotify)g_object_unref);
	
	// Connect the "toggled" signal for the Teamplayer checkbox.
	g_signal_connect(GTK_OBJECT(chkTeamplayer[port-1]), "toggled",
			 G_CALLBACK(cc_window_callback_teamplayer_toggled),
			 GINT_TO_POINTER(port-1));
	
	// Padding for the player control table.
	GtkWidget *alignPlayers = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	sprintf(tmp, "alignPlayers_%d", port);
	gtk_widget_set_name(alignPlayers, tmp);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignPlayers), 0, 8, 8, 8);
	gtk_widget_show(alignPlayers);
	gtk_box_pack_start(GTK_BOX(vboxController), alignPlayers, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "alignPlayers",
			       g_object_ref(alignPlayers), (GDestroyNotify)g_object_unref);
	
	// Table for the player controls.
	GtkWidget *tblPlayers = gtk_table_new(4, 3, FALSE);
	sprintf(tmp, "tblPlayers_%d", port);
	gtk_widget_set_name(tblPlayers, tmp);
	gtk_container_set_border_width(GTK_CONTAINER(tblPlayers), 0);
	gtk_table_set_col_spacings(GTK_TABLE(tblPlayers), 12);
	gtk_widget_show(tblPlayers);
	gtk_container_add(GTK_CONTAINER(alignPlayers), tblPlayers);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(tblPlayers), (GDestroyNotify)g_object_unref);
	
	// Player inputs.
	unsigned int i, player;
	char playerName[4];
	
	for (i = 0; i < 4; i++)
	{
		if (i == 0)
			sprintf(playerName, "%d", port);
		else
			sprintf(playerName, "%d%c", port, 'A' + (char)i);
		
		// Determine the player number to use for the callback and widget pointer storage.
		if (i == 0)
			player = port - 1;
		else
		{
			if (port == 1)
				player = i + 1;
			else //if (port == 2)
				player = i + 4;
		}
		
		// Player label.
		sprintf(tmp, "Player %s", playerName);
		lblPlayer[player] = gtk_label_new(tmp);
		sprintf(tmp, "lblPlayer_%s", playerName);
		gtk_widget_set_name(lblPlayer[player], tmp);
		gtk_misc_set_alignment(GTK_MISC(lblPlayer[player]), 0, 0.5);
		gtk_widget_show(lblPlayer[player]);
		gtk_table_attach(GTK_TABLE(tblPlayers), lblPlayer[player],
				 0, 1, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblPlayer[player]), (GDestroyNotify)g_object_unref);
		
		// Pad type dropdown.
		cboPadType[player] = gtk_combo_box_new_text();
		sprintf(tmp, "cboPadType_%s", playerName);
		gtk_widget_set_name(cboPadType[player], tmp);
		gtk_widget_show(cboPadType[player]);
		gtk_table_attach(GTK_TABLE(tblPlayers), cboPadType[player],
				 1, 2, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(cboPadType[player]), (GDestroyNotify)g_object_unref);
		
		// Pad type dropdown entries.
		gtk_combo_box_append_text(GTK_COMBO_BOX(cboPadType[player]), "3 buttons");
		gtk_combo_box_append_text(GTK_COMBO_BOX(cboPadType[player]), "6 buttons");
		
		// Connect the "changed" signal for the pad type dropdown.
		g_signal_connect(GTK_OBJECT(cboPadType[player]), "changed",
				 G_CALLBACK(cc_window_callback_padtype_changed),
				 GINT_TO_POINTER(player));
		
		// "Configure" button.
		optConfigure[player] = gtk_radio_button_new_with_label(gslConfigure, "Configure");
		gslConfigure = gtk_radio_button_get_group(GTK_RADIO_BUTTON(optConfigure[player]));
		gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(optConfigure[player]), FALSE);
		sprintf(tmp, "btnConfigure_%s", playerName);
		gtk_widget_set_name(optConfigure[player], tmp);
		gtk_widget_show(optConfigure[player]);
		gtk_table_attach(GTK_TABLE(tblPlayers), optConfigure[player],
				 2, 3, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(optConfigure[player]), (GDestroyNotify)g_object_unref);
		
		// Connect the "toggled" signal for the "Configure" button.
		g_signal_connect(GTK_OBJECT(optConfigure[player]), "toggled",
				 G_CALLBACK(cc_window_callback_configure_toggled),
				 GINT_TO_POINTER(player));
	}
}


/**
 * cc_window_create_input_devices_frame(): Create the "Input Devices" frame.
 * @param container Container for the frame.
 */
static void cc_window_create_input_devices_frame(GtkWidget *container)
{
	// Align the "Input Devices" frame to the top of the container.
	GtkWidget *alignInputDevices = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	gtk_widget_set_name(alignInputDevices, "alignInputDevices");
	gtk_widget_show(alignInputDevices);
	gtk_box_pack_start(GTK_BOX(container), alignInputDevices, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "alignInputDevices",
			       g_object_ref(alignInputDevices), (GDestroyNotify)g_object_unref);
	
	// "Input Devices" frame.
	GtkWidget *fraInputDevices = gtk_frame_new("<b><i>Input Devices</i></b>");
	gtk_widget_set_name(fraInputDevices, "fraInputDevices");
	gtk_frame_set_shadow_type(GTK_FRAME(fraInputDevices), GTK_SHADOW_ETCHED_IN);
	gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraInputDevices))), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(fraInputDevices), 4);
	gtk_widget_show(fraInputDevices);
	gtk_container_add(GTK_CONTAINER(alignInputDevices), fraInputDevices);
	g_object_set_data_full(G_OBJECT(container), "fraInputDevices",
			       g_object_ref(fraInputDevices), (GDestroyNotify)g_object_unref);
	
	// Scrolled Window for the list of input devices.
	GtkWidget *scrlInputDevices = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_name(scrlInputDevices, "scrlInputDevices");
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlInputDevices), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlInputDevices),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width(GTK_CONTAINER(scrlInputDevices), 4);
	gtk_widget_show(scrlInputDevices);
	gtk_container_add(GTK_CONTAINER(fraInputDevices), scrlInputDevices);
	g_object_set_data_full(G_OBJECT(container), "scrlInputDevices",
			       g_object_ref(scrlInputDevices), (GDestroyNotify)g_object_unref);
	
	// Create a treeview for the list of input devices.
	lstInputDevices = gtk_tree_view_new();
	gtk_widget_set_name(lstInputDevices, "lstInputDevices");
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstInputDevices), FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(lstInputDevices), FALSE);
	gtk_widget_set_size_request(lstInputDevices, -1, 96);
	gtk_widget_show(lstInputDevices);
	gtk_container_add(GTK_CONTAINER(scrlInputDevices), lstInputDevices);
	g_object_set_data_full(G_OBJECT(container), "lstInputDevices",
			       g_object_ref(lstInputDevices), (GDestroyNotify)g_object_unref);
	
	// Create the list model.
	lstoreInputDevices = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(lstInputDevices), GTK_TREE_MODEL(lstoreInputDevices));
	g_object_set_data_full(G_OBJECT(container), "lstoreInputDevices",
			       g_object_ref(lstoreInputDevices), (GDestroyNotify)g_object_unref);
	
	// Create the renderer and columns.
	GtkCellRenderer  *rendText = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colText = gtk_tree_view_column_new_with_attributes("Input Device", rendText, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstInputDevices), colText);
}


/**
 * cc_window_populate_input_devices(): Populate the "Input Devices" treeview.
 * @param list List to store the input devices in.
 */
static void cc_window_populate_input_devices(GtkListStore *list)
{
	GtkTreeIter iter;
	
	// Clear the list model.
	gtk_list_store_clear(list);
	
	// Add "Keyboard" as the first entry.
	gtk_list_store_append(list, &iter);
	gtk_list_store_set(GTK_LIST_STORE(list), &iter, 0, "Keyboard", -1);
	
	// Add any detected joysticks to the list model.
	// TODO: This is SDL-specific. Move to input.c/input_sdl.c?
	int joysticks = SDL_NumJoysticks();
	int joy;
	char joy_name[64];
	
	for (joy = 0; joy < joysticks; joy++)
	{
		const char *joy_name_SDL = SDL_JoystickName(joy);
		
		if (joy_name_SDL)
			snprintf(joy_name, sizeof(joy_name), "Joystick %d: %s", joy, joy_name_SDL);
		else
			snprintf(joy_name, sizeof(joy_name), "Joystick %d", joy);
		
		joy_name[sizeof(joy_name) - 1] = 0x00;
		
		// Add the joystick entry to the list model.
		gtk_list_store_append(list, &iter);
		gtk_list_store_set(GTK_LIST_STORE(list), &iter, 0, joy_name, -1);
	}
}


/**
 * cc_window_create_configure_controller_frame(): Create the "Configure Controller" frame.
 * @param container Container for the frame.
 */
static void cc_window_create_configure_controller_frame(GtkWidget *container)
{
	// Align the "Configure Controller" frame to the top of the container.
	GtkWidget *alignConfigure = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	gtk_widget_set_name(alignConfigure, "alignConfigure");
	gtk_widget_show(alignConfigure);
	gtk_box_pack_start(GTK_BOX(container), alignConfigure, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "alignConfigure",
			       g_object_ref(alignConfigure), (GDestroyNotify)g_object_unref);
	
	// "Configure Controller" frame.
	fraConfigure = gtk_frame_new("");
	gtk_widget_set_name(fraConfigure, "fraConfigure");
	gtk_frame_set_shadow_type(GTK_FRAME(fraConfigure), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraConfigure), 4);
	gtk_widget_show(fraConfigure);
	gtk_container_add(GTK_CONTAINER(alignConfigure), fraConfigure);
	g_object_set_data_full(G_OBJECT(container), "fraConfigure",
			       g_object_ref(fraConfigure), (GDestroyNotify)g_object_unref);
	
	// VBox for the "Configure Controller" frame.
	GtkWidget *vboxConfigure = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_name(vboxConfigure, "vboxConfigure");
	gtk_container_set_border_width(GTK_CONTAINER(vboxConfigure), 0);
	gtk_widget_show(vboxConfigure);
	gtk_container_add(GTK_CONTAINER(fraConfigure), vboxConfigure);
	g_object_set_data_full(G_OBJECT(container), "vboxConfigure",
			       g_object_ref(vboxConfigure), (GDestroyNotify)g_object_unref);
		
	// Create the table for button remapping.
	GtkWidget *tblButtonRemap = gtk_table_new(12, 3, FALSE);
	gtk_widget_set_name(tblButtonRemap, "tblButtonRemap");
	gtk_container_set_border_width(GTK_CONTAINER(tblButtonRemap), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tblButtonRemap), 12);
	gtk_widget_show(tblButtonRemap);
	gtk_box_pack_start(GTK_BOX(vboxConfigure), tblButtonRemap, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "tblButtonRemap",
			       g_object_ref(tblButtonRemap), (GDestroyNotify)g_object_unref);
	
	// Populate the table.
	unsigned int button;
	char tmp[16];
	for (button = 0; button < 12; button++)
	{
		// Button label.
		sprintf(tmp, "%s:", input_key_names[button]);
		lblButton[button] = gtk_label_new(tmp);
		sprintf(tmp, "lblButton_%d", button);
		gtk_widget_set_name(lblButton[button], tmp);
		gtk_misc_set_alignment(GTK_MISC(lblButton[button]), 1.0f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblButton[button]), GTK_JUSTIFY_RIGHT);
		gtk_widget_show(lblButton[button]);
		gtk_table_attach(GTK_TABLE(tblButtonRemap), lblButton[button],
				 0, 1, button, button + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblButton[button]), (GDestroyNotify)g_object_unref);
		
		// Current configuration label.
		lblCurConfig[button] = gtk_label_new(NULL);
		sprintf(tmp, "lblCurConfig_%d", button);
		gtk_widget_set_name(lblCurConfig[button], tmp);
		gtk_misc_set_alignment(GTK_MISC(lblCurConfig[button]), 0.0f, 0.5f);
		gtk_label_set_justify(GTK_LABEL(lblCurConfig[button]), GTK_JUSTIFY_CENTER);
		gtk_label_set_width_chars(GTK_LABEL(lblCurConfig[button]), 24);
		gtk_widget_show(lblCurConfig[button]);
		gtk_table_attach(GTK_TABLE(tblButtonRemap), lblCurConfig[button],
				 1, 2, button, button + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(lblCurConfig[button]), (GDestroyNotify)g_object_unref);
		
		// "Change" button.
		btnChange[button] = gtk_button_new_with_label("Change");
		sprintf(tmp, "btnChange_%d", button);
		gtk_widget_set_name(btnChange[button], tmp);
		gtk_widget_show(btnChange[button]);
		gtk_table_attach(GTK_TABLE(tblButtonRemap), btnChange[button],
				 2, 3, button, button + 1,
				 (GtkAttachOptions)(0),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(container), tmp,
				       g_object_ref(btnChange[button]), (GDestroyNotify)g_object_unref);
		
		// Connect the "clicked" signal for the "Change" button.
		g_signal_connect(GTK_OBJECT(btnChange[button]), "clicked",
				 G_CALLBACK(cc_window_callback_btnChange_clicked),
				 GINT_TO_POINTER(button));
	}
	
	// Separator between the table and the miscellaneous buttons.
	GtkWidget *hsepConfigure = gtk_hseparator_new();
	gtk_widget_set_name(hsepConfigure, "hsepConfigure");
	gtk_widget_show(hsepConfigure);
	gtk_box_pack_start(GTK_BOX(vboxConfigure), hsepConfigure, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "hsepConfigure",
			       g_object_ref(hsepConfigure), (GDestroyNotify)g_object_unref);
	
	// HButton Box for miscellaneous controller configuration buttons.
	GtkWidget *hbtnOptions = gtk_hbutton_box_new();
	gtk_widget_set_name(hbtnOptions, "hbtnOptions");
	gtk_container_set_border_width(GTK_CONTAINER(hbtnOptions), 4);
	gtk_widget_show(hbtnOptions);
	gtk_box_pack_start(GTK_BOX(vboxConfigure), hbtnOptions, FALSE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "hbtnOptions",
			       g_object_ref(hbtnOptions), (GDestroyNotify)g_object_unref);
	
	// "Change All Buttons" button.
	GtkWidget *btnChangeAll = gtk_button_new_with_label("Change All Buttons");
	gtk_widget_set_name(btnChangeAll, "btnChangeAll");
	gtk_widget_show(btnChangeAll);
	gtk_box_pack_start(GTK_BOX(hbtnOptions), btnChangeAll, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "btnChangeAll",
			       g_object_ref(btnChangeAll), (GDestroyNotify)g_object_unref);
	
	// Connect the "clicked" signal for the "Change All Buttons" button.
	g_signal_connect(GTK_OBJECT(btnChangeAll), "clicked",
			 G_CALLBACK(cc_window_callback_btnChangeAll_clicked), NULL);
	
	// "Clear All Buttons" button.
	GtkWidget *btnClearAll = gtk_button_new_with_label("Clear All Buttons");
	gtk_widget_set_name(btnClearAll, "btnClearAll");
	gtk_widget_show(btnClearAll);
	gtk_box_pack_start(GTK_BOX(hbtnOptions), btnClearAll, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "btnClearAll",
			       g_object_ref(btnClearAll), (GDestroyNotify)g_object_unref);
	
	// Connect the "clicked" signal for the "Clear All Buttons" button.
	g_signal_connect(GTK_OBJECT(btnClearAll), "clicked",
			 G_CALLBACK(cc_window_callback_btnClearAll_clicked), NULL);
}


/**
 * cc_window_close(): Close the Controller Configuration window.
 */
void cc_window_close(void)
{
	if (!cc_window)
		return;
	
	// Clear the "Configuring" variable.
	cc_window_is_configuring = FALSE;
	
	// Destroy the window.
	gtk_widget_destroy(cc_window);
	cc_window = NULL;
}


/**
 * cc_window_init(): Initialize the internal variables.
 */
static void cc_window_init(void)
{
	// Copy the current controller configuration into the internal input_keymap_t array.
	memcpy(&cc_key_config, &input_keymap, sizeof(cc_key_config));
	
	// Set the Teamplayer checkboxes.
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkTeamplayer[0]), (Controller_1_Type & 0x10));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkTeamplayer[1]), (Controller_2_Type & 0x10));
	
	// Set the pad type dropdowns.
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[0]), (Controller_1_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[1]), (Controller_2_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[2]), (Controller_1B_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[3]), (Controller_1C_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[4]), (Controller_1D_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[5]), (Controller_2B_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[6]), (Controller_2C_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[7]), (Controller_2D_Type & 0x01));
	
	// Run the teamplayer callbacks.
	cc_window_callback_teamplayer_toggled(GTK_TOGGLE_BUTTON(chkTeamplayer[0]), GINT_TO_POINTER(0));
	cc_window_callback_teamplayer_toggled(GTK_TOGGLE_BUTTON(chkTeamplayer[1]), GINT_TO_POINTER(1));
}


/**
 * cc_window_save(): Save the controller configuration.
 */
static void cc_window_save(void)
{
	// Copy the modified controller configuration into the Gens keymap array.
	memcpy(&input_keymap, &cc_key_config, sizeof(input_keymap));
	
	// Clear the Controller Type variables.
	Controller_1_Type = 0;
	Controller_2_Type = 0;
	Controller_1B_Type = 0;
	Controller_1C_Type = 0;
	Controller_1D_Type = 0;
	Controller_2B_Type = 0;
	Controller_2C_Type = 0;
	Controller_2D_Type = 0;
	
	// Save the Teamplayer settings.
	Controller_1_Type  |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkTeamplayer[0])) ? 0x10 : 0x00);
	Controller_2_Type  |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkTeamplayer[1])) ? 0x10 : 0x00);
	
	// Save the pad type settings.
	Controller_1_Type  |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[0])) ? 0x01 : 0x00);
	Controller_2_Type  |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[1])) ? 0x01 : 0x00);
	Controller_1B_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[2])) ? 0x01 : 0x00);
	Controller_1C_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[3])) ? 0x01 : 0x00);
	Controller_1D_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[4])) ? 0x01 : 0x00);
	Controller_2B_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[5])) ? 0x01 : 0x00);
	Controller_2C_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[6])) ? 0x01 : 0x00);
	Controller_2D_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[7])) ? 0x01 : 0x00);
}


/**
 * cc_window_display_key_name(): Display a key name.
 * @param label Label widget.
 * @param key Key value.
 */
static inline void cc_window_display_key_name(GtkWidget *label, uint16_t key)
{
	char tmp[64], key_name[64];
	
	input_get_key_name(key, &key_name[0], sizeof(key_name));
	sprintf(tmp, "<tt>0x%04X: %s</tt>", key, key_name);
	gtk_label_set_text(GTK_LABEL(label), tmp);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
}


/**
 * cc_window_show_configuration(): Show controller configuration.
 * @param player Player number.
 */
static void cc_window_show_configuration(int player)
{
	if (player < 0 || player > 8)
		return;
	
	char tmp[64];
	
	// Set the current player number.
	cc_cur_player = player;
	
	// Set the "Configure Controller" frame title.
	GtkWidget *lblConfigure = gtk_frame_get_label_widget(GTK_FRAME(fraConfigure));
	sprintf(tmp, "<b><i>Configure Player %s</i></b>", &input_player_names[player][1]);
	gtk_label_set_text(GTK_LABEL(lblConfigure), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblConfigure), TRUE);
	
	// Show the key configuration.
	unsigned int button;
	for (button = 0; button < 12; button++)
	{
		cc_window_display_key_name(lblCurConfig[button], cc_key_config[player].data[button]);
	}
	
	// Enable/Disable the Mode/X/Y/Z buttons, depending on whether the pad is set to 3-button or 6-button.
	gboolean is6button = (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[player])) == 1);
	for (button = 8; button < 12; button++)
	{
		gtk_widget_set_sensitive(lblButton[button], is6button);
		gtk_widget_set_sensitive(lblCurConfig[button], is6button);
		gtk_widget_set_sensitive(btnChange[button], is6button);
	}
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
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			cc_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			cc_window_save();
			break;
		case GTK_RESPONSE_OK:
			cc_window_save();
			cc_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * cc_window_callback_teamplayer_toggled(): "Teamplayer" checkbox was toggled.
 * @param togglebutton Button that was toggled.
 * @param user_data Player number.
 */
static void cc_window_callback_teamplayer_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	int port = GPOINTER_TO_INT(user_data);
	if (port < 0 || port > 1)
		return;
	
	gboolean active = gtk_toggle_button_get_active(togglebutton);
	int startPort = (port == 0 ? 2 : 5);
	
	// If new state is "Disabled", check if any of the buttons to be disabled are currently toggled.
	if (!active)
	{
		if ((cc_cur_player >= startPort) && (cc_cur_player < startPort + 3))
		{
			// One of the teamplayer players is selected.
			// Select the main player for the port.
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optConfigure[port]), TRUE);
		}
	}
	
	// Enable/Disable teamplayer ports for this port.
	int i;
	for (i = startPort; i < startPort + 3; i++)
	{
		gtk_widget_set_sensitive(lblPlayer[i], active);
		gtk_widget_set_sensitive(cboPadType[i], active);
		gtk_widget_set_sensitive(optConfigure[i], active);
	}
}


/**
 * cc_window_callback_padtype_changed(): Pad type for a player was changed.
 * @param widget Combo box that was changed.
 * @param user_data Player number.
 */
static void cc_window_callback_padtype_changed(GtkComboBox *widget, gpointer user_data)
{
	int player = GPOINTER_TO_INT(user_data);
	if (player < 0 || player > 8)
		return;
	
	// Check if this player is currently being configured.
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optConfigure[player])))
	{
		// Player is not currently being configured.
		return;
	}
	
	// Player is currently being configured.
	// Enable/Disable the appropriate widgets in the table.
	unsigned int button;
	gboolean is6button = (gtk_combo_box_get_active(widget) == 1);
	for (button = 8; button < 12; button++)
	{
		gtk_widget_set_sensitive(lblButton[button], is6button);
		gtk_widget_set_sensitive(lblCurConfig[button], is6button);
		gtk_widget_set_sensitive(btnChange[button], is6button);
	}
}


/**
 * cc_window_callback_configure_toggled(): "Configure" button for a player was toggled.
 * @param togglebutton Button that was toggled.
 * @param user_data Player number.
 */
static void cc_window_callback_configure_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if (!gtk_toggle_button_get_active(togglebutton))
		return;
	
	// Show the controller configuration.
	cc_window_show_configuration(GPOINTER_TO_INT(user_data));
}


/**
 * cc_window_callback_btnChange_clicked(): A "Change" button was clicked.
 * @param button Button that was clicked.
 * @param user_data Button number.
 */
static void cc_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	
	if (cc_window_is_configuring)
		return;
	
	cc_window_is_configuring = TRUE;
	cc_window_configure_key(cc_cur_player, GPOINTER_TO_INT(user_data));
	cc_window_is_configuring = FALSE;
}


/**
 * cc_window_configure_key(): Configure a key.
 * @param player Player to configure.
 * @param button Button ID.
 */
static void cc_window_configure_key(int player, int button)
{
	if (!cc_window_is_configuring)
		return;
	
	if (button < 0 || button >= 12)
		return;
	
	// If pad type is set to 3 buttons, don't allow button IDs >= 8.
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[player])) == 0)
	{
		if (button >= 8)
			return;
	}
	
	// Set cc_window_is_configuring to indicate that the key is being configured.
	cc_cur_player_button = button;
	
	// Set the current configure text.
	gtk_label_set_text(GTK_LABEL(lblCurConfig[button]), "<tt>Press a Key...</tt>");
	gtk_label_set_use_markup(GTK_LABEL(lblCurConfig[button]), TRUE);
	
	// Set the blink timer for 500 ms.
	g_timeout_add(500, cc_window_callback_blink, GINT_TO_POINTER(button));
	
	// Get a key value.
	cc_key_config[cc_cur_player].data[button] = input_get_key();
	
	if (!cc_window)
	{
		// Window has closed.
		cc_cur_player_button = -1;
		return;
	}
	
	// Set the text of the label with the key name.
	cc_window_display_key_name(lblCurConfig[button], cc_key_config[cc_cur_player].data[button]);
	
	// Key is no longer being configured.
	cc_cur_player_button = -1;
	
	// Make sure the label is visible now.
	gtk_widget_show(lblCurConfig[button]);
}


/**
 * cc_window_callback_blink(): Blink handler.
 * Blinks the current button configuration label when configuring.
 * @param data Data set when setting up the timer.
 * @return FALSE to disable the timer; TRUE to continue the timer.
 */
static gboolean cc_window_callback_blink(gpointer data)
{
	int btnID = GPOINTER_TO_INT(data);
	if (btnID < 0 || btnID > 12)
		return FALSE;
	
	if (!cc_window_is_configuring || cc_cur_player_button != btnID || !cc_window)
	{
		// Not configuring, or configuring a different button.
		// Show the label and disable the timer.
		if (cc_window)
			gtk_widget_show(lblCurConfig[btnID]);
		
		return FALSE;
	}
	
	// Invert the label visibility.
	if (GTK_WIDGET_VISIBLE(lblCurConfig[btnID]))
		gtk_widget_hide(lblCurConfig[btnID]);
	else
		gtk_widget_show(lblCurConfig[btnID]);
	
	// If the window is still configuring, keep the timer going.
	return cc_window_is_configuring;
}


/**
 * cc_window_callback_btnChangeAll_clicked(): "Change All Buttons" button was clicked.
 * @param button Button that was clicked.
 * @param user_data Button number.
 */
static void cc_window_callback_btnChangeAll_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	if (cc_window_is_configuring)
		return;
	
	if (cc_cur_player < 0 || cc_cur_player > 8)
		return;
	
	// Number of buttons to configure.
	int btnCount = (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[cc_cur_player])) == 1 ? 12 : 8);
	
	// Set the "Configuring" flag.
	cc_window_is_configuring = TRUE;
	
	int i;
	for (i = 0; i < btnCount; i++)
	{
		// Sleep for 250 ms between button presses.
		if (i != 0)
		{
			while (gtk_events_pending())
				gtk_main_iteration_do(FALSE);
			usleep(250000);
		}
		
		cc_window_configure_key(cc_cur_player, i);
		
		if (!cc_window)
		{
			// Window has closed.
			break;
		}
	}
	
	// Unset the "Configuring" flag.
	cc_window_is_configuring = FALSE;
}


/**
 * cc_window_callback_btnClearAll_clicked(): "Clear All Buttons" button was clicked.
 * @param button Button that was clicked.
 * @param user_data Button number.
 */
static void cc_window_callback_btnClearAll_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	if (cc_cur_player < 0 || cc_cur_player > 8)
		return;
	
	// Clear all buttons for the current player.
	memset(&cc_key_config[cc_cur_player], 0x00, sizeof(cc_key_config[cc_cur_player]));
	
	// Show the cleared configuration.
	cc_window_show_configuration(cc_cur_player);
}
