/**
 * GENS: (GTK+) Game Genie Window Callbacks.
 */
 
 
 #include <string.h>
 #include "game_genie_window_callbacks.h"
 #include "game_genie_window_misc.h"
 
 #include "gtk-misc.h"
 
 
/**
 * STUB: Indicates that this function is a stub.
 */
#define STUB fprintf(stderr, "TODO: STUB: %s()\n", __func__);


/**
 * Add Code
 */
void on_button_gg_addCode_clicked(GtkButton *button, gpointer user_data)
{
	GtkWidget *entry_gg_code, *entry_gg_name;
	gchar *code, *name;
	int length;
	
	// Look up the entry widgets.
	entry_gg_code = lookup_widget(GTK_WIDGET(button), "entry_gg_code");
	entry_gg_name = lookup_widget(GTK_WIDGET(button), "entry_gg_name");
	
	// Get the text.
	code = strdup(gtk_entry_get_text(GTK_ENTRY(entry_gg_code)));
	name = strdup(gtk_entry_get_text(GTK_ENTRY(entry_gg_name)));
	
	// Get the length of the code.
	// Game Genie codes are 9 characters long. ("XXXX-YYYY")
	// Patch codes are 11 characters long. ("AAAAAA-DDDD")
	length = strlen(code);
	if (length == 9 || length == 11)
		GG_AddCode(NULL, name, code, 0);
	
	g_free(code);
	g_free(name);
}
 

/**
 * Delete Code
 */
void on_button_gg_delCode_clicked(GtkButton *button, gpointer user_data)
{
	STUB;
}


/**
 * Deactivate All Codes
 */
void on_button_gg_deactAllCodes_clicked(GtkButton *button, gpointer user_data)
{
	STUB;
}


/**
 * Cancel
 */
void on_button_gg_cancel_clicked(GtkButton *button, gpointer user_data)
{
	STUB;
}


/**
 * OK
 */
void on_button_gg_OK_clicked(GtkButton *button, gpointer user_data)
{
	STUB;
}


/**
 * Code was toggled.
 */
void on_treeview_gg_list_item_toggled(GtkCellRendererToggle *cell_renderer,
				      gchar *path, gpointer user_data)
{
	// Toggle the state of this item.
	GtkTreeIter iter;
	gboolean curState;
	
	// Get the iterator pointing to this list item.
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(user_data), &iter, path);
	
	// Get the current value.
	gtk_tree_model_get(GTK_TREE_MODEL(user_data), &iter, 0, &curState, -1);
	
	// Set the inverted value.
	gtk_list_store_set(GTK_LIST_STORE(user_data), &iter, 0, !curState, -1);
}
