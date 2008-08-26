/**
 * GENS: (GTK+) General Options Window Callbacks.
 */


#include <string.h>
#include "general_options_window.h"
#include "general_options_window_callbacks.h"

#include "gtk-misc.h"


/**
 * STUB: Indicates that this function is a stub.
 */
#define STUB fprintf(stderr, "TODO: STUB: %s()\n", __func__);


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
 * Save
 */
void on_button_go_Save_clicked(GtkButton *button, gpointer user_data)
{
	STUB;
	//General_Options_Save();
	gtk_widget_destroy(general_options_window);
	general_options_window = NULL;
}
