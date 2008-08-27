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

#include "gens.h"

// Stores the entries in the TreeView.
GtkListStore *listmodel_country = NULL;


/**
 * Open_Country_Code(): Opens the Country Code window.
 */
void Open_Country_Code(void)
{
	GtkWidget *Country, *treeview;
	GtkCellRenderer *text_renderer;
	GtkTreeViewColumn *col_text;
	GtkTreeIter iter;
	int i;
	
	Country = create_country_code_window();
	if (!Country)
	{
		// Either an error occurred while creating the Country Code window,
		// or the Country Code window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(Country), GTK_WINDOW(gens_window));
	
	// Set up the country order treeview.
	// Elements in Country_Order[3] can have one of three values:
	// - 0 [USA]
	// - 1 [JAP]
	// - 2 [EUR]
	
	// Make sure the country code order is valid.
	Check_Country_Order();
	
	// Populate the TreeView.
	treeview = lookup_widget(Country, "treeview_country");
	
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
	col_text = gtk_tree_view_column_new_with_attributes("Country", text_renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col_text);
	
	// Add the country codes to the treeview in the appropriate order.
	for (i = 0; i < 3; i++)
	{
		gtk_list_store_append(listmodel_country, &iter);
		gtk_list_store_set(GTK_LIST_STORE(listmodel_country), &iter, 0, Country_Code_String[i], 1, i, -1);
	}
	
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
