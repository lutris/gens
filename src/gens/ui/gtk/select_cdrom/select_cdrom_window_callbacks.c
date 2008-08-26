/**
 * GENS: (GTK+) Select CD-ROM Window Callbacks.
 */


#include <string.h>
#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_select_cdrom_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(select_cdrom_window);
	select_cdrom_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_SelCD_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(select_cdrom_window);
	select_cdrom_window = NULL;
}


/**
 * Apply
 */
void on_button_SelCD_Apply_clicked(GtkButton *button, gpointer user_data)
{
	SelCD_Save();
}


/**
 * Save
 */
void on_button_SelCD_Save_clicked(GtkButton *button, gpointer user_data)
{
	SelCD_Save();
	gtk_widget_destroy(select_cdrom_window);
	select_cdrom_window = NULL;
}
