/***************************************************************************
 * Gens: (GTK+) Controller Configuration Window - Callback Functions.      *
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
#include "controller_config_window.h"
#include "controller_config_window_callbacks.h"
#include "controller_config_window_misc.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_controller_config_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
	return FALSE;
}


/**
 * Teamplayer check box
 */
void on_check_cc_Teamplayer_clicked(GtkButton *button, gpointer user_data)
{
	// Enable/Disable the appropriate controller options depending on the port number.
	int port; char player; gboolean thisChecked;
	char tmpName[64];
	GtkWidget *label_player;
	GtkWidget *optionmenu_padtype;
	GtkWidget *button_configure;
	
	// Get the port number.
	port = GPOINTER_TO_INT(user_data);
	
	// Get whether this check button is checked or not.
	thisChecked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
	
	// Loop for players xB through xD.
	for (player = 'B'; player <= 'D'; player++)
	{
		// Label
		sprintf(tmpName, "label_player_%d%c", port, player);
		label_player = lookup_widget(controller_config_window, tmpName);
		gtk_widget_set_sensitive(label_player, thisChecked);
		
		// Pad type
		sprintf(tmpName, "optionmenu_padtype_%d%c", port, player);
		optionmenu_padtype = lookup_widget(controller_config_window, tmpName);
		gtk_widget_set_sensitive(optionmenu_padtype, thisChecked);
		
		// Reconfigure button
		sprintf(tmpName, "button_configure_%d%c", port, player);
		button_configure = lookup_widget(controller_config_window, tmpName);
		gtk_widget_set_sensitive(button_configure, thisChecked);
	}
}


/**
 * "Reconfigure"
 */
void on_button_cc_Reconfigure_clicked(GtkButton *button, gpointer user_data)
{
	int player, padtype;
	char playerID[4];
	char objID[64];
	GtkWidget *optionmenu_padtype;
	
	player = GPOINTER_TO_INT(user_data);
	
	// Determine how many buttons are needed.
	// TODO: Make a separate player numeric-to-string function.
	switch (player)
	{
		case 0:
			strcpy(playerID, "1");
			break;
		case 2:
			strcpy(playerID, "1B");
			break;
		case 3:
			strcpy(playerID, "1C");
			break;
		case 4:
			strcpy(playerID, "1D");
			break;
		case 1:
			strcpy(playerID, "2");
			break;
		case 5:
			strcpy(playerID, "2B");
			break;
		case 6:
			strcpy(playerID, "2C");
			break;
		case 7:
			strcpy(playerID, "2D");
			break;
	}
	
	sprintf(objID, "optionmenu_padtype_%s", playerID);
	optionmenu_padtype = lookup_widget(controller_config_window, objID);
	padtype = gtk_option_menu_get_history(GTK_OPTION_MENU(optionmenu_padtype));
	
	Reconfigure_Input(player, padtype);
}


/**
 * Cancel
 */
void on_button_cc_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
}


/**
 * Apply
 */
void on_button_cc_Apply_clicked(GtkButton *button, gpointer user_data)
{
	Controller_Config_Save();
}


/**
 * Save
 */
void on_button_cc_Save_clicked(GtkButton *button, gpointer user_data)
{
	Controller_Config_Save();
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
}
