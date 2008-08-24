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


/**
 * create_game_genie(): Create the Controller Configuration Window.
 * @return Game Genie Window.
 */
GtkWidget* create_controller_config_window(void)
{
	GdkPixbuf *controller_config_window_icon_pixbuf;
	
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
	
	gtk_widget_show_all(controller_config_window);
	return controller_config_window;
}
