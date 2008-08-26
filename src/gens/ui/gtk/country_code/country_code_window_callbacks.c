/**
 * GENS: (GTK+) Country Code Window Callbacks.
 */


#include <string.h>
#include "country_code_window.h"
#include "country_code_window_callbacks.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_country_code_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(country_code_window);
	country_code_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_Country_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(country_code_window);
	country_code_window = NULL;
}


/**
 * Apply
 */
void on_button_Country_Apply_clicked(GtkButton *button, gpointer user_data)
{
	Country_Save();
}


/**
 * Save
 */
void on_button_Country_Save_clicked(GtkButton *button, gpointer user_data)
{
	Country_Save();
	gtk_widget_destroy(country_code_window);
	country_code_window = NULL;
}
