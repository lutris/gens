/***************************************************************************
 * Gens: (GTK+) Zip File Selection Dialog.                                 *
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

#include "zipsel_dialog.h"
#include "gens/gens_window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "libgsft/gsft_unused.h"

// Gens GTK+ miscellaneous functions
#include "gtk-misc.h"
#include "ui/gtk/gtk-compat.h"


typedef struct _zipsel_dialog_t
{
	GtkWidget *dialog;
	GtkWidget *btnOK;
	GtkWidget *btnCancel;
	GtkWidget *lstFiles;
} zipsel_dialog_t;

static zipsel_dialog_t	zipsel_dialog_create(void);
static gboolean		zipsel_dialog_callback_lstFiles_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

// TODO: Improve this dialog.


/**
 * zipsel_dialog_create(): Create the Zip File Selection Dialog.
 * @return Zip File Selection Dialog.
 */
static zipsel_dialog_t zipsel_dialog_create(void)
{
	zipsel_dialog_t zs;
	
	// Create the Zip File Selection window.
	zs.dialog = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(zs.dialog), 5);
	gtk_window_set_title(GTK_WINDOW(zs.dialog), "Archive File Selection");
	gtk_window_set_position(GTK_WINDOW(zs.dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(zs.dialog), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(zs.dialog), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(zs.dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(zs.dialog), FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(zs.dialog), GTK_WINDOW(gens_window));
	
	// Make the window a decent size.
	gtk_widget_set_size_request(zs.dialog, 480, 280);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(zs.dialog));
	gtk_box_set_spacing(GTK_BOX(vboxDialog), 4);
	gtk_widget_show(vboxDialog);
	
	// Add a label to the dialog.
	GtkWidget *lblZip = gtk_label_new("This archive contains multiple files.\nSelect which file you want to load.");
	gtk_label_set_justify(GTK_LABEL(lblZip), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblZip);
	gtk_box_pack_start(GTK_BOX(vboxDialog), lblZip, FALSE, FALSE, 0);
	
	// Scrolled Window for the file list
	GtkWidget *scrlFileList = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlFileList),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrlFileList);
	gtk_box_pack_start(GTK_BOX(vboxDialog), scrlFileList, TRUE, TRUE, 0);
	
	// Tree view containing the files in the archive.
	zs.lstFiles = gtk_tree_view_new();
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(zs.lstFiles), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(zs.lstFiles), FALSE);
	gtk_widget_show(zs.lstFiles);
	gtk_container_add(GTK_CONTAINER(scrlFileList), zs.lstFiles);
	g_signal_connect((gpointer)zs.lstFiles, "button_press_event",
			  G_CALLBACK(zipsel_dialog_callback_lstFiles_button_press), zs.dialog);
	
	// Create the dialog buttons.
	zs.btnCancel = gtk_dialog_add_button(GTK_DIALOG(zs.dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	zs.btnOK     = gtk_dialog_add_button(GTK_DIALOG(zs.dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(zs.dialog),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						-1);
#endif
	
	// Return the dialog struct.
	return zs;
}


static gboolean zipsel_dialog_callback_lstFiles_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	
	if (GTK_IS_DIALOG(user_data) && event->type == GDK_2BUTTON_PRESS)
	{
		// Item was double-clicked in the treeview.
		// Select the current item.
		gtk_dialog_response(GTK_DIALOG(user_data), GTK_RESPONSE_OK);
		return TRUE;
	}
	return FALSE;
}


/**
 * zipsel_dialog_get_file(): Get a file using the Zip Select dialog.
 */
mdp_z_entry_t* zipsel_dialog_get_file(mdp_z_entry_t *z_list)
{
	if (!z_list)
	{
		// NULL list pointer passed. Don't do anything.
		return NULL;
	}
	
	// Create a new dialog.
	zipsel_dialog_t zs = zipsel_dialog_create();
	
	gint dialogResponse;
	GtkTreeSelection *selection;
	gboolean valid;
	GtkTreeIter iter;
	
	// Stores the entries in the TreeView.
	GtkListStore *lstdataFiles = NULL;
	
	// Create a list model.
	lstdataFiles = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(zs.lstFiles), GTK_TREE_MODEL(lstdataFiles));
	
	// Create the renderer and the columns.
	GtkCellRenderer *rendText = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colText = gtk_tree_view_column_new_with_attributes("Zip", rendText, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(zs.lstFiles), colText);
	
	// Add all files from the CompressedFile list.
	mdp_z_entry_t *z_entry_cur = z_list;
	while (z_entry_cur)
	{
		gtk_list_store_append(lstdataFiles, &iter);
		gtk_list_store_set(GTK_LIST_STORE(lstdataFiles), &iter,
				   0, z_entry_cur->filename, 1, z_entry_cur, -1);
		z_entry_cur = z_entry_cur->next;
	}
	
	// Select the first item by default.
	GtkTreePath *path;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(zs.lstFiles));
	path = gtk_tree_path_new_from_string("0");
	gtk_tree_selection_select_path(selection, path);
	gtk_tree_path_free(path);
	
	// Run the dialog.
	dialogResponse = gtk_dialog_run(GTK_DIALOG(zs.dialog));
	if (dialogResponse != GTK_RESPONSE_OK)
	{
		// No file was selected.
		gtk_widget_destroy(zs.dialog);
		return NULL;
	}
	
	// Change the mouse pointer to "busy".
	GdkCursor *cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(gtk_widget_get_window(zs.dialog), cursor);
	gdk_cursor_destroy(cursor);
	
	while (gtk_events_pending())
		gtk_main_iteration_do(FALSE);
	
	// Get the selected file.
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(zs.lstFiles));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lstdataFiles), &iter);
	while (valid)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			// Found the selected file.
			gtk_tree_model_get(GTK_TREE_MODEL(lstdataFiles), &iter, 1, &z_entry_cur, -1);
			break;
		}
		else
		{
			// Didn't find the selected file yet.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lstdataFiles), &iter);
		}
	}
	
	// Destroy the dialog.
	gtk_widget_destroy(zs.dialog);
	
	// Return the selected mdp_z_entry_t*.
	return z_entry_cur;
}
