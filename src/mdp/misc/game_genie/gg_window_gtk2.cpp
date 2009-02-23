/***************************************************************************
 * Gens: [MDP] Game Genie. (Window Code) (GTK+)                            *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gg_window.hpp"
#include "gg_plugin.h"
#include "gg.hpp"
#include "gg_code.h"

// C includes.
#include <string.h>

// C++ includes.
#include <list>
using std::list;

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Window.
static GtkWidget *gg_window = NULL;

// Widgets.
static GtkWidget *txtEntry_Code;
static GtkWidget *txtEntry_Name;
static GtkWidget *lstCodes;

// List model for the code listing.
static GtkListStore *lmCodes = NULL;

// Widget creation functions.
static void gg_window_create_lstCodes(GtkWidget *container);

// Custom response IDs.
#define GG_RESPONSE_DELETE		1
#define GG_RESPONSE_DEACTIVATE_ALL	2

// Code load/save functions.
static void	gg_window_init(void);
static void	gg_window_save(void);

// Callbacks.
static gboolean	gg_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
//static void gg_window_callback_button(GtkButton *button, gpointer user_data);
static void	gg_window_callback_btnAddCode_clicked(GtkButton *button, gpointer user_data);
static gboolean	gg_window_callback_txtEntry_keypress(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void	gg_window_callback_lstCodes_toggled(GtkCellRendererToggle *cell_renderer,
						    gchar *path, gpointer user_data);

// Miscellaneous.
static int	gg_window_add_code_from_textboxes(void);
static int	gg_window_add_code(const gg_code_t *gg_code, const char* name);


/**
 * gg_window_show(): Show the Game Genie window.
 * @param parent Parent window.
 */
