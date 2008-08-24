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
