/**
 * GENS: (GTK+) Directory Configuration Window - Miscellaneous Functions.
 */


#include <string.h>

#include "directory_config_window.h"
#include "directory_config_window_callbacks.h"
#include "directory_config_window_misc.h"
#include "gens/gens_window.h"

#include "g_main.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"


/**
 * Open_Directory_Config(): Opens the Directory Configuration window.
 */
void Open_Directory_Config(void)
{
	GtkWidget *dc;
	
	dc = create_directory_config_window();
	if (!dc)
	{
		// Either an error occurred while creating the Controller Configuration window,
		// or the Controller Configuration window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(dc), GTK_WINDOW(gens_window));
	
	// Show the BIOS/Misc Files window.
	gtk_widget_show_all(dc);
}


/**
 * Directory_Config_Change(): Change a directory.
 * @param dir Directory ID.
 */
void Directory_Config_Change(int dir)
{
	GtkWidget *entry_direntry;
	char tmp[64];
	char newDir[GENS_PATH_MAX];
	int ret;
	
	// Get the entry widget for this directory.
	sprintf(tmp, "entry_%s", DirEntries[dir].tag);
	entry_direntry = lookup_widget(directory_config_window, tmp);
	
	// Request a new directory.
	sprintf(tmp, "Select %s Directory", DirEntries[dir].title);
	ret = UI_SelectDir(tmp, gtk_entry_get_text(GTK_ENTRY(entry_direntry)), newDir);
	
	// If Cancel was selected, don't do anything.
	if (ret)
		return;
	
	// Set the new directory.
	gtk_entry_set_text(GTK_ENTRY(entry_direntry), newDir);
}


/**
 * Directory_Config_Save(): Save the Directories.
 */
void Directory_Config_Save(void)
{
	int dir = 0;
	char tmp[64];
	GtkWidget *entry_direntry;
	
	// Go through the DirEntries[] struct.
	while (DirEntries[dir].title)
	{
		// Get the entry object.
		sprintf(tmp, "entry_%s", DirEntries[dir].tag);
		entry_direntry = lookup_widget(directory_config_window, tmp);
		
		// Get the entry text.
		strncpy(DirEntries[dir].entry, gtk_entry_get_text(GTK_ENTRY(entry_direntry)), GENS_PATH_MAX);
		
		// Increment the directory counter.
		dir++;
	}
}
