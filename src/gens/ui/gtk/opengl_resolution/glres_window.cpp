/***************************************************************************
 * Gens: (GTK+) OpenGL Resolution Window.                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "glres_window.hpp"
#include "gens/gens_window.h"

// GTK+ includes.
#include <gtk/gtk.h>

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

// Gens options.
#include "emulator/g_main.hpp"
#include "emulator/options.hpp"


// Window.
GtkWidget *glres_window = NULL;

// Widgets.
static GtkWidget	*spnWidth;
static GtkWidget	*spnHeight;
static GtkWidget	*btnCancel, *btnApply, *btnSave;

// OpenGL resolution load/save functions.
static void	glres_window_init(void);
static void	glres_window_save(void);

// Callbacks.
static gboolean	glres_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	glres_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	glres_window_callback_spn_value_changed(GtkSpinButton *spinbutton, gpointer user_data);


/**
 * glres_window_show(): Show the OpenGL Resolution window.
 */
void glres_window_show(void)
{
	if (glres_window)
	{
		// OpenGL Resolution window is already visible. Set focus.
		gtk_widget_grab_focus(glres_window);
		return;
	}
	
	// Create the window.
	glres_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(glres_window), 4);
	gtk_window_set_title(GTK_WINDOW(glres_window), "OpenGL Resolution");
	gtk_window_set_position(GTK_WINDOW(glres_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(glres_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(glres_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(glres_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)(glres_window), "delete_event",
			 G_CALLBACK(glres_window_callback_close), NULL);
	g_signal_connect((gpointer)(glres_window), "destroy_event",
			 G_CALLBACK(glres_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(glres_window), "response",
			 G_CALLBACK(glres_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(glres_window));
	gtk_widget_show(vboxDialog);
	
	// Create the main HBox.
	GtkWidget *hboxMain = gtk_hbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hboxMain), 4);
	gtk_widget_show(hboxMain);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxMain, TRUE, TRUE, 0);
	
	// Create the Width spinbutton label.
	GtkWidget *lblWidth = gtk_label_new_with_mnemonic("_Width");
	gtk_misc_set_alignment(GTK_MISC(lblWidth), 0.0f, 0.5f);
	gtk_widget_show(lblWidth);
	gtk_box_pack_start(GTK_BOX(hboxMain), lblWidth, FALSE, FALSE, 0);
	
	// Create the adjustment object for the Width spinbutton.
	GtkObject *adjWidth = gtk_adjustment_new(1, 0, 9999, 1, 10, 0);

	// Create the Width spinbutton.
	spnWidth = gtk_spin_button_new(GTK_ADJUSTMENT(adjWidth), 1, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lblWidth), spnWidth);
	gtk_widget_show(spnWidth);
	gtk_box_pack_start(GTK_BOX(hboxMain), spnWidth, FALSE, FALSE, 0);
	g_signal_connect((gpointer)spnWidth, "value-changed",
			 G_CALLBACK(glres_window_callback_spn_value_changed), NULL);
	
	// Create the separator.
	GtkWidget *vsepResolution = gtk_vseparator_new();
	gtk_widget_show(vsepResolution);
	gtk_box_pack_start(GTK_BOX(hboxMain), vsepResolution, TRUE, TRUE, 0);
	
	// Create the Height spinbutton label.
	GtkWidget *lblHeight = gtk_label_new_with_mnemonic("_Height");
	gtk_misc_set_alignment(GTK_MISC(lblHeight), 0.0f, 0.5f);
	gtk_widget_show(lblHeight);
	gtk_box_pack_start(GTK_BOX(hboxMain), lblHeight, FALSE, FALSE, 0);
	
	// Create the adjustment object for the Height spinbutton.
	GtkObject *adjHeight = gtk_adjustment_new(1, 0, 9999, 1, 10, 0);
	
	// Create the Height spinbutton.
	spnHeight = gtk_spin_button_new(GTK_ADJUSTMENT(adjHeight), 1, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lblHeight), spnHeight);
	gtk_widget_show(spnHeight);
	gtk_box_pack_start(GTK_BOX(hboxMain), spnHeight, FALSE, FALSE, 0);
	g_signal_connect((gpointer)spnHeight, "value-changed",
			 G_CALLBACK(glres_window_callback_spn_value_changed), NULL);
	
	// Create the dialog buttons.
	btnApply  = gtk_dialog_add_button(GTK_DIALOG(glres_window), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
	btnCancel = gtk_dialog_add_button(GTK_DIALOG(glres_window), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	btnSave   = gtk_dialog_add_button(GTK_DIALOG(glres_window), GTK_STOCK_SAVE, GTK_RESPONSE_OK);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(glres_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						GTK_RESPONSE_APPLY,
						-1);
#endif
	
	// Initialize the OpenGL resolution spinbuttons.
	glres_window_init();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(glres_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(glres_window);
}


/**
 * glres_window_close(): Close the OpenGL Resolution window.
 */
void glres_window_close(void)
{
	if (!glres_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(glres_window);
	glres_window = NULL;
}


/**
 * glres_window_init(): Initialize the OpenGL resolution spinbuttons.
 */
static void glres_window_init(void)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spnWidth), Video.GL.width);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spnHeight), Video.GL.height);
	
	// Disable the "Apply" button initially.
	gtk_widget_set_sensitive(btnApply, false);
}


/**
 * glres_window_save(): Save the OpenGL resolution.
 */
static void glres_window_save(void)
{
	const int w = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spnWidth));
	const int h = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spnHeight));
	
	Options::setOpenGL_Resolution(w, h);
	
	// Disable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, false);
}


/**
 * glres_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean glres_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	glres_window_close();
	return FALSE;
}


/**
 * glres_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void glres_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(dialog);
	GSFT_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			glres_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			glres_window_save();
			break;
		case GTK_RESPONSE_OK:
			glres_window_save();
			glres_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * glres_window_callback_spn_value_changed(): One of the spinbuttons was changed.
 * @param spinbutton
 * @param user_data
 */
static void glres_window_callback_spn_value_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(spinbutton);
	GSFT_UNUSED_PARAMETER(user_data);
	
	// Enable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, true);
}
