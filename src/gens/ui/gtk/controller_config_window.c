/**
 * GENS: (GTK+) Controller Configuration Window.
 */

#include "controller_config_window.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

GtkWidget *controller_config_window = NULL;

GtkAccelGroup *accel_group;
GtkTooltips *tooltips;


void AddControllerVBox(GtkWidget *vbox, int port);


/**
 * create_controller_config_window(): Create the Controller Configuration Window.
 * @return Game Genie Window.
 */
GtkWidget* create_controller_config_window(void)
{
	GdkPixbuf *controller_config_window_icon_pixbuf;
	GtkWidget *table_cc;
	GtkWidget *frame_port_1;
	GtkWidget *frame_port_2;
	
	if (controller_config_window)
	{
		// Controller Configuration window is already created. Set focus.
		gtk_widget_grab_focus(controller_config_window);
		return NULL;
	}
	
	// Create the Controller Configuration window.
	controller_config_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_name(controller_config_window, "controller_config_window");
	gtk_container_set_border_width(GTK_CONTAINER(controller_config_window), 5);
	gtk_window_set_title(GTK_WINDOW(controller_config_window), "Controller Configuration");
	gtk_window_set_position(GTK_WINDOW(controller_config_window), GTK_WIN_POS_CENTER);
	gtk_window_set_type_hint(GTK_WINDOW(controller_config_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	GLADE_HOOKUP_OBJECT_NO_REF(controller_config_window, controller_config_window, "controller_config_window");
	
	// Load the window icon.
	controller_config_window_icon_pixbuf = create_pixbuf("Gens2.ico");
	if (controller_config_window_icon_pixbuf)
	{
		gtk_window_set_icon(GTK_WINDOW(controller_config_window), controller_config_window_icon_pixbuf);
		gdk_pixbuf_unref(controller_config_window_icon_pixbuf);
	}
	
	// Main table.
	table_cc = gtk_table_new(2, 2, FALSE);
	gtk_widget_set_name(table_cc, "table_cc");
	gtk_container_set_border_width(GTK_CONTAINER(table_cc), 5);
	gtk_table_set_row_spacings(GTK_TABLE(table_cc), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table_cc), 5);
	gtk_widget_show(table_cc);
	gtk_container_add(GTK_CONTAINER(controller_config_window), table_cc);
	GLADE_HOOKUP_OBJECT(controller_config_window, table_cc, "table_cc");
	
	// Frame for Port 1.
	frame_port_1 = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_port_1, "frame_port_1");
	gtk_frame_set_shadow_type(GTK_FRAME(frame_port_1), GTK_SHADOW_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(frame_port_1), 5);
	gtk_widget_show(frame_port_1);
	gtk_table_attach(GTK_TABLE(table_cc), frame_port_1, 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, frame_port_1, "frame_port_1");
	
	// Frame for Port 2.
	frame_port_2 = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_port_2, "frame_port_2");
	gtk_frame_set_shadow_type(GTK_FRAME(frame_port_2), GTK_SHADOW_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(frame_port_2), 5);
	gtk_widget_show(frame_port_2);
	gtk_table_attach(GTK_TABLE(table_cc), frame_port_2, 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, frame_port_2, "frame_port_2");
	
	// Add VBoxes for controller input.
	AddControllerVBox(frame_port_1, 1);
	AddControllerVBox(frame_port_2, 2);
	
	gtk_widget_show_all(controller_config_window);
	return controller_config_window;
}


/**
 * AddControllerVBox(): Add a VBox containing controller configuration options.
 * @param frame Frame to add the VBox to.
 * @param port Port number (1 or 2).
 */
