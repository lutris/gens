/**
 * GENS: (GTK+) BIOS/Misc Files Window Callbacks.
 */


#include <string.h>
#include "bios_misc_files_window.h"
#include "bios_misc_files_window_callbacks.h"

#include "gtk-misc.h"


/**
 * STUB: Indicates that this function is a stub.
 */
#define STUB fprintf(stderr, "TODO: STUB: %s()\n", __func__);


/**
 * Window is closed.
 */
gboolean on_bios_misc_files_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(bios_misc_files_window);
	bios_misc_files_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_bmf_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(bios_misc_files_window);
	bios_misc_files_window = NULL;
}


/**
 * Save
 */
void on_button_bmf_Save_clicked(GtkButton *button, gpointer user_data)
{
	// TODO
	STUB;
	//BIOSMiscFiles_Save();
	gtk_widget_destroy(bios_misc_files_window);
	bios_misc_files_window = NULL;
}
