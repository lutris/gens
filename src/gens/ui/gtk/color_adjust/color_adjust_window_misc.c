/**
 * GENS: (GTK+) Color Adjust Window - Miscellaneous Functions.
 */


#include <string.h>

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
#include "color_adjust_window_misc.h"
#include "gens/gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"

/**
 * Open_Color_Adjust(): Opens the Color Adjust window.
 */
void Open_Color_Adjust(void)
{
	GtkWidget *ca;
	
	ca = create_color_adjust_window();
	if (!ca)
	{
		// Either an error occurred while creating the Color Adjust window,
		// or the Color Adjust window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(ca), GTK_WINDOW(gens_window));
	
	// TODO: Load settings.
	
	// Show the Color Adjust window.
	gtk_widget_show_all(ca);
}


/**
 * CA_Save(): Save the settings.
 */
void CA_Save(void)
{
	// TODO: Save settings.
}
