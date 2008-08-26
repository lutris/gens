/**
 * GENS: (GTK+) OpenGL Resolution Window Callbacks.
 */


#include <string.h>
#include "opengl_resolution_window.h"
#include "opengl_resolution_window_callbacks.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_opengl_resolution_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(opengl_resolution_window);
	opengl_resolution_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_glres_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(opengl_resolution_window);
	opengl_resolution_window = NULL;
}


/**
 * Apply
 */
void on_button_glres_Apply_clicked(GtkButton *button, gpointer user_data)
{
	GLRes_Save();
}


/**
 * Save
 */
void on_button_glres_Save_clicked(GtkButton *button, gpointer user_data)
{
	GLRes_Save();
	gtk_widget_destroy(opengl_resolution_window);
	opengl_resolution_window = NULL;
}
