/***************************************************************************
 * Gens: (GTK+) Controller Configuration Window - Miscellaneous Functions. *
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

#include <string.h>

#include "controller_config_window.hpp"
#include "controller_config_window_callbacks.hpp"
#include "controller_config_window_misc.hpp"
#include "gens/gens_window.hpp"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "emulator/g_main.hpp"
#include "gens_core/io/io.h"
#include "gens_core/io/io_teamplayer.h"
#include "ui/gens_ui.hpp"


// This is set to 1 when controllers are being configured.
// This prevents GTK+ keypresses from triggering buttons.
int CC_Configuring = 0;


/**
 * Open_Controller_Config(): Opens the Controller Configuration window.
 */
void Open_Controller_Config(void)
{
	GtkWidget *cc;
	GtkWidget *check_teamplayer_1, *check_teamplayer_2;
	GtkWidget *combobox_padtype_P1;
	GtkWidget *combobox_padtype_P1B;
	GtkWidget *combobox_padtype_P1C;
	GtkWidget *combobox_padtype_P1D;
	GtkWidget *combobox_padtype_P2;
	GtkWidget *combobox_padtype_P2B;
	GtkWidget *combobox_padtype_P2C;
	GtkWidget *combobox_padtype_P2D;
	
	cc = create_controller_config_window();
	if (!cc)
	{
		// Either an error occurred while creating the Controller Configuration window,
		// or the Controller Configuration window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(cc), GTK_WINDOW(gens_window));
	
	// Copy the current controller key configuration.
	memcpy(keyConfig, input_keymap, sizeof(keyConfig));
	
	// Set the controller types.
	// (Controller_1_Type & 0x10) == Teamplayer enabled
	// (Controller_1_Type & 0x01) == 6-button
	// TODO: Clean up controller type handling.
	
	// Set the Teamplayer options.
	check_teamplayer_1 = lookup_widget(cc, "check_teamplayer_1");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_teamplayer_1), (Controller_1_Type & 0x10));
	check_teamplayer_2 = lookup_widget(cc, "check_teamplayer_2");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_teamplayer_2), (Controller_2_Type & 0x10));
	
	// Set 3/6 button options.
	combobox_padtype_P1 = lookup_widget(cc, "combobox_padtype_P1");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P1), (Controller_1_Type & 0x01));
	combobox_padtype_P1B = lookup_widget(cc, "combobox_padtype_P1B");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P1B), (Controller_1B_Type & 0x01));
	combobox_padtype_P1C = lookup_widget(cc, "combobox_padtype_P1C");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P1C), (Controller_1C_Type & 0x01));
	combobox_padtype_P1D = lookup_widget(cc, "combobox_padtype_P1D");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P1D), (Controller_1D_Type & 0x01));
	combobox_padtype_P2 = lookup_widget(cc, "combobox_padtype_P2");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P2), (Controller_2_Type & 0x01));
	combobox_padtype_P2B = lookup_widget(cc, "combobox_padtype_P2B");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P2B), (Controller_2B_Type & 0x01));
	combobox_padtype_P2C = lookup_widget(cc, "combobox_padtype_P2C");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P2C), (Controller_2C_Type & 0x01));
	combobox_padtype_P2D = lookup_widget(cc, "combobox_padtype_P2D");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_padtype_P2D), (Controller_2D_Type & 0x01));
	
	// Initialize the Teamplayer state.
	on_check_cc_Teamplayer_clicked(GTK_BUTTON(check_teamplayer_1), GINT_TO_POINTER(1));
	on_check_cc_Teamplayer_clicked(GTK_BUTTON(check_teamplayer_2), GINT_TO_POINTER(2));
	
	// Show the Controller Configuration window.
	gtk_widget_show_all(cc);
}


/**
 * Reconfigure_Input(): Reconfigure an input device.
 * @param player Player number.
 * @param padtype Pad type. (0 == 3-button; 1 == 6-button.)
 * @return 1 on success.
 */