void gg_window_show(void *parent)
{
	if (gg_window)
	{
		// Game Genie window is already visible.
		// Set focus.
		gtk_widget_grab_focus(gg_window);
		return;
	}
	
	// Create the Game Genie window.
	gg_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(gg_window), 4);
	gtk_window_set_title(GTK_WINDOW(gg_window), "Game Genie");
	gtk_window_set_position(GTK_WINDOW(gg_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(gg_window), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(gg_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(gg_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)gg_window, "delete_event",
			 G_CALLBACK(gg_window_callback_close), NULL);
	g_signal_connect((gpointer)gg_window, "destroy_event",
			 G_CALLBACK(gg_window_callback_close), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(gg_window)->vbox;
	gtk_widget_show(vboxDialog);
	
	// Create the main frame.
	GtkWidget *fraMain = gtk_frame_new(NULL);
	gtk_widget_show(fraMain);
	gtk_box_pack_start(GTK_BOX(vboxDialog), fraMain, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraMain), GTK_SHADOW_NONE);
	
	// Strings.
	static const char* const strInfoTitle = "<b><i>Information about Game Genie / Patch codes</i></b>";
	static const char* const strInfo =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA-DDDD (address-data)";
	
	// Header label.
	GtkWidget *lblInfoTitle = gtk_label_new(strInfoTitle);
	gtk_label_set_use_markup(GTK_LABEL(lblInfoTitle), TRUE);
	gtk_widget_show(lblInfoTitle);
	gtk_frame_set_label_widget(GTK_FRAME(fraMain), lblInfoTitle);
	
	// Description label.
	GtkWidget *lblInfo = gtk_label_new(strInfo);
	gtk_widget_show(lblInfo);
	gtk_container_add(GTK_CONTAINER(fraMain), lblInfo);
	gtk_misc_set_alignment(GTK_MISC(lblInfo), 0.02f, 0.0f);
	
	// VBox for table layout.
	GtkWidget *vboxTable = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vboxTable);
	gtk_box_pack_start(GTK_BOX(vboxDialog), vboxTable, FALSE, TRUE, 0);
	
	// Table layout.
	GtkWidget *tblEntry = gtk_table_new(2, 3, FALSE);
	gtk_widget_show(tblEntry);
	gtk_table_set_col_spacings(GTK_TABLE(tblEntry), 10);
	gtk_box_pack_start(GTK_BOX(vboxTable), tblEntry, FALSE, TRUE, 0);
	
	// Code label and entry widgets.
	GtkWidget *lblEntry_Code = gtk_label_new("Code");
	gtk_misc_set_alignment(GTK_MISC(lblEntry_Code), 0.0f, 0.5f);
	gtk_widget_show(lblEntry_Code);
	gtk_table_attach(GTK_TABLE(tblEntry), lblEntry_Code, 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	txtEntry_Code = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(txtEntry_Code), 11);
	gtk_widget_show(txtEntry_Code);
	gtk_table_attach(GTK_TABLE(tblEntry), txtEntry_Code, 1, 2, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// Connect the "key-press-event" signal for the textbox.
	g_signal_connect((gpointer)txtEntry_Code, "key-press-event",
			  G_CALLBACK(gg_window_callback_txtEntry_keypress), NULL);
	
	// Name label and entry widgets.
	GtkWidget *lblEntry_Name = gtk_label_new("Name");
	gtk_misc_set_alignment(GTK_MISC(lblEntry_Name), 0.0f, 0.5f);
	gtk_widget_show(lblEntry_Name);
	gtk_table_attach(GTK_TABLE(tblEntry), lblEntry_Name, 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	txtEntry_Name = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(txtEntry_Name), 127);
	gtk_widget_show(txtEntry_Name);
	gtk_table_attach(GTK_TABLE(tblEntry), txtEntry_Name, 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// Connect the "key-press-event" signal for the textbox.
	g_signal_connect((gpointer)txtEntry_Name, "key-press-event",
			  G_CALLBACK(gg_window_callback_txtEntry_keypress), NULL);
	
	// "Add Code" button.
	GtkWidget *btnAddCode = gtk_button_new_with_mnemonic("Add C_ode");
	gtk_widget_show(btnAddCode);
	gtk_table_attach(GTK_TABLE(tblEntry), btnAddCode, 2, 3, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// Set the "Add Code" button's icon to "gtk-add".
	GtkWidget *btnAddCode_icon = gtk_image_new_from_stock("gtk-add", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(btnAddCode_icon);
	gtk_button_set_image(GTK_BUTTON(btnAddCode), btnAddCode_icon);
	
	// Connect the "clicked" signal for the "Add Code" button.
	g_signal_connect((gpointer)btnAddCode, "clicked",
			 G_CALLBACK(gg_window_callback_btnAddCode_clicked), NULL);
	
	// Set the focus chain for the entry boxes.
	GList *lFocusChain = NULL;
	lFocusChain = g_list_append(lFocusChain, txtEntry_Code);
	lFocusChain = g_list_append(lFocusChain, txtEntry_Name);
	lFocusChain = g_list_append(lFocusChain, btnAddCode);
	lFocusChain = g_list_first(lFocusChain);
	gtk_container_set_focus_chain(GTK_CONTAINER(tblEntry), lFocusChain);
	g_list_free(lFocusChain);
	
	// HBox for the code list.
	GtkWidget *hboxList = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxList);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxList, TRUE, TRUE, 0);
	
	// Scrolled Window for the code list.
	GtkWidget *scrlList = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlList), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlList),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrlList);
	gtk_box_pack_start(GTK_BOX(hboxList), scrlList, TRUE, TRUE, 0);
	
	// Create the treeview for the Game Genie codes.
	gg_window_create_lstCodes(scrlList);
	
	// Create the dialog buttons.
	
	// "Delete" button.
	gtk_dialog_add_button(GTK_DIALOG(gg_window),
			      GTK_STOCK_DELETE,
			      GG_RESPONSE_DELETE);
	
	// "Deactivate All" button.
	GtkWidget *btnDeactivateAll = gtk_dialog_add_button(
					GTK_DIALOG(gg_window),
					"Deac_tivate All",
					GG_RESPONSE_DEACTIVATE_ALL);
	
	// Create the icon for the "Deactivate All" button.
	GtkWidget *btnDeactivateAll_icon = gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(btnDeactivateAll_icon);
	gtk_button_set_image(GTK_BUTTON(btnDeactivateAll), btnDeactivateAll_icon);
	
	// Add the rest of the buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(gg_window),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
			       GTK_STOCK_SAVE, GTK_RESPONSE_OK,
			       NULL);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(gg_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_APPLY,
						GTK_RESPONSE_CANCEL,
						GG_RESPONSE_DEACTIVATE_ALL,
						GG_RESPONSE_DELETE,
						-1);
#endif
	
	// Initialize the Game Genie code treeview.
	gg_window_init();
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(gg_window), GTK_WINDOW(parent));
	
	// Show the window.
	gtk_widget_show_all(gg_window);
	
	// Register the window with MDP Host Services.
	gg_host_srv->window_register(&mdp, gg_window);
}


