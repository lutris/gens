/**
 * GENS: (GTK+) General Options Window Callbacks.
 */


#include <string.h>
#include "general_options_window.h"
#include "general_options_window_callbacks.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_general_options_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(general_options_window);
	general_options_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_go_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(general_options_window);
	general_options_window = NULL;
}


/**
 * Apply
 */
void on_button_go_Apply_clicked(GtkButton *button, gpointer user_data)
{
	General_Options_Save();
}


/**
 * Save
 */
void on_button_go_Save_clicked(GtkButton *button, gpointer user_data)
{
	General_Options_Save();
	gtk_widget_destroy(general_options_window);
	general_options_window = NULL;
}
