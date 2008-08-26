/**
 * GENS: (GTK+) Select CD-ROM Window - Miscellaneous Functions.
 */


#include <string.h>

#include "select_cdrom_window.h"
#include "select_cdrom_window_callbacks.h"
#include "select_cdrom_window_misc.h"
#include "gens/gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"


/**
 * Open_Select_CDROM(): Opens the Select CD-ROM window.
 */
void Open_Select_CDROM(void)
{
	GtkWidget *SelCD;
	
	SelCD = create_select_cdrom_window();
	if (!SelCD)
	{
		// Either an error occurred while creating the Select CD-ROM window,
		// or the Select CD-ROM window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(SelCD), GTK_WINDOW(gens_window));
	
	// Load settings.
	// TODO
	
	// Show the Select CD-ROM window.
	gtk_widget_show_all(SelCD);
}


/**
 * SelCD_Save(): Save the settings.
 */
void SelCD_Save(void)
{
	GtkWidget *spinbutton_width, *spinbutton_height;
	
	// Save settings.
	// TODO
}
