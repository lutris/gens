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
int cc_window_is_configuring = FALSE;

// Internal key configuration, which is copied when Save is clicked.
static input_keymap_t cc_key_config[8];

// Widgets.
static GtkWidget	*chkTeamplayer[2];
static GtkWidget	*lblPlayer[8];
static GtkWidget	*cboPadType[8];
static GtkWidget	*optConfigure[8];
static GtkWidget	*cboInputDevice;

// Widgets: "Configure Controller" frame.
static GtkWidget	*lblShowConfig;
static GtkWidget	*lblButton[12];
static GtkWidget	*lblCurConfig[12];
static GtkWidget	*btnChange[12];

// GSList used to link the "Configure" radio buttons.
static GSList		*gslConfigure;

// Widget creation functions.
static void	cc_window_create_controller_port_frame(GtkWidget *container, int port);
static void	cc_window_create_configure_controller_frame(GtkWidget *container);

// Callbacks.
static gboolean	cc_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	cc_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	cc_window_callback_teamplayer_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	cc_window_callback_configure_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	cc_window_callback_padtype_changed(GtkComboBox *widget, gpointer user_data);

// Configuration load/save functions.
static void	cc_window_init(void);
static void	cc_window_load_configuration(int player);


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
	gslConfigure = NULL;
	cc_window_create_controller_port_frame(vboxControllerPorts, 1);
	cc_window_create_controller_port_frame(vboxControllerPorts, 2);
	
	// "Configure Controller" outer VBox.
	GtkWidget *vboxConfigureOuter = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxConfigureOuter, "vboxConfigureOuter");
	gtk_widget_show(vboxConfigureOuter);
	gtk_box_pack_start(GTK_BOX(hboxMain), vboxConfigureOuter, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(cc_window), "vboxConfigureOuter",
			       g_object_ref(vboxConfigureOuter), (GDestroyNotify)g_object_unref);
	
	// Create the "Configure Controller" frame.
	cc_window_create_configure_controller_frame(vboxConfigureOuter);
	
	// Initialize the internal data variables.
	cc_window_init();
	
	// Load the controller configuration for the first player.
	cc_window_load_configuration(0);
	
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
	
	// Align the frame to the top of the window.
	GtkWidget *alignPort = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	sprintf(tmp, "alignPort_%d", port);
	gtk_widget_set_name(alignPort, tmp);
	gtk_widget_show(alignPort);
	gtk_box_pack_start(GTK_BOX(container), alignPort, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "alignPort",
			       g_object_ref(alignPort), (GDestroyNotify)g_object_unref);
	
	// Create the frame.
	GtkWidget *fraPort = gtk_frame_new(NULL);
	sprintf(tmp, "fraPort_%d", port);
	gtk_widget_set_name(fraPort, tmp);
	gtk_frame_set_shadow_type(GTK_FRAME(fraPort), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraPort), 4);
	gtk_widget_show(fraPort);
	gtk_container_add(GTK_CONTAINER(alignPort), fraPort);
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
	chkTeamplayer[port-1] = gtk_check_button_new_with_label("Use Teamplayer");
	sprintf(tmp, "chkTeamplayer_%d", port);
	gtk_widget_set_name(chkTeamplayer[port-1], tmp);
	gtk_widget_show(chkTeamplayer[port-1]);
	gtk_box_pack_start(GTK_BOX(vboxController), chkTeamplayer[port-1], FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), tmp,
			       g_object_ref(chkTeamplayer[port-1]), (GDestroyNotify)g_object_unref);
	
	// Connect the "toggled" signal for the Teamplayer checkbox.
	g_signal_connect(GTK_OBJECT(chkTeamplayer[port-1]), "toggled",
			 G_CALLBACK(cc_window_callback_teamplayer_toggled),
			 GINT_TO_POINTER(port-1));
	
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
		
		// Pad type.
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
		
		// Pad type dropdown.
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
 * cc_window_create_configure_controller_frame(): Create the "Configure Controller" frame.
 * @param container Container for the frame.
 */
