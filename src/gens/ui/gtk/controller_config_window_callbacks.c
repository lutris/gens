/**
 * GENS: (GTK+) Controller Configuration Window Callbacks.
 */
 
 
#include <string.h>
#include "controller_config_window.h"
#include "controller_config_window_callbacks.h"

#include "gtk-misc.h"


/**
 * STUB: Indicates that this function is a stub.
 */
#define STUB fprintf(stderr, "TODO: STUB: %s()\n", __func__);


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
 * Cancel
 */
void on_button_cc_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
}


/**
 * "Reconfigure"
 */
void on_button_cc_Reconfigure_clicked(GtkButton *button, gpointer user_data)
{
	printf("Reconfigure: %d\n", GPOINTER_TO_INT(user_data));
}