/**
 * gg_window_create_lstCodes(): Create the treeview for the Game Genie codes.
 * @param container Container for the treeview.
 */
static void gg_window_create_lstCodes(GtkWidget *container)
{
	// Create the list model.
	if (lmCodes)
	{
		// List model already exists. Clear it.
		gtk_list_store_clear(GTK_LIST_STORE(lmCodes));
	}
	else
	{
		lmCodes = gtk_list_store_new(4,
				G_TYPE_BOOLEAN,		// Enabled
				G_TYPE_STRING,		// Code (Hex)
				G_TYPE_STRING,		// Code (GG)
				G_TYPE_STRING);		// Name
	}
	
	// Treeview containing the Game Genie codes.
	lstCodes = gtk_tree_view_new_with_model(GTK_TREE_MODEL(lmCodes));
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstCodes), TRUE);
	gtk_widget_set_size_request(lstCodes, -1, 160);
	gtk_widget_show(lstCodes);
	gtk_container_add(GTK_CONTAINER(container), lstCodes);
	
	// Create the renderers and columns for the treeview.
	GtkCellRenderer  *rendEnabled = gtk_cell_renderer_toggle_new();
	GtkTreeViewColumn *colEnabled = gtk_tree_view_column_new_with_attributes("Enabled", rendEnabled, "active", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colEnabled);
	
	// Connect the toggle renderer to the callback.
	g_signal_connect((gpointer)rendEnabled, "toggled",
			 G_CALLBACK(gg_window_callback_lstCodes_toggled),
			 (gpointer)lmCodes);
	
	GtkCellRenderer  *rendCodeHex = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colCodeHex = gtk_tree_view_column_new_with_attributes("Code (Hex)", rendCodeHex, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colCodeHex);
	
	GtkCellRenderer  *rendCodeGG = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colCodeGG = gtk_tree_view_column_new_with_attributes("Code (GG)", rendCodeGG, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colCodeGG);
	
	GtkCellRenderer  *rendName = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colName = gtk_tree_view_column_new_with_attributes("Name", rendName, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colName);
}


/**
 * gg_window_close(): Close the Game Genie window.
 */
void gg_window_close(void)
{
	if (!gg_window)
		return;
	
	// Unregister the window from MDP Host Services.
	gg_host_srv->window_unregister(&mdp, gg_window);
	
	// Destroy the window.
	gtk_widget_destroy(gg_window);
	gg_window = NULL;
}


/**
 * gg_window_init(): Initialize the Game Genie code treeview.
 */
static void gg_window_init(void)
{
	// Add all loaded codes to the treeview.
	for (list<gg_code_t>::iterator iter = gg_code_list.begin();
	     iter != gg_code_list.end(); iter++)
	{
		gg_window_add_code(&(*iter), NULL);
	}
}


/**
 * gg_window_save(): Save the Game Genie codes.
 */
static void gg_window_save(void)
{
	// TODO: Undo any ROM-modifying codes.
	
	// Clear the code list.
	gg_code_list.clear();
	
	gg_code_t gg_code;
	gboolean enabled;
	gchar *code_hex, *name;
	
	// Add all codes in the listview to the code list.
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
	for (int i = 0; valid == true; i++)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(lmCodes), &iter, 0, &enabled, 1, &code_hex, 3, &name, -1);
		
		// Parse the code.
		// TODO: Store the already-parsed code in the list model?
		// TODO: Error handling.
		gg_code_parse(code_hex, &gg_code, CPU_M68K);
		
		// Copy the name of the code into gg_code_t.
		strncpy(gg_code.name, name, sizeof(gg_code.name));
		gg_code.name[sizeof(gg_code.name)-1] = 0x00;
		gg_code.enabled = enabled;
		
		// Add the code to the list of codes.
		gg_code_list.push_back(gg_code);
		
		// Get the next list element.
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
	}	
	
	// TODO: Apply codes if a game is running.
}


/**
 * gg_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 */
static gboolean gg_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	gg_window_close();
	return FALSE;
}


/**
 * gg_window_callback_btnAddCode_clicked(): "Add Code" button was clicked.
 * @param button Button widget.
 * @param user_data User data.
 */
static void gg_window_callback_btnAddCode_clicked(GtkButton *button, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(button);
	MDP_UNUSED_PARAMETER(user_data);
	
	gg_window_add_code_from_textboxes();
}


/**
 * gg_window_callback_txtEntry_keypress(): A key was pressed in a txtEntry textbox.
 * @param button Button widget.
 * @param user_data User data.
 */