static void cc_window_create_configure_controller_frame(GtkWidget *container)
{
	// Align the "Configure Controller" frame to the top of the window.
	GtkWidget *alignConfigure = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	gtk_widget_set_name(alignConfigure, "alignConfigure");
	gtk_widget_show(alignConfigure);
	gtk_box_pack_start(GTK_BOX(container), alignConfigure, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "alignConfigure",
			       g_object_ref(alignConfigure), (GDestroyNotify)g_object_unref);
	
	// "Configure Controller" frame.
	GtkWidget *fraConfigure = gtk_frame_new(NULL);
	gtk_widget_set_name(fraConfigure, "fraConfigure");
	gtk_frame_set_shadow_type(GTK_FRAME(fraConfigure), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraConfigure), 4);
	gtk_widget_show(fraConfigure);
	gtk_container_add(GTK_CONTAINER(alignConfigure), fraConfigure);
	g_object_set_data_full(G_OBJECT(container), "fraConfigure",
			       g_object_ref(fraConfigure), (GDestroyNotify)g_object_unref);
	
	// Frame label.
	GtkWidget *lblConfigure = gtk_label_new("<b><i>Configure Controller</i></b>");
	gtk_widget_set_name(lblConfigure, "lblConfigure");
	gtk_label_set_use_markup(GTK_LABEL(lblConfigure), TRUE);
	gtk_widget_show(lblConfigure);
	gtk_frame_set_label_widget(GTK_FRAME(fraConfigure), lblConfigure);
	g_object_set_data_full(G_OBJECT(container), "lblConfigure",
			       g_object_ref(lblConfigure), (GDestroyNotify)g_object_unref);
	
	// VBox for "Configure Controller".
	GtkWidget *vboxConfigure = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxConfigure, "vboxConfigure");
	gtk_container_set_border_width(GTK_CONTAINER(vboxConfigure), 8);
	gtk_widget_show(vboxConfigure);
	gtk_container_add(GTK_CONTAINER(fraConfigure), vboxConfigure);
	g_object_set_data_full(G_OBJECT(container), "vboxConfigure",
			       g_object_ref(vboxConfigure), (GDestroyNotify)g_object_unref);
	
	// HBox for input device selection.
	GtkWidget *hboxInputDevice = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hboxInputDevice, "hboxInputDevice");
	gtk_container_set_border_width(GTK_CONTAINER(hboxInputDevice), 8);
	gtk_widget_show(hboxInputDevice);
	gtk_box_pack_start(GTK_BOX(vboxConfigure), hboxInputDevice, TRUE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "hboxInputDevice",
			       g_object_ref(hboxInputDevice), (GDestroyNotify)g_object_unref);
	
	// Label for input device selection.
	GtkWidget *lblInputDevice = gtk_label_new("Input Device: ");
	gtk_widget_set_name(lblInputDevice, "lblInputDevice");
	gtk_widget_show(lblInputDevice);
	gtk_box_pack_start(GTK_BOX(hboxInputDevice), lblInputDevice, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "lblInputDevice",
			       g_object_ref(lblInputDevice), (GDestroyNotify)g_object_unref);
	
	// Dropdown box for input device selection.
	cboInputDevice = gtk_combo_box_new_text();
	gtk_widget_set_name(cboInputDevice, "cboInputDevice");
	gtk_widget_show(cboInputDevice);
	gtk_box_pack_start(GTK_BOX(hboxInputDevice), cboInputDevice, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "cboInputDevice",
			       g_object_ref(cboInputDevice), (GDestroyNotify)g_object_unref);
	
	// Create the frame for showing the configuration.
	GtkWidget *fraShowConfig = gtk_frame_new(NULL);
	gtk_widget_set_name(fraShowConfig, "fraShowConfig");
	gtk_frame_set_shadow_type(GTK_FRAME(fraShowConfig), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraShowConfig), 0);
	gtk_widget_show(fraShowConfig);
	gtk_box_pack_start(GTK_BOX(vboxConfigure), fraShowConfig, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(container), "fraShowConfig",
			       g_object_ref(fraShowConfig), (GDestroyNotify)g_object_unref);
	
	// Create the label for showing the configuration.
	lblShowConfig = gtk_label_new(NULL);
	gtk_widget_set_name(lblShowConfig, "lblShowConfig");
	gtk_widget_show(lblShowConfig);
	gtk_frame_set_label_widget(GTK_FRAME(fraShowConfig), lblShowConfig);
	g_object_set_data_full(G_OBJECT(container), "lblShowConfig",
			       g_object_ref(lblShowConfig), (GDestroyNotify)g_object_unref);
	
	// Create the table for button remapping.
	GtkWidget *tblButtonRemap = gtk_table_new(12, 3, FALSE);
	gtk_widget_set_name(tblButtonRemap, "tblButtonRemap");
	gtk_container_set_border_width(GTK_CONTAINER(tblButtonRemap), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tblButtonRemap), 12);
	gtk_widget_show(tblButtonRemap);
	gtk_container_add(GTK_CONTAINER(fraShowConfig), tblButtonRemap);
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
		gtk_label_set_selectable(GTK_LABEL(lblCurConfig[button]), TRUE);
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
	
	// Populate the "Input Device" dropdown.
	gtk_combo_box_append_text(GTK_COMBO_BOX(cboInputDevice), "Keyboard");
	
	// TODO: This is SDL-specific. Move to input.c/input_sdl.c?
	int joysticks = SDL_NumJoysticks();
	int joy;
	char tmp[64];
	
	for (joy = 0; joy < joysticks; joy++)
	{
		const char *joy_name = SDL_JoystickName(joy);
		
		if (joy_name)
			snprintf(tmp, sizeof(tmp), "Joystick %d: %s", joy, joy_name);
		else
			snprintf(tmp, sizeof(tmp), "Joystick %d", joy);
		
		tmp[sizeof(tmp) - 1] = 0x00;
		
		// Add the joystick entry to the dropdown.
		gtk_combo_box_append_text(GTK_COMBO_BOX(cboInputDevice), tmp);
	}
	
	// Select the first item in the "Input Device" dropdown.
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboInputDevice), 0);
}