void AddControllerVBox(GtkWidget *frame, int port)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label_port;
	GtkWidget *check_teamplayer;
	GtkWidget *table_players;
	GtkWidget *label_player;
	GtkWidget *optionmenu_padtype;
	GtkWidget *optionmenu_padtype_menu;
	GtkWidget *optionmenu_padtype_menuitem;
	GtkWidget *button_configure;
	char tmp[128];
	char player[4];
	int i, j;
	
	// TODO: Move this somewhere else?
	const char* PadTypes[2] =
	{
		"3 buttons",
		"6 buttons",
	};
	
	if (!frame || (port != 1 && port != 2))
		return;
	
	// VBox for this controller
	vbox = gtk_vbox_new(FALSE, 0);
	sprintf(tmp, "vbox_port_%d", port);
	gtk_widget_set_name(vbox, tmp);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	// Create the hbox.
	hbox = gtk_hbox_new(FALSE, 5);
	sprintf(tmp, "hbox_controller_%d", port);
	gtk_widget_set_name(hbox, tmp);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, hbox, tmp);
	
	// Port label
	sprintf(tmp, "<b><i>Port %d</i></b>", port);
	label_port = gtk_label_new(tmp);
	sprintf(tmp, "label_port_%d", port);
	gtk_widget_set_name(label_port, tmp);
	gtk_label_set_use_markup(GTK_LABEL(label_port), TRUE);
	gtk_widget_show(label_port);
	gtk_frame_set_label_widget(GTK_FRAME(frame), label_port);
	
	// Checkbox for enabling teamplayer.
	check_teamplayer = gtk_check_button_new_with_label("Use Teamplayer");
	sprintf(tmp, "check_teamplayer_%d", port);
	gtk_widget_set_name(check_teamplayer, tmp);
	gtk_widget_show(check_teamplayer);
	gtk_box_pack_start(GTK_BOX(hbox), check_teamplayer, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, check_teamplayer, tmp);
	
	// Table for the player controls.
	table_players = gtk_table_new(4, 3, FALSE);
	sprintf(tmp, "table_players_%d", port);
	gtk_widget_set_name(table_players, tmp);
	gtk_table_set_col_spacings(GTK_TABLE(table_players), 10);
	gtk_widget_show(table_players);
	gtk_box_pack_start(GTK_BOX(vbox), table_players, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(controller_config_window, table_players, tmp);
	
	// Player inputs
	for (i = 0; i < 4; i++)
	{
		if (i == 0)
			sprintf(player, "%d", port);
		else
			sprintf(player, "%d%c", port, 'A' + (char)i);
		
		// Player label
		sprintf(tmp, "Player %s", player);
		label_player = gtk_label_new(tmp);
		sprintf(tmp, "label_player_%s", player);
		gtk_widget_set_name(label_player, tmp);
		gtk_misc_set_alignment(GTK_MISC(label_player), 0, 0.5);
		gtk_widget_show(label_player);
		GLADE_HOOKUP_OBJECT(controller_config_window, label_player, tmp);
		gtk_table_attach(GTK_TABLE(table_players), label_player, 0, 1, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Pad type
		optionmenu_padtype = gtk_option_menu_new();
		sprintf(tmp, "optionmenu_padtype_%s", player);
		gtk_widget_set_name(optionmenu_padtype, tmp);
		gtk_widget_show(optionmenu_padtype);
		GLADE_HOOKUP_OBJECT(controller_config_window, optionmenu_padtype, tmp);
		gtk_table_attach(GTK_TABLE(table_players), optionmenu_padtype, 1, 2, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Pad type menu
		optionmenu_padtype_menu = gtk_menu_new();
		sprintf(tmp, "optionmenu_padtype_menu_%s", player);
		gtk_widget_set_name(optionmenu_padtype_menu, tmp);
		gtk_widget_show(optionmenu_padtype_menu);
		GLADE_HOOKUP_OBJECT(controller_config_window, optionmenu_padtype_menu, tmp);
		
		// Pad type submenu
		for (j = 0; j < 2; j++)
		{
			optionmenu_padtype_menuitem = gtk_menu_item_new_with_label(PadTypes[j]);
			sprintf(tmp, "optionmenu_padtype_menuitem_%s_%d", player, j);
			gtk_widget_set_name(optionmenu_padtype_menuitem, tmp);
			gtk_widget_show(optionmenu_padtype_menuitem);
			GLADE_HOOKUP_OBJECT(controller_config_window, optionmenu_padtype_menuitem, tmp);
			gtk_container_add(GTK_CONTAINER(optionmenu_padtype_menu), optionmenu_padtype_menuitem);
		}
		
		// Add the pad type submenu to the pad type dropdown.
		gtk_option_menu_set_menu(GTK_OPTION_MENU(optionmenu_padtype), optionmenu_padtype_menu);
		
		// "Reconfigure" button
		button_configure = gtk_button_new_with_label("Reconfigure");
		sprintf(tmp, "button_configure_%s", player);
		gtk_widget_set_name(button_configure, tmp);
		gtk_widget_show(button_configure);
		GLADE_HOOKUP_OBJECT(controller_config_window, button_configure, tmp);
		gtk_table_attach(GTK_TABLE(table_players), button_configure, 2, 3, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
	}
}
