/**
 * GENS: (GTK+) Controller Configuration Window - Miscellaneous Functions.
 */
 
 
#include <string.h>

#include "controller_config_window.h"
#include "controller_config_window_callbacks.h"
#include "controller_config_window_misc.h"
#include "gens_window.h"

#include "g_main.h"

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
	
	// Initialize the Teamplayer state.
	on_check_cc_Teamplayer_clicked(GTK_BUTTON(check_teamplayer_1), GINT_TO_POINTER(1));
	on_check_cc_Teamplayer_clicked(GTK_BUTTON(check_teamplayer_2), GINT_TO_POINTER(2));
}


/**
 * Reconfigure_Input(): Reconfigure an input device.
 * @param player Player number.
 * @param padtype Pad type. (0 == 3-button; 1 == 6-button.)
 * @return 1 on success.
 */
int Reconfigure_Input(int player, int padtype)
{
	GtkWidget *label_echo = lookup_widget(controller_config_window, "label_echo");
	
	// TODO: Somehow condense this code.
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR UP");
	Keys_Config[player].Up = Get_Key();
	Sleep(250);
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR DOWN");
	Keys_Config[player].Down = Get_Key();
	Sleep(250);
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR LEFT");
	Keys_Config[player].Left = Get_Key();
	Sleep(250);
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR RIGHT");
	Keys_Config[player].Right = Get_Key();
	Sleep(250);
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR START");
	Keys_Config[player].Start = Get_Key();
	Sleep(250);
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR A");
	Keys_Config[player].A = Get_Key();
	Sleep(250);
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR B");
	Keys_Config[player].B = Get_Key();
	Sleep(250);
	
	gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR C");
	Keys_Config[player].C = Get_Key();
	Sleep(250);
	
	if (padtype & 0x01)
	{
		// 6-button control pad. Get additional keys.
		gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR MODE");
		Keys_Config[player].Mode = Get_Key();
		Sleep(250);
		
		gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR X");
		Keys_Config[player].X = Get_Key();
		Sleep(250);
		
		gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR Y");
		Keys_Config[player].Y = Get_Key();
		Sleep(250);
		
		gtk_label_set_text(GTK_LABEL(label_echo), "INPUT KEY FOR Z");
		Keys_Config[player].Z = Get_Key();
		Sleep(250);
	}
	
	// Configuration successful.
	gtk_label_set_text(GTK_LABEL(label_echo),
			"CONFIGURATION SUCCESSFUL.\n"
			"PRESS ANY KEY TO CONTINUE...");
	Get_Key();
	Sleep(500);
	gtk_label_set_text(GTK_LABEL(label_echo), "");
	
	return 1;
}
