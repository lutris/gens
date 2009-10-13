/***************************************************************************
 * Gens: (GTK+) BIOS/Misc Files Window.                                    *
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

#include "bmf_window.hpp"
#include "ui/common/bmf_window_common.h"
#include "gens/gens_window.h"

#include "gens_ui.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

// GTK+ includes.
#include <gtk/gtk.h>

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"


// Window.
GtkWidget *bmf_window = NULL;

// Widgets.
static GtkWidget	*txtFile[12];
static GtkWidget	*btnCancel, *btnApply, *btnSave;

// Configuration load/save functions.
static void	bmf_window_init(void);
static void	bmf_window_save(void);

// Callbacks.
static gboolean	bmf_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	bmf_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	bmf_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data);
static void	bmf_window_callback_txtFile_changed(GtkEditable *editable, gpointer user_data);


/**
 * bmf_window_show(): Show the BIOS/Misc Files window.
 */
void bmf_window_show(void)
{
	if (bmf_window)
	{
		// BIOS/Misc Files window is already visible. Set focus.
		gtk_widget_grab_focus(bmf_window);
		return;
	}
	
	// Create the window.
	bmf_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(bmf_window), 4);
	gtk_window_set_title(GTK_WINDOW(bmf_window), "BIOS/Misc Files");
	gtk_window_set_position(GTK_WINDOW(bmf_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(bmf_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(bmf_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(bmf_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)bmf_window, "delete_event",
			 G_CALLBACK(bmf_window_callback_close), NULL);
	g_signal_connect((gpointer)bmf_window, "destroy_event",
			 G_CALLBACK(bmf_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(bmf_window), "response",
			 G_CALLBACK(bmf_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(bmf_window));
	gtk_widget_show(vboxDialog);
	
	// Create all frames.
	int tblRow = 0;
	GtkWidget *tblBMF = NULL;
	
	for (int file = 0; bmf_entries[file].title != NULL; file++)
	{
		if (!bmf_entries[file].entry)
		{
			// No entry buffer. This is a new frame.
			GtkWidget *fraBMF = gtk_frame_new(bmf_entries[file].title);
			gtk_frame_set_shadow_type(GTK_FRAME(fraBMF), GTK_SHADOW_ETCHED_IN);
			gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraBMF))), true);
			gtk_container_set_border_width(GTK_CONTAINER(fraBMF), 4);
			gtk_box_pack_start(GTK_BOX(vboxDialog), fraBMF, true, true, 0);
			
			// Add the frame table.
			tblBMF = gtk_table_new(1, 3, false);
			gtk_container_set_border_width(GTK_CONTAINER(tblBMF), 8);
			gtk_table_set_row_spacings(GTK_TABLE(tblBMF), 4);
			gtk_table_set_col_spacings(GTK_TABLE(tblBMF), 4);
			gtk_widget_show(tblBMF);
			gtk_container_add(GTK_CONTAINER(fraBMF), tblBMF);
			
			// Reset the table row.
			tblRow = 0;
		}
		else
		{
			// File entry.
			
			// Check if the table needs to be resized.
			if (tblRow > 0)
				gtk_table_resize(GTK_TABLE(tblBMF), tblRow + 1, 3);
			
			// Create the label for the file.
			GtkWidget *lblFile = gtk_label_new(bmf_entries[file].title);
			gtk_widget_set_size_request(lblFile, 85, -1);
			gtk_misc_set_alignment(GTK_MISC(lblFile), 0.0f, 0.5f);
			gtk_widget_show(lblFile);
			gtk_table_attach(GTK_TABLE(tblBMF), lblFile,
					 0, 1, tblRow, tblRow + 1,
					 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			
			// Create the textbox for the file.
			txtFile[file] = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(txtFile[file]), GENS_PATH_MAX - 1);
			gtk_widget_set_size_request(txtFile[file], 256, -1);
			gtk_widget_show(txtFile[file]);
			gtk_table_attach(GTK_TABLE(tblBMF), txtFile[file],
					 1, 2, tblRow, tblRow + 1,
					 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			g_signal_connect((gpointer)txtFile[file], "changed",
					 G_CALLBACK(bmf_window_callback_txtFile_changed),
					 GINT_TO_POINTER(file));
			
			// Create the "Change..." button for the file.
			GtkWidget *btnChange = gtk_button_new_with_label("Change...");
			gtk_widget_show(btnChange);
			gtk_table_attach(GTK_TABLE(tblBMF), btnChange,
					 2, 3, tblRow, tblRow + 1,
					 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					 (GtkAttachOptions)(0), 0, 0);
			g_signal_connect((gpointer)btnChange, "clicked",
					 G_CALLBACK(bmf_window_callback_btnChange_clicked),
					 GINT_TO_POINTER(file));
			
			// Next table row.
			tblRow++;
		}
	}
	
	// Create the dialog buttons.
	btnApply  = gtk_dialog_add_button(GTK_DIALOG(bmf_window), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
	btnCancel = gtk_dialog_add_button(GTK_DIALOG(bmf_window), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	btnSave   = gtk_dialog_add_button(GTK_DIALOG(bmf_window), GTK_STOCK_SAVE, GTK_RESPONSE_OK);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(bmf_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						GTK_RESPONSE_APPLY,
						-1);
#endif
	
	// Initialize the internal data variables.
	bmf_window_init();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(bmf_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(bmf_window);
}


/**
 * bmf_window_close(): Close the About window.
 */
void bmf_window_close(void)
{
	if (!bmf_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(bmf_window);
	bmf_window = NULL;
}


/**
 * bmf_window_init(): Initialize the file text boxes.
 */
static void bmf_window_init(void)
{
	for (int file = 0; bmf_entries[file].title != NULL; file++)
	{
		if (!bmf_entries[file].entry)
			continue;
		
		// Set the entry text.
		gtk_entry_set_text(GTK_ENTRY(txtFile[file]), bmf_entries[file].entry);
	}
	
	// Disable the "Apply" button initially.
	gtk_widget_set_sensitive(btnApply, false);
}


/**
 * bmf_window_save(): Save the BIOS/Misc Files.
 */
static void bmf_window_save(void)
{
	for (int file = 0; bmf_entries[file].title != NULL; file++)
	{
		if (!bmf_entries[file].entry)
			continue;
		
		// Save the entry text.
		strlcpy(bmf_entries[file].entry, gtk_entry_get_text(GTK_ENTRY(txtFile[file])), GENS_PATH_MAX);
	}
	
	// Disable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, false);
}


/**
 * bmf_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean bmf_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	bmf_window_close();
	return FALSE;
}


/**
 * bmf_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void bmf_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(dialog);
	GSFT_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			bmf_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			bmf_window_save();
			break;
		case GTK_RESPONSE_OK:
			bmf_window_save();
			bmf_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * bmf_window_callback_btnChange_clicked(): A "Change..." button was clicked.
 * @param button Button that was clicked.
 * @param user_data File ID number.
 */
static void bmf_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(button);
	
	const int file = GPOINTER_TO_INT(user_data);
	
	// Check that this is a valid file entry.
	if (!bmf_entries[file].entry)
		return;
	
	char tmp[128];
	szprintf(tmp, sizeof(tmp), "Select %s File", bmf_entries[file].title);
	
	// Request a new file.
	string new_file = GensUI::openFile(tmp, gtk_entry_get_text(GTK_ENTRY(txtFile[file])),
					   bmf_entries[file].filter);
	
	// If "Cancel" was selected, don't do anything.
	if (new_file.empty())
		return;
	
	// Set the new file.
	gtk_entry_set_text(GTK_ENTRY(txtFile[file]), new_file.c_str());
}


/**
 * bmf_window_callback_txtFile_changed(): One of the textboxes was changed.
 * @param editable
 * @param user_data File ID number.
 */
static void bmf_window_callback_txtFile_changed(GtkEditable *editable, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(editable);
	GSFT_UNUSED_PARAMETER(user_data);
	
	// Enable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, true);
}
