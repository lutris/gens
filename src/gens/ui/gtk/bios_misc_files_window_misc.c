/**
 * GENS: (GTK+) BIOS/Misc Files Window - Miscellaneous Functions.
 */

#include <string.h>

#include "bios_misc_files_window.h"
#include "bios_misc_files_window_callbacks.h"
#include "bios_misc_files_window_misc.h"
#include "gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"


/**
 * Open_BIOS_Misc_Files(): Opens the BIOS/Misc Files window.
 */
void Open_BIOS_Misc_Files(void)
{
	GtkWidget *bmf;
	
	bmf = create_bios_misc_files_window();
	if (!bmf)
	{
		// Either an error occurred while creating the Controller Configuration window,
		// or the Controller Configuration window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(bmf), GTK_WINDOW(gens_window));
	
	// Show the BIOS/Misc Files window.
	gtk_widget_show_all(bios_misc_files_window);
}
