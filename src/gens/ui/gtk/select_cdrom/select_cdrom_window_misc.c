/**
 * GENS: (GTK+) Select CD-ROM Window - Miscellaneous Functions.
 */


#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"
#include "select_cdrom_window_misc.h"
#include "gens/gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "rom.h"


// CD-ROM device prefixes.
const char* CDROM_Prefix[4] = {"/dev/cdrom", "/dev/scd", "/dev/sr", NULL};

/**
 * Open_Select_CDROM(): Opens the Select CD-ROM window.
 */
void Open_Select_CDROM(void)
{
	GtkWidget *SelCD;
	GtkWidget *combo_drive;
	int curPrefix, i; char tmp[64];
	struct stat fileStat;
	
	SelCD = create_select_cdrom_window();
	if (!SelCD)
	{
		// Either an error occurred while creating the Select CD-ROM window,
		// or the Select CD-ROM window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(SelCD), GTK_WINDOW(gens_window));
	
	// Load settings.
	
	// CD-ROM drive
	combo_drive = lookup_widget(SelCD, "combo_drive");
	gtk_entry_set_text(GTK_ENTRY(GTK_BIN(combo_drive)->child), CDROM_DEV);
	
	// Add CD-ROM drives to the dropdown.
	// TODO: Improve this using udev or something.
	curPrefix = 0;
	while (CDROM_Prefix[curPrefix])
	{
		for (i = -1; i <= 9; i++)
		{
			if (i == -1)
				strcpy(tmp, CDROM_Prefix[curPrefix]);
			else
				sprintf(tmp, "%s%d", CDROM_Prefix[curPrefix], i);
				
			if (lstat(tmp, &fileStat))
			{
				// Error occurred while stat'ing this file. Skip it.
				continue;
			}
			
			// File exists.
			
			// If this isn't "/dev/cdrom*", don't add it if it's a symlink.
			if (curPrefix != 0 && S_ISLNK(fileStat.st_mode))
				continue;
			
			// If this is neither a symlink nor a block device, don't add it.
			if (!S_ISLNK(fileStat.st_mode) && !S_ISBLK(fileStat.st_mode))
				continue;
			
			// Add the device file.
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo_drive), tmp);
		}
		
		// Next prefix.
		curPrefix++;
	}
	
	// Show the Select CD-ROM window.
	gtk_widget_show_all(SelCD);
}


/**
 * SelCD_Save(): Save the settings.
 */
void SelCD_Save(void)
{
	GtkWidget *combo_drive;
	gchar *tmpDrive;
	// Save settings.
	
	// CD-ROM drive
	combo_drive = lookup_widget(select_cdrom_window, "combo_drive");
	tmpDrive = strdup(gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo_drive)));
	if (strlen(tmpDrive))
	{
		strncpy(CDROM_DEV, tmpDrive, 63);
		CDROM_DEV[63] = 0x00;
	}
	free(tmpDrive);
}