/**
 * cc_window_load_configuration(): Load controller configuration.
 * @param player Player number.
 */
static void cc_window_load_configuration(int player)
{
	if (player < 0 || player > 8)
		return;
	
	char tmp[64];
	char key_name[64];
	
	// Set the "Configure Controller" frame title.
	sprintf(tmp, "<b><i>Configure Player %s</i></b>", &input_player_names[player][1]);
	gtk_label_set_text(GTK_LABEL(lblShowConfig), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblShowConfig), TRUE);
	
	// Load the key configuration.
	// TODO: Convert thm to human-readable text.
	unsigned int button;
	for (button = 0; button < 12; button++)
	{
		input_get_key_name(cc_key_config[player].data[button], &key_name[0], sizeof(key_name));
		sprintf(tmp, "<tt>0x%04X: %s</tt>", cc_key_config[player].data[button], key_name);
		gtk_label_set_text(GTK_LABEL(lblCurConfig[button]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblCurConfig[button]), TRUE);
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
	
	// TODO
}


/**
 * cc_window_callback_teamplayer_toggled(): "Teamplayer" checkbox was toggled.
 * @param togglebutton Button that was toggled.
 * @param user_data Player number.
 */
static void cc_window_callback_teamplayer_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	gboolean active = gtk_toggle_button_get_active(togglebutton);
	
	int port = GPOINTER_TO_INT(user_data);
	if (port < 0 || port > 1)
		return;
	
	unsigned int startPort = (port == 0 ? 2: 5);
	unsigned int i;
	
	// If new state is "Disabled", check if any of the buttons to be disabled are currently toggled.
	if (!active)
	{
		for (i = startPort; i < startPort + 3; i++)
		{
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optConfigure[i])))
			{
				// One of the buttons is toggled.
				// Toggle the main button for the port.
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optConfigure[port]), TRUE);
				break;
			}
		}
	}
	
	// Enable/Disable teamplayer ports for this port.
	for (i = startPort; i < startPort + 3; i++)
	{
		gtk_widget_set_sensitive(lblPlayer[i], active);
		gtk_widget_set_sensitive(cboPadType[i], active);
		gtk_widget_set_sensitive(optConfigure[i], active);
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
	
	// Load the controller configuration.
	cc_window_load_configuration(GPOINTER_TO_INT(user_data));
}


/**
 * cc_window_callback_padtype_changed(): Pad Type for a player was changed.
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
