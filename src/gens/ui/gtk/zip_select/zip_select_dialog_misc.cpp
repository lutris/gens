/***************************************************************************
 * Gens: (GTK+) Zip File Selection Dialog - Miscellaneous Functions.       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "zip_select_dialog.h"
#include "zip_select_dialog_misc.hpp"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "emulator/gens.h"
#include "emulator/g_main.h"

#include <string>
#include <list>
using std::string;
using std::list;

/**
 * Get_Zip_Select(): Opens the Zip File Selection Dialog.
 */
CompressedFile* Open_Zip_Select_Dialog(list<CompressedFile>* lst)
{
	GtkWidget *Zip, *treeview;
	GtkCellRenderer *text_renderer;
	GtkTreeViewColumn *col_text;
	GtkTreeIter iter;
	gint dialogResponse;
	GtkTreeSelection *selection;
	gboolean valid;
	
	list<CompressedFile>::iterator lstIter;
	CompressedFile *selFile;
	
	if (!lst)
	{
		// NULL list pointer passed. Don't do anything.
		return NULL;
	}
	
	// Stores the entries in the TreeView.
	GtkListStore *listmodel_zip = NULL;
	
	Zip = create_zip_select_dialog();
	if (!Zip)
	{
		// Either an error occurred while creating the Zip Code dialog,
		// or the Zip Code dialog is already created.
		return NULL;
	}
	
	// Get the treeview widget.
	treeview = lookup_widget(Zip, "treeview_zip_list");
	
	// Create a list model.
	listmodel_zip = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(listmodel_zip));
	
	// Create the renderer and the columns.
	text_renderer = gtk_cell_renderer_text_new();
	col_text = gtk_tree_view_column_new_with_attributes("Zip", text_renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_text);
	
	// Add all files from the CompressedFile list.
	for (lstIter = lst->begin(); lstIter != lst->end(); lstIter++)
	{
		gtk_list_store_append(listmodel_zip, &iter);
		gtk_list_store_set(GTK_LIST_STORE(listmodel_zip), &iter,
				   0, (*lstIter).filename.c_str(), 1, &(*lstIter), -1);
	}
	
	// Run the dialog.
	dialogResponse = gtk_dialog_run(GTK_DIALOG(Zip));
	if (dialogResponse != GTK_RESPONSE_OK)
	{
		// No file was selected.
		return NULL;
	}
	
	// Get the selected file.
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(listmodel_zip), &iter);
	while (valid)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			// Found the selected file.
			gtk_tree_model_get(GTK_TREE_MODEL(listmodel_zip), &iter, 1, &selFile, -1);
			break;
		}
		else
		{
			// Didn't find the selected file yet.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(listmodel_zip), &iter);
		}
	}
	
	// Delete the dialog.
	gtk_widget_destroy(Zip);
	
	// Return the selected CompressedFile*.
	return selFile;
}
