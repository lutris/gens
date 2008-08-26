/**
 * GENS: (GTK+) Color Adjust Window Callbacks.
 */


#include <string.h>
#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_color_adjust_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_ca_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
}


/**
 * Apply
 */
void on_button_ca_Apply_clicked(GtkButton *button, gpointer user_data)
{
	CA_Save();
}


/**
 * Save
 */
void on_button_ca_Save_clicked(GtkButton *button, gpointer user_data)
{
	CA_Save();
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
}
