/***************************************************************************
 * Gens: (GTK+) Directory Configuration Window.                            *
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

#include "dir_window.hpp"
#include "ui/common/dir_window_common.h"

// C includes.
#include <string.h>

// C++ includes.
#include <string>
using std::string;

// GTK+ includes.
#include <gtk/gtk.h>

// Unused Parameter macro.
#include "macros/unused.h"

// Gens includes.
#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "util/file/file.hpp"


// Window.
GtkWidget *dir_window = NULL;

// Widgets.
static GtkWidget	*txtDirectory[DIR_WINDOW_ENTRIES_COUNT];

// Directory configuration load/save functions.
static void	dir_window_init(void);
static void	dir_window_save(void);

// Callbacks.
static gboolean	dir_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	dir_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	dir_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data);


/**
 * dir_window_show(): Show the Directory Configuration window.
 * @param parent Parent window.
 */
void dir_window_show(GtkWindow *parent)
{
	if (dir_window)
	{
		// Directory Configuration window is already visible. Set focus.
		gtk_widget_grab_focus(dir_window);
		return;
	}
	
	// Create the window.
	dir_window = gtk_dialog_new();
	gtk_widget_set_name(dir_window, "dir_window");
	gtk_container_set_border_width(GTK_CONTAINER(dir_window), 4);
	gtk_window_set_title(GTK_WINDOW(dir_window), "Configure Directories");
	gtk_window_set_position(GTK_WINDOW(dir_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dir_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(dir_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(dir_window), FALSE);
	g_object_set_data(G_OBJECT(dir_window), "dir_window", dir_window);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)(dir_window), "delete_event",
			 G_CALLBACK(dir_window_callback_close), NULL);
	g_signal_connect((gpointer)(dir_window), "destroy_event",
			 G_CALLBACK(dir_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(dir_window), "response",
			 G_CALLBACK(dir_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(dir_window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(dir_window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the directory entry frame.
	GtkWidget *fraDirectories = gtk_frame_new("<b><i>Configure Directories</i></b>");
	gtk_widget_set_name(fraDirectories, "fraDirectories");
	gtk_frame_set_shadow_type(GTK_FRAME(fraDirectories), GTK_SHADOW_ETCHED_IN);
	gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraDirectories))), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(fraDirectories), 4);
	gtk_widget_show(fraDirectories);
	gtk_box_pack_start(GTK_BOX(vboxDialog), fraDirectories, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(dir_window), "fraDirectories",
			       g_object_ref(fraDirectories), (GDestroyNotify)g_object_unref);
	
	// Create the table for the directories.
	GtkWidget *tblDirectories = gtk_table_new(DIR_WINDOW_ENTRIES_COUNT, 3, FALSE);
	gtk_widget_set_name(tblDirectories, "tblDirectories");
	gtk_container_set_border_width(GTK_CONTAINER(tblDirectories), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tblDirectories), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tblDirectories), 4);
	gtk_widget_show(tblDirectories);
	gtk_container_add(GTK_CONTAINER(fraDirectories), tblDirectories);
	g_object_set_data_full(G_OBJECT(dir_window), "tblDirectories",
			       g_object_ref(tblDirectories), (GDestroyNotify)g_object_unref);
	
	// Create all directory entry widgets.
	char tmp[64];
	for (unsigned int dir = 0; dir < DIR_WINDOW_ENTRIES_COUNT; dir++)
	{
		// Create tbe label for the directory.
		sprintf(tmp, "lblDirectory_%d", dir);
		GtkWidget *lblDirectory = gtk_label_new(dir_window_entries[dir].title);
		gtk_widget_set_name(lblDirectory, tmp);
		gtk_misc_set_alignment(GTK_MISC(lblDirectory), 0.0f, 0.5f);
		gtk_widget_show(lblDirectory);
		gtk_table_attach(GTK_TABLE(tblDirectories), lblDirectory,
				 0, 1, dir, dir + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
		g_object_set_data_full(G_OBJECT(dir_window), tmp,
				       g_object_ref(lblDirectory), (GDestroyNotify)g_object_unref);
		
		// Create the textbox for the directory.
		sprintf(tmp, "txtDirectory_%d", dir);
		txtDirectory[dir] = gtk_entry_new();
		gtk_widget_set_name(txtDirectory[dir], tmp);
		gtk_entry_set_max_length(GTK_ENTRY(txtDirectory[dir]), GENS_PATH_MAX - 1);
		gtk_widget_set_size_request(txtDirectory[dir], 256, -1);
		gtk_widget_show(txtDirectory[dir]);
		gtk_table_attach(GTK_TABLE(tblDirectories), txtDirectory[dir],
				 1, 2, dir, dir + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(dir_window), tmp,
				       g_object_ref(txtDirectory[dir]), (GDestroyNotify)g_object_unref);
		
		// Create the "Change" button for the directory.
		// TODO: Use an icon?
		sprintf(tmp, "btnChange_%d", dir);
		GtkWidget *btnChange = gtk_button_new_with_label("Change...");
		gtk_widget_set_name(btnChange, tmp);
		gtk_widget_show(btnChange);
		gtk_table_attach(GTK_TABLE(tblDirectories), btnChange,
				 2, 3, dir, dir + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_object_set_data_full(G_OBJECT(dir_window), tmp,
				       g_object_ref(btnChange), (GDestroyNotify)g_object_unref);
		
		// Connect the "clicked" signal for the "Change" button.
		g_signal_connect(GTK_OBJECT(btnChange), "clicked",
				 G_CALLBACK(dir_window_callback_btnChange_clicked), GINT_TO_POINTER(dir));
	}
	
	// Create the dialog buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(dir_window),
			       "gtk-cancel", GTK_RESPONSE_CANCEL,
			       "gtk-apply", GTK_RESPONSE_APPLY,
			       "gtk-save", GTK_RESPONSE_OK,
			       NULL);
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dir_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_APPLY,
						GTK_RESPONSE_CANCEL,
						-1);