static gboolean	gg_window_callback_txtEntry_keypress(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(user_data);
	
	if (event->keyval == GDK_Return ||
	    event->keyval == GDK_KP_Enter)
	{
		// "Enter" was pressed. Add the code.
		gg_window_add_code_from_textboxes();
		
		// Stop processing the key.
		return TRUE;
	}
	
	// Continue processing the key.
	return FALSE;
}


/**
 * gg_window_add_code_from_textboxes(): Add a code to the treeview from the textboxes.
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code_from_textboxes(void)
{
	// Decode the code.
	// TODO: Add support for more CPUs, datasizes, etc.
	gg_code_t gg_code;
	gg_code.name[0] = 0x00;
	
	if (gg_code_parse(gtk_entry_get_text(GTK_ENTRY(txtEntry_Code)), &gg_code, CPU_M68K))
	{
		// Error parsing the code.
		// TODO: Show an error message.
		return 1;
	}
	
	int rval = gg_window_add_code(&gg_code, gtk_entry_get_text(GTK_ENTRY(txtEntry_Name)));
	if (rval == 0)
	{
		// Code added successfully.
		// Clear the textboxes and set focus to the "Code" textbox.
		gtk_entry_set_text(GTK_ENTRY(txtEntry_Code), "");
		gtk_entry_set_text(GTK_ENTRY(txtEntry_Name), "");
		gtk_widget_grab_focus(txtEntry_Code);
	}
	
	return rval;
}
	
/**
 * gg_window_add_code(): Add a code to the treeview.
 * @param gg_code Pointer to gg_code_t containing the code to add.
 * @param name Name of the code. (If NULL, the name in gg_code is used.)
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code(const gg_code_t *gg_code, const char* name)
{
	if (!gg_code)
		return 1;
	
	// Create the hex version of the code.
	char s_code_hex[32];
	
	switch (gg_code->cpu)
	{
		case CPU_M68K:
		case CPU_S68K:
		{
			// 68000: 24-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					sprintf(s_code_hex, "%06X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					sprintf(s_code_hex, "%06X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					sprintf(s_code_hex, "%06X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					// TODO: Show an error message.
					return 1;
			}
			break;
		}
		case CPU_Z80:
		{
			// Z80: 16-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					sprintf(s_code_hex, "%04X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					sprintf(s_code_hex, "%04X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					sprintf(s_code_hex, "%04X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					// TODO: Show an error message.
					return 1;
			}
			break;
		}
		case CPU_MSH2:
		case CPU_SSH2:
		{
			// SH2: 32-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					sprintf(s_code_hex, "%08X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					sprintf(s_code_hex, "%08X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					sprintf(s_code_hex, "%08X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					// TODO: Show an error message.
					return 1;
			}
			break;
		}
		case CPU_INVALID:
		default:
			// Invalid code.
			// TODO: Show an error message.
			return 1;
	}
	
	// Determine what should be used for the Game Genie code.
	const char* s_code_gg;
	if (!gg_code->game_genie[0])
	{
		// The code can't be converted to Game Genie.
		s_code_gg = "N/A";
	}
	else
	{
		// The code can be converted to Game Genie.
		s_code_gg = gg_code->game_genie;
	}
	
	// If no name is given, use the name in the gg_code.
	if (!name)
		name = &gg_code->name[0];
	
	// Code is decoded. Add it to the treeview.
	GtkTreeIter iter;
	gtk_list_store_append(lmCodes, &iter);
	gtk_list_store_set(GTK_LIST_STORE(lmCodes), &iter,
			   0, FALSE,		// Disable the code by default.
			   1, s_code_hex,	// Hex code.
			   2, s_code_gg,	// Game Genie code. (if applicable)
			   3, name, -1);
	
	// Code added successfully.
	return 0;
}


/**
 * gg_window_callback_lstCodes_toggled(): Code was toggled.
 * @param cell_renderer
 * @param path
 * @param user_data Pointer to the list model.
 */
static void gg_window_callback_lstCodes_toggled(GtkCellRendererToggle *cell_renderer,
						gchar *path, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(cell_renderer);
	
	// Toggle the state of this item.
	GtkTreeIter iter;
	gboolean cur_state;
	
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(user_data), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(user_data), &iter, 0, &cur_state, -1);
	gtk_list_store_set(GTK_LIST_STORE(user_data), &iter, 0, !cur_state, -1);
}
