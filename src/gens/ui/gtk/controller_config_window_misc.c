/**
 * GENS: (GTK+) Controller Configuration Window - Miscellaneous Functions.
 */
 
 
#include <string.h>

#include "controller_config_window.h"
#include "controller_config_window_callbacks.h"
#include "controller_config_window_misc.h"
#include "gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "io.h"


/**
 * Open_Controller_Config(): Opens the Controller Configuration window.
 */
void Open_Controller_Config(void)
{
	GtkWidget *cc;
	GtkWidget *check_teamplayer_1, *check_teamplayer_2;
	GtkWidget *optionmenu_padtype_1;
	GtkWidget *optionmenu_padtype_1B;
	GtkWidget *optionmenu_padtype_1C;
	GtkWidget *optionmenu_padtype_1D;
	GtkWidget *optionmenu_padtype_2;
	GtkWidget *optionmenu_padtype_2B;
	GtkWidget *optionmenu_padtype_2C;
	GtkWidget *optionmenu_padtype_2D;
	
	cc = create_controller_config_window();
	if (!cc)
	{
		// Either an error occurred while creating the Controller Configuration window,
		// or the Controller Configuration window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(cc), GTK_WINDOW(gens_window));
	
	// Copy the current controller configuration.
	memcpy(Keys_Config, Keys_Def, sizeof(Keys_Def));
	
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
	optionmenu_padtype_1 = lookup_widget(cc, "optionmenu_padtype_1");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_1), (Controller_1_Type & 0x01));
	optionmenu_padtype_1B = lookup_widget(cc, "optionmenu_padtype_1B");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_1B), (Controller_1B_Type & 0x01));
	optionmenu_padtype_1C = lookup_widget(cc, "optionmenu_padtype_1C");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_1C), (Controller_1C_Type & 0x01));
	optionmenu_padtype_1D = lookup_widget(cc, "optionmenu_padtype_1D");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_1D), (Controller_1D_Type & 0x01));
	optionmenu_padtype_2 = lookup_widget(cc, "optionmenu_padtype_2");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_2), (Controller_2_Type & 0x01));
	optionmenu_padtype_2B = lookup_widget(cc, "optionmenu_padtype_2B");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_2B), (Controller_2B_Type & 0x01));
	optionmenu_padtype_2C = lookup_widget(cc, "optionmenu_padtype_2C");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_2C), (Controller_2C_Type & 0x01));
	optionmenu_padtype_2D = lookup_widget(cc, "optionmenu_padtype_2D");
	gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu_padtype_2D), (Controller_2D_Type & 0x01));
}