#endif
	
	// Initialize the directory entries.
	dir_window_init();
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(dir_window), parent);
	
	// Show the window.
	gtk_widget_show_all(dir_window);
}


/**
 * dir_window_close(): Close the OpenGL Resolution window.
 */
void dir_window_close(void)
{
	if (!dir_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(dir_window);
	dir_window = NULL;
}


/**
 * dir_window_init(): Initialize the Directory Configuration entries.
 */
static void dir_window_init(void)
{
	for (unsigned int dir = 0; dir < DIR_WINDOW_ENTRIES_COUNT; dir++)
	{
		gtk_entry_set_text(GTK_ENTRY(txtDirectory[dir]), dir_window_entries[dir].entry);
	}
}

/**
 * dir_window_save(): Save the Directory Configuration entries.
 */
static void dir_window_save(void)
{
	size_t len;
	
	for (unsigned int dir = 0; dir < DIR_WINDOW_ENTRIES_COUNT; dir++)
	{
		// Get the entry text.
		strncpy(dir_window_entries[dir].entry,
			gtk_entry_get_text(GTK_ENTRY(txtDirectory[dir])),
			GENS_PATH_MAX);
		
		// Make sure the entry is null-terminated.
		dir_window_entries[dir].entry[GENS_PATH_MAX - 1] = 0x00;
		
		// Make sure the end of the directory has a slash.
		// TODO: Do this in functions that use pathnames.
		len = strlen(dir_window_entries[dir].entry);
		if (len > 0 && dir_window_entries[dir].entry[len - 1] != GENS_DIR_SEPARATOR_CHR)
		{
			// String needs to be less than 1 minus the max path length
			// in order to be able to append the directory separator.
			if (len < (GENS_PATH_MAX - 1))
			{
				dir_window_entries[dir].entry[len] = GENS_DIR_SEPARATOR_CHR;
				dir_window_entries[dir].entry[len + 1] = 0x00;
			}
		}
	}
}


/**
 * dir_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean dir_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	dir_window_close();
	return FALSE;
}


/**
 * dir_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void dir_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(dialog);
	GENS_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			dir_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			dir_window_save();
			break;
		case GTK_RESPONSE_OK:
			dir_window_save();
			dir_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * dir_window_callback_btnChange_clicked(): A "Change" button was clicked.
 * @param button Button that was clicked.
 * @param user_data Button number.
 */
static void dir_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	
	int dir = GPOINTER_TO_INT(user_data);
	if (dir < 0 || dir >= DIR_WINDOW_ENTRIES_COUNT)
		return;
	
	char tmp[64];
	sprintf(tmp, "Select %s Directory", dir_window_entries[dir].title);
	
	// Request a new directory.
	string new_dir = GensUI::selectDir(tmp, gtk_entry_get_text(GTK_ENTRY(txtDirectory[dir])));
	
	// If "Cancel" was selected, don't do anything.
	if (new_dir.empty())
		return;
	
	// Make sure the end of the directory has a slash.
	if (new_dir.at(new_dir.length() - 1) != GENS_DIR_SEPARATOR_CHR)
		new_dir += GENS_DIR_SEPARATOR_CHR;
	
	// Set the new directory.
	gtk_entry_set_text(GTK_ENTRY(txtDirectory[dir]), new_dir.c_str());
}
