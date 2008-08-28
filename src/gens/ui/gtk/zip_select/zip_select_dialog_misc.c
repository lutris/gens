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
#include "zip_select_dialog_misc.h"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "gens.h"
#include "g_main.h"

#include "compress/compress.h"


/**
 * Get_Zip_Select(): Opens the Zip File Selection Dialog.
 */
struct fileInfo_t* Open_Zip_Select_Dialog(struct fileInfo_t* fip)
{
	GtkWidget *Zip, *treeview;
	GtkCellRenderer *text_renderer;
	GtkTreeViewColumn *col_text;
	GtkTreeIter iter;
	int i;
	
	// Stores the entries in the TreeView.
	GtkListStore *listmodel_zip = NULL;
	
	Zip = create_zip_select_dialog();
	if (!Zip)
	{
		// Either an error occurred while creating the Zip Code dialog,
		// or the Zip Code dialog is already created.
		return NULL;
	}
	
	// Populate the TreeView.
	/*
	treeview = lookup_widget(Zip, "treeview_country_list");
	
	// Check if the listmodel_country is already created.
	// If it is, clear it; if not, create a new one.
	if (listmodel_country)
		gtk_list_store_clear(listmodel_country);
	else
		listmodel_country = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);	
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(listmodel_country));
	
	// Create the renderer and the columns.
	text_renderer = gtk_cell_renderer_text_new();
	col_text = gtk_tree_view_column_new_with_attributes("Zip", text_renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_text);
	
	// Add the country codes to the treeview in the appropriate order.
	for (i = 0; i < 3; i++)
	{
		gtk_list_store_append(listmodel_country, &iter);
		gtk_list_store_set(GTK_LIST_STORE(listmodel_country), &iter,
						  0, Zip_Code_String[Zip_Order[i]],
						  1, Zip_Order[i], -1);
	}
	*/
	
	// Run the dialog.
	printf("RESPONSE: %d\n", gtk_dialog_run(GTK_DIALOG(Zip)));
	
	// Delete the dialog.
	gtk_widget_destroy(Zip);
}
