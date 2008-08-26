/**
 * GENS: (GTK+) Country Code Window - Miscellaneous Functions.
 */


#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "country_code_window.h"
#include "country_code_window_callbacks.h"
#include "country_code_window_misc.h"
#include "gens/gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "rom.h"


/**
 * Open_Country_Code(): Opens the Country Code window.
 */
void Open_Country_Code(void)
{
	GtkWidget *Country;
	
	Country = create_country_code_window();
	if (!Country)
	{
		// Either an error occurred while creating the Country Code window,
		// or the Country Code window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(Country), GTK_WINDOW(gens_window));
	
	// Load settings.
	// TODO
	
	// Show the Country Code window.
	gtk_widget_show_all(Country);
}


/**
 * Country_Save(): Save the settings.
 */
void Country_Save(void)
{
	// Save settings.
	// TODO
}
