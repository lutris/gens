/**
 * GENS: (GTK+) Directory Configuration Window Callbacks.
 */


#include <string.h>
#include "directory_config_window.h"
#include "directory_config_window_callbacks.h"
#include "directory_config_window_misc.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_directory_config_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(directory_config_window);
	directory_config_window = NULL;
	return FALSE;
}


/**
 * Change...
 */
void on_button_dc_Change_clicked(GtkButton *button, gpointer user_data)
{
	Directory_Config_Change(GPOINTER_TO_INT(user_data));
}


/**
 * Cancel
 */
void on_button_dc_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(directory_config_window);
	directory_config_window = NULL;
}


/**
 * Apply
 */
void on_button_dc_Apply_clicked(GtkButton *button, gpointer user_data)
{
	Directory_Config_Save();
}


/**
 * Save
 */
void on_button_dc_Save_clicked(GtkButton *button, gpointer user_data)
{
	Directory_Config_Save();
	gtk_widget_destroy(directory_config_window);
	directory_config_window = NULL;
}
