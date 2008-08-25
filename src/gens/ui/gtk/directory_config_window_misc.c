/**
 * GENS: (GTK+) Directory Configuration Window - Miscellaneous Functions.
 */

#include <string.h>

#include "directory_config_window.h"
#include "directory_config_window_callbacks.h"
#include "directory_config_window_misc.h"
#include "gens_window.h"

#include "g_main.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"


/**
 * Open_Directory_Config(): Opens the Directory Configuration window.
 */
void Open_Directory_Config(void)
{
	GtkWidget *bmf;
	
	bmf = create_directory_config_window();
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
 * Directory_Config_Change(): Change a directory.
 * @param dir Directory ID.
 */
void Directory_Config_Change(int dir)
{
	// TODO
	/*
	GtkWidget *entry_file;
	char tmp[64];
	char newFile[GENS_PATH_MAX];
	int ret;
	
	// Get the entry widget for this file.
	sprintf(tmp, "entry_%s", BIOSMiscFiles[file].tag);
	entry_file = lookup_widget(directory_config_window, tmp);
	
	// Request a new file.
	sprintf(tmp, "Select %s File", BIOSMiscFiles[file].title);
	ret = UI_OpenFile(tmp, gtk_entry_get_text(GTK_ENTRY(entry_file)),
			  BIOSMiscFiles[file].filter, newFile);
	
	// If Cancel was selected, don't do anything.
	if (ret)
		return;
	
	// Set the new file.
	gtk_entry_set_text(GTK_ENTRY(entry_file), newFile);
	*/
}


/**
 * Directory_Config_Save(): Save the Directories.
 */
void Directory_Config_Save(void)
{
	// TODO
	/*
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
		entry_file = lookup_widget(directory_config_window, tmp);
		
		// Get the entry text.
		strncpy(BIOSMiscFiles[file].entry, gtk_entry_get_text(GTK_ENTRY(entry_file)), GENS_PATH_MAX);
		
		// Increment the file counter.
		file++;
	}
	*/
}
