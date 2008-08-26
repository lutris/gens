/**
 * GENS: (GTK+) OpenGL Resolution Window - Miscellaneous Functions.
 */


#include <string.h>

#include "opengl_resolution_window.h"
#include "opengl_resolution_window_callbacks.h"
#include "opengl_resolution_window_misc.h"
#include "gens/gens_window.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "g_palette.h"
#include "vdp_io.h"
#include "g_main.h"
#include "g_sdldraw.h"


/**
 * Open_OpenGL_Resolution(): Opens the OpenGL Resolution window.
 */
void Open_OpenGL_Resolution(void)
{
	GtkWidget *glres;
	
	glres = create_opengl_resolution_window();
	if (!glres)
	{
		// Either an error occurred while creating the OpenGL Resolution window,
		// or the OpenGL Resolution window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(glres), GTK_WINDOW(gens_window));
	
	// Load settings.
	// TODO
	
	// Show the OpenGL Resolution window.
	gtk_widget_show_all(glres);
}


/**
 * GLRes_Save(): Save the settings.
 */
void GLRes_Save(void)
{
	// TODO
}