int Reconfigure_Input(int player, int padtype)
{
	GtkWidget *label_cc_echo = lookup_widget(controller_config_window, "label_cc_echo");
	
	// TODO: Somehow condense this code.
	
	CC_Configuring = 1;
	
	const int maxKeys = (padtype & 0x01 ? 12 : 8);
	uint16_t *curKey = &keyConfig[player].data[0];
	char buf[32];
	
	for (int i = 0; i < maxKeys; i++)
	{
		sprintf(buf, "Press a key for: <b>%s</b>\n", input_key_names[i]);
		gtk_label_set_text(GTK_LABEL(label_cc_echo), buf);
		gtk_label_set_use_markup(GTK_LABEL(label_cc_echo), TRUE);
		*curKey++ = input_get_key();
		printf("Key: 0x%04X\n", *(curKey - 1));
		GensUI::sleep(250, true);
	}
	
	// Configuration successful.
	gtk_label_set_text(GTK_LABEL(label_cc_echo),
			"Controller configuration successful.\n"
			"Press any key to continue...");
	input_get_key();
	GensUI::sleep(500, true);
	gtk_label_set_text(GTK_LABEL(label_cc_echo), "\n");
	
	CC_Configuring = 0;
	
	return 1;
}


/**
 * Controller_Config_Save(): Copy the new controller config to the main Gens config.
 */
void Controller_Config_Save(void)
{
	GtkWidget *check_teamplayer_1, *check_teamplayer_2;
	GtkWidget *combobox_padtype_P1;
	GtkWidget *combobox_padtype_P1B;
	GtkWidget *combobox_padtype_P1C;
	GtkWidget *combobox_padtype_P1D;
	GtkWidget *combobox_padtype_P2;
	GtkWidget *combobox_padtype_P2B;
	GtkWidget *combobox_padtype_P2C;
	GtkWidget *combobox_padtype_P2D;
	
	// Copy the new controller key configuration.
	memcpy(input_keymap, keyConfig, sizeof(input_keymap));

	// Set the controller types.
	// (Controller_1_Type & 0x10) == Teamplayer enabled
	// (Controller_1_Type & 0x01) == 6-button
	// TODO: Clean up controller type handling.
	
	// Set the Teamplayer options.
	check_teamplayer_1 = lookup_widget(controller_config_window, "check_teamplayer_1");
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_teamplayer_1)))
		Controller_1_Type |= 0x10;
	else
		Controller_1_Type &= ~0x10;
	
	check_teamplayer_2 = lookup_widget(controller_config_window, "check_teamplayer_2");
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_teamplayer_2)))
		Controller_2_Type |= 0x10;
	else
		Controller_2_Type &= ~0x10;
	
	// Set 3/6 button options.
	combobox_padtype_P1 = lookup_widget(controller_config_window, "combobox_padtype_P1");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P1)) == 1)
		Controller_1_Type |= 0x01;
	else
		Controller_1_Type &= ~0x01;
	
	combobox_padtype_P1B = lookup_widget(controller_config_window, "combobox_padtype_P1B");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P1B)) == 1)
		Controller_1B_Type |= 0x01;
	else
		Controller_1B_Type &= ~0x01;
	
	combobox_padtype_P1C = lookup_widget(controller_config_window, "combobox_padtype_P1C");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P1C)) == 1)
		Controller_1C_Type |= 0x01;
	else
		Controller_1C_Type &= ~0x01;
	
	combobox_padtype_P1D = lookup_widget(controller_config_window, "combobox_padtype_P1D");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P1D)) == 1)
		Controller_1D_Type |= 0x01;
	else
		Controller_1D_Type &= ~0x01;
	
	combobox_padtype_P2 = lookup_widget(controller_config_window, "combobox_padtype_P2");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P2)) == 1)
		Controller_2_Type |= 0x01;
	else
		Controller_2_Type &= ~0x01;
	
	combobox_padtype_P2B = lookup_widget(controller_config_window, "combobox_padtype_P2B");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P2B)) == 1)
		Controller_2B_Type |= 0x01;
	else
		Controller_2B_Type &= ~0x01;
	
	combobox_padtype_P2C = lookup_widget(controller_config_window, "combobox_padtype_P2C");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P2C)) == 1)
		Controller_2C_Type |= 0x01;
	else
		Controller_2C_Type &= ~0x01;
	
	combobox_padtype_P2D = lookup_widget(controller_config_window, "combobox_padtype_P2D");
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(combobox_padtype_P2D)) == 1)
		Controller_2D_Type |= 0x01;
	else
		Controller_2D_Type &= ~0x01;
	
	// Rebuild the I/O table for teamplayer.
	Make_IO_Table();
}
