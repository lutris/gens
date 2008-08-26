/**
 * GENS: (GTK+) About Window.
 */

#include "about_window.h"
#include "gens_window.h"

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

#include "gens.h"

GtkWidget *about_window = NULL;

GtkAccelGroup *accel_group;


/**
 * create_about_window(): Create the About Window.
 * @return About Window.
 */
GtkWidget* create_about_window(void)
{
	GdkPixbuf *about_window_icon_pixbuf;
	GtkWidget *vbox_about_dialog, *vbox_about_main;
	GtkWidget *hbox_about_logo;
	GtkWidget *image_gens_logo, *label_gens_version;
	GtkWidget *frame_copyright, *label_copyright;
	GtkWidget *about_dialog_action_area, *button_about_OK;
	
	if (about_window)
	{
		// About window is already created. Set focus.
		gtk_widget_grab_focus(about_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the About window.
	about_window = gtk_dialog_new();
	gtk_widget_set_name(about_window, "about_window");
	gtk_container_set_border_width(GTK_CONTAINER(about_window), 5);
	gtk_window_set_title(GTK_WINDOW(about_window), "About Gens");
	gtk_window_set_position(GTK_WINDOW(about_window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(about_window), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(about_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(about_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(about_window), FALSE);
	GLADE_HOOKUP_OBJECT_NO_REF(about_window, about_window, "about_window");
	
	// Load the Gens icon.
	about_window_icon_pixbuf = create_pixbuf("Gens2.ico");
	if (about_window_icon_pixbuf)
	{
		gtk_window_set_icon(GTK_WINDOW(about_window), about_window_icon_pixbuf);
		gdk_pixbuf_unref(about_window_icon_pixbuf);
	}
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)about_window, "delete_event",
			 G_CALLBACK(on_about_window_close), NULL);
	g_signal_connect((gpointer)about_window, "destroy_event",
			 G_CALLBACK(on_about_window_close), NULL);
	
	// Get the dialog VBox.
	vbox_about_dialog = GTK_DIALOG(about_window)->vbox;
	gtk_widget_set_name(vbox_about_dialog, "vbox_about_dialog");
	gtk_widget_show(vbox_about_dialog);
	GLADE_HOOKUP_OBJECT_NO_REF(about_window, vbox_about_dialog, "vbox_about_dialog");
	
	// Create the main VBox.
	vbox_about_main = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_about_main, "vbox_about_main");
	gtk_widget_show(vbox_about_main);
	gtk_container_add(GTK_CONTAINER(vbox_about_dialog), vbox_about_main);
	GLADE_HOOKUP_OBJECT(about_window, vbox_about_main, "vbox_about_main");
	
	// Create the HBox for the logo and the program information.
	hbox_about_logo = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hbox_about_logo, "hbox_about_logo");
	gtk_widget_show(hbox_about_logo);
	gtk_box_pack_start(GTK_BOX(vbox_about_main), hbox_about_logo, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(about_window, hbox_about_logo, "hbox_about_logo");
	
	// Gens logo
	image_gens_logo = create_pixmap(about_window, "gens_small.png");
	gtk_widget_set_name(image_gens_logo, "image_gens_logo");
	gtk_widget_show(image_gens_logo);
	gtk_box_pack_start(GTK_BOX(hbox_about_logo), image_gens_logo, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(about_window, image_gens_logo, "image_gens_logo");
	
	// Version information
	label_gens_version = gtk_label_new(
		"<b><i>Gens for Linux\n"
		"Version [TODO]</i></b>\n\n"
		"Sega Genesis / Mega Drive,\n"
		"Sega CD / Mega CD,\n"
		"Sega 32X emulator"
		);
	gtk_widget_set_name(label_gens_version, "label_gens_version");
	gtk_label_set_use_markup(GTK_LABEL(label_gens_version), TRUE);
	gtk_label_set_justify(GTK_LABEL(label_gens_version), GTK_JUSTIFY_CENTER);
	gtk_widget_show(label_gens_version);
	gtk_box_pack_start(GTK_BOX(hbox_about_logo), label_gens_version, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(about_window, label_gens_version, "label_gens_version");
	
	// Copyright frame
	frame_copyright = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_copyright, "frame_copyright");
	gtk_container_set_border_width(GTK_CONTAINER(frame_copyright), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_copyright), GTK_SHADOW_NONE);
	gtk_widget_show(frame_copyright);
	gtk_box_pack_start(GTK_BOX(vbox_about_main), frame_copyright, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(about_window, frame_copyright, "frame_copyright");
	
	// Copyright label
	label_copyright = gtk_label_new (
		"(c) 1999-2002 by Stéphane Dallongeville\n"
		"(c) 2003-2004 by Stéphane Akhoun\n\n"
		"Gens/GS (c) 2008 by David Korth\n\n"
		"Visit the Gens Homepage:\n"
		"http://gens.consolemul.com\n\n"
		"For news on Gens/GS, visit Sonic Retro:\n"
		"http://www.sonicretro.org"
		);
	gtk_widget_set_name(label_copyright, "label_copyright");
	gtk_widget_show(label_copyright);
	gtk_container_add(GTK_CONTAINER(frame_copyright), label_copyright);
	GLADE_HOOKUP_OBJECT(about_window, label_copyright, "label_copyright");
	
	// Get the dialog action area.
	about_dialog_action_area = GTK_DIALOG(about_window)->action_area;
	gtk_widget_set_name(about_dialog_action_area, "about_dialog_action_area");
	gtk_widget_show(about_dialog_action_area);
	GLADE_HOOKUP_OBJECT_NO_REF(about_window, about_dialog_action_area, "about_dialog_action_area");
	
	// Add the OK button to the dialog action area.
	button_about_OK = gtk_button_new_from_stock("gtk-ok");
	gtk_widget_set_name(button_about_OK, "button_about_OK");
	GTK_WIDGET_SET_FLAGS(button_about_OK, GTK_CAN_DEFAULT);
	gtk_widget_show(button_about_OK);
	gtk_dialog_add_action_widget(GTK_DIALOG(about_window), button_about_OK, GTK_RESPONSE_OK);
	AddButtonCallback_Clicked(button_about_OK, on_button_about_OK_clicked);
	gtk_widget_add_accelerator(button_about_OK, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_about_OK, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	GLADE_HOOKUP_OBJECT(about_window, button_about_OK, "button_about_OK");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(about_window), accel_group);
	
	gtk_widget_show_all(about_window);
	return about_window;
}


/**
 * Window is closed.
 */
gboolean on_about_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(about_window);
	about_window = NULL;
	return FALSE;
}


/**
 * OK
 */
void on_button_about_OK_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(about_window);
	about_window = NULL;
}
