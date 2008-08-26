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

#include "g_main.h"
#include "g_sdldraw.h"


/**
 * Open_OpenGL_Resolution(): Opens the OpenGL Resolution window.
 */
void Open_OpenGL_Resolution(void)
{
	GtkWidget *GLRes;
	GtkWidget *spinbutton_width, *spinbutton_height;
	
	GLRes = create_opengl_resolution_window();
	if (!GLRes)
	{
		// Either an error occurred while creating the OpenGL Resolution window,
		// or the OpenGL Resolution window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(GLRes), GTK_WINDOW(gens_window));
	
	// Load settings.
	spinbutton_width = lookup_widget(GLRes, "spinbutton_width");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_width), Width_gl);
	spinbutton_height = lookup_widget(GLRes, "spinbutton_height");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_height), Height_gl);
	
	// Show the OpenGL Resolution window.
	gtk_widget_show_all(GLRes);
}


/**
 * GLRes_Save(): Save the settings.
 */
void GLRes_Save(void)
{
	GtkWidget *spinbutton_width, *spinbutton_height;
	
	// Save settings.
	// TODO: If this is a predefined resolution,
	// uncheck "Custom" and check the predefined resolution.
	spinbutton_width = lookup_widget(opengl_resolution_window, "spinbutton_width");
	spinbutton_height = lookup_widget(opengl_resolution_window, "spinbutton_height");
	
	Set_GL_Resolution(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbutton_width)),
			  gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbutton_height)));
}
