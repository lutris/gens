/**
 * GENS: (GTK+) BIOS/Misc Files Window - Miscellaneous Functions.
 */

#include <string.h>

#include "bios_misc_files_window.h"
#include "bios_misc_files_window_callbacks.h"
#include "bios_misc_files_window_misc.h"
#include "gens_window.h"

#include "g_main.h"

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
	gtk_widget_show_all(bmf);
}


/**
 * BIOS_Misc_Files_Save(): Save the BIOS/Misc Files.
 */
void BIOS_Misc_Files_Save(void)
{
	int file = 0;
	char tmp[64];
	GtkWidget *entry_file;
	
	// Go through the BIOSMiscFiles[] struct.
	while (BIOSMiscFiles[file].title)
	{
		if (!BIOSMiscFiles[file].entry)
		{
			// Skip frame headers here. We just want to get file entries.
			file++;
			continue;
		}
		
		// Get the entry object.
		sprintf(tmp, "entry_%s", BIOSMiscFiles[file].tag);
		entry_file = lookup_widget(bios_misc_files_window, tmp);
		
		// Get the entry text.
		strncpy(BIOSMiscFiles[file].entry, gtk_entry_get_text(GTK_ENTRY(entry_file)), GENS_PATH_MAX);
		
		// Increment the file counter.
		file++;
	}	
}
