/**
 * GENS: (GTK+) BIOS/Misc Files Window Callbacks.
 */


#include <string.h>
#include "bios_misc_files_window.h"
#include "bios_misc_files_window_callbacks.h"
#include "bios_misc_files_window_misc.h"

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
 * Change...
 */
void on_button_bmf_Change_clicked(GtkButton *button, gpointer user_data)
{
	int fileID = GPOINTER_TO_INT(user_data);
	printf("CHANGE: File %d\n", fileID);
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
	BIOS_Misc_Files_Save();
	gtk_widget_destroy(bios_misc_files_window);
	bios_misc_files_window = NULL;
}
