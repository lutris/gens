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
 * Cancel
 */
void on_button_cc_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(controller_config_window);
	controller_config_window = NULL;
}
