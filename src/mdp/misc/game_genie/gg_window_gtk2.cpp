/***************************************************************************
 * MDP: Game Genie. (Window Code) (GTK+)                                   *
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

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// C includes.
#include <string.h>
#include <stdlib.h>

// C++ includes.
#include <list>
using std::list;

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

#include "gg_window.hpp"
#include "gg_plugin.h"
#include "gg.hpp"
#include "gg_code.h"
#include "gg_engine.hpp"

// XPM icon.
#include "gg_32x32.xpm"

// gtk_button_set_image() was added in GTK+ 2.6.
#if (GTK_MAJOR_VERSION < 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 6))
#define gtk_button_set_image(button, image)
#endif

// Window.
static GtkWidget *gg_window = NULL;

// Widgets.
static GtkWidget *txtCode;
static GtkWidget *txtName;
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
static void	gg_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	gg_window_callback_btnAddCode_clicked(GtkButton *button, gpointer user_data);
static gboolean	gg_window_callback_txtEntry_keypress(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void	gg_window_callback_lstCodes_toggled(GtkCellRendererToggle *cell_renderer,
						    gchar *path, gpointer user_data);
static void	gg_window_callback_delete(void);
static void	gg_window_callback_deactivate_all(void);

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
	
	// Add the window icon.
	GList *icon_list = NULL;
	GdkPixbuf *icon_pixbuf_32;
	
	// Load the 32x32 icon.
	icon_pixbuf_32 = gdk_pixbuf_new_from_xpm_data(gg_icon_xpm_32x32);
	icon_list = g_list_append(icon_list, icon_pixbuf_32);
	
	// Set the icon list.
	gtk_window_set_icon_list(GTK_WINDOW(gg_window), icon_list);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)gg_window, "delete_event",
			 G_CALLBACK(gg_window_callback_close), NULL);
	g_signal_connect((gpointer)gg_window, "destroy_event",
			 G_CALLBACK(gg_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(gg_window), "response",
			 G_CALLBACK(gg_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(gg_window));
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
			"Syntax for Patch codes: AAAAAA:DDDD (address:data)";
	
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
	
	// "Code" label.
	GtkWidget *lblCode = gtk_label_new("Code");
	gtk_misc_set_alignment(GTK_MISC(lblCode), 0.0f, 0.5f);
	gtk_widget_show(lblCode);
	gtk_table_attach(GTK_TABLE(tblEntry), lblCode, 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// "Code" textbox.
	txtCode = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(txtCode), 17);
	gtk_widget_show(txtCode);
	gtk_table_attach(GTK_TABLE(tblEntry), txtCode, 1, 2, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// Connect the "key-press-event" signal for the textbox.
	g_signal_connect((gpointer)txtCode, "key-press-event",
			  G_CALLBACK(gg_window_callback_txtEntry_keypress), NULL);
	
	// "Name" label.
	GtkWidget *lblName = gtk_label_new("Name");
	gtk_misc_set_alignment(GTK_MISC(lblName), 0.0f, 0.5f);
	gtk_widget_show(lblName);
	gtk_table_attach(GTK_TABLE(tblEntry), lblName, 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// "Name" textbox.
	txtName = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(txtName), 127);
	gtk_widget_show(txtName);
	gtk_table_attach(GTK_TABLE(tblEntry), txtName, 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// Connect the "key-press-event" signal for the textbox.
	g_signal_connect((gpointer)txtName, "key-press-event",
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
	lFocusChain = g_list_append(lFocusChain, txtCode);
	lFocusChain = g_list_append(lFocusChain, txtName);
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
	
	// "Deactivate All" button.
	GtkWidget *btnDeactivateAll = gtk_dialog_add_button(
					GTK_DIALOG(gg_window),
					"Deac_tivate All",
					GG_RESPONSE_DEACTIVATE_ALL);
	
	// "Delete" button.
	gtk_dialog_add_button(GTK_DIALOG(gg_window),
			      GTK_STOCK_DELETE,
			      GG_RESPONSE_DELETE);
	
	// Create the icon for the "Deactivate All" button.
	GtkWidget *btnDeactivateAll_icon = gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(btnDeactivateAll_icon);
	gtk_button_set_image(GTK_BUTTON(btnDeactivateAll), btnDeactivateAll_icon);
	
	// Add the rest of the buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(gg_window),
			       GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			       GTK_STOCK_SAVE, GTK_RESPONSE_OK,
			       NULL);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(gg_window),
						GG_RESPONSE_DEACTIVATE_ALL,
						GG_RESPONSE_DELETE,
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						GTK_RESPONSE_APPLY,
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
		lmCodes = gtk_list_store_new(6,
				G_TYPE_BOOLEAN,		// Enabled
				G_TYPE_STRING,		// Code (Hex)
				G_TYPE_STRING,		// Code (GG)
				G_TYPE_STRING,		// CPU
				G_TYPE_STRING,		// Name
				G_TYPE_POINTER);	// gg_code_t*
	}
	
	// Treeview containing the Game Genie codes.
	lstCodes = gtk_tree_view_new_with_model(GTK_TREE_MODEL(lmCodes));
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstCodes), true);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(lstCodes)),
				    GTK_SELECTION_MULTIPLE);
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
	
	// Code (Hex)
	GtkCellRenderer  *rendCodeHex = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colCodeHex = gtk_tree_view_column_new_with_attributes("Code (Hex)", rendCodeHex, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colCodeHex);
	
	// Code (GG)
	GtkCellRenderer  *rendCodeGG = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colCodeGG = gtk_tree_view_column_new_with_attributes("Code (GG)", rendCodeGG, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colCodeGG);
	
	// CPU
	GtkCellRenderer  *rendCPU = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colCPU = gtk_tree_view_column_new_with_attributes("CPU", rendCPU, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colCPU);
	
	// Name
	GtkCellRenderer  *rendName = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colName = gtk_tree_view_column_new_with_attributes("Name", rendName, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCodes), colName);
	
	// Set focus to the "Code" textbox.
	gtk_widget_grab_focus(txtCode);
}


/**
 * gg_window_close(): Close the Game Genie window.
 */
void gg_window_close(void)
{
	if (!gg_window)
		return;
	
	// Delete any gg_code pointers that may be stored in the list store.
	if (lmCodes)
	{
		GtkTreeIter iter;
		gg_code_t *stored_code;
		
		gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
		for (int i = 0; valid == true; i++)
		{
			gtk_tree_model_get(GTK_TREE_MODEL(lmCodes), &iter, 5, &stored_code, -1);
			
			// Delete the code.
			free(stored_code);
			
			// Get the next list element.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
		}
		
		// Clear the list store.
		gtk_list_store_clear(GTK_LIST_STORE(lmCodes));
	}
	
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
	// Unapply ROM codes.
	gg_engine_unapply_rom_codes();
	
	// Clear the code list.
	gg_code_list.clear();
	
	gg_code_t gg_code;
	
	// Values retrieved from the list model.
	gg_code_t *stored_code;
	gboolean enabled;
	
	// Add all codes in the listview to the code list.
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
	for (int i = 0; valid == true; i++)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(lmCodes), &iter, 0, &enabled, 5, &stored_code, -1);
		
		// Copy the code.
		memcpy(&gg_code, stored_code, sizeof(gg_code));
		
		// Copy the "enabled" value.
		gg_code.enabled = enabled;
		
		// Add the code to the list of codes.
		gg_code_list.push_back(gg_code);
		
		// Get the next list element.
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
	}
	
	// Apply ROM codes.
	gg_engine_apply_rom_codes();
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
 * gg_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void gg_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(dialog);
	MDP_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			gg_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			gg_window_save();
			break;
		case GTK_RESPONSE_OK:
			gg_window_save();
			gg_window_close();
			break;
		case GG_RESPONSE_DELETE:
			gg_window_callback_delete();
			break;
		case GG_RESPONSE_DEACTIVATE_ALL:
			gg_window_callback_deactivate_all();
			break;
			
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
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
	const gchar* code_txt = gtk_entry_get_text(GTK_ENTRY(txtCode));
	if (!code_txt || code_txt[0] == 0x00)
		return 1;
	
	// Decode the code.
	gg_code_t gg_code;
	gg_code.name[0] = 0x00;
	gg_code.enabled = 0;
	
	GG_CODE_ERR gcp_rval = gg_code_parse(code_txt, &gg_code, CPU_M68K);
	
	if (gcp_rval != GGCE_OK)
	{
		// Error parsing the code.
		char err_msg_full[1024];
		const char *err_msg;
		
		switch (gcp_rval)
		{
			case GGCE_UNRECOGNIZED:
				err_msg = "The code could not be parsed correctly.";
				break;
			case GGCE_ADDRESS_RANGE:
				// TODO: Show range depending on the selected CPU.
				err_msg = "The address for this code is out of range for the specified CPU.\n"
					  "Valid range for MC68000 CPUs: 0x000000 - 0xFFFFFF";
				break;
			case GGCE_ADDRESS_ALIGNMENT:
				// TODO: Show range and alignment info based on CPU and datasize.
				err_msg = "The address is not aligned properly for the specified data.\n"
					  "For MC68000, 16-bit data must be stored at even addresses.";
				break;
			case GGCE_DATA_TOO_LARGE:
				err_msg = "The data value is too large. Usually, this means that you\n"
					  "entered too many characters.";
				break;
			default:
				// Other error.
				err_msg = "Unknown error code.";
				break;
		}
		
		szprintf(err_msg_full, sizeof(err_msg_full),
			 "The specified code, \"%s\", could not be added due to an error:\n\n%s",
			 code_txt, err_msg);
		
		// Show an error message.
		GtkWidget *msgbox = gtk_message_dialog_new(
					GTK_WINDOW(gg_window),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s", err_msg_full);
		gtk_window_set_title(GTK_WINDOW(msgbox), "Game Genie: Code Error");
		gtk_dialog_run(GTK_DIALOG(msgbox));
		gtk_widget_destroy(msgbox);
		
		// Set focus to the "Code" textbox.
		gtk_widget_grab_focus(txtCode);
		
		return gcp_rval;
	}
	
	int ggw_ac_rval = gg_window_add_code(&gg_code, gtk_entry_get_text(GTK_ENTRY(txtName)));
	if (ggw_ac_rval == 0)
	{
		// Code added successfully.
		// Clear the textboxes and set focus to the "Code" textbox.
		gtk_entry_set_text(GTK_ENTRY(txtCode), "");
		gtk_entry_set_text(GTK_ENTRY(txtName), "");
		gtk_widget_grab_focus(txtCode);
	}
	
	return ggw_ac_rval;
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
	if (gg_code_format_hex(gg_code, s_code_hex, sizeof(s_code_hex)))
	{
		// TODO: Show an error message.
		return 1;
	}
	
	// CPU string.
	static const char* const s_cpu_list[8] = {NULL, "M68K", "S68K", "Z80", "MSH2", "SSH2", NULL, NULL};
	const char* s_cpu = s_cpu_list[(int)(gg_code->cpu) & 0x07];
	if (!s_cpu)
		return 1;
	
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
	
	// Store the gg_code_t in the list store.
	gg_code_t *lst_code = (gg_code_t*)(malloc(sizeof(gg_code_t)));
	memcpy(lst_code, gg_code, sizeof(gg_code_t));
	
	// If no name is given, use the name in the gg_code.
	if (!name)
		name = &gg_code->name[0];
	else
	{
		// Copy the specified name into lst_code.
		memcpy(lst_code->name, name, sizeof(lst_code->name));
		lst_code->name[sizeof(lst_code->name)-1] = 0x00;
	}
	
	// Code is decoded. Add it to the treeview.
	GtkTreeIter iter;
	gtk_list_store_append(lmCodes, &iter);
	gtk_list_store_set(GTK_LIST_STORE(lmCodes), &iter,
			   0, gg_code->enabled,	// Enabled.
			   1, s_code_hex,	// Hex code.
			   2, s_code_gg,	// Game Genie code. (if applicable)
			   3, s_cpu,		// CPU.
			   4, name,		// Code name.
			   5, lst_code, -1);	// gg_code_t
	
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


/**
 * gg_window_callback_delete(): Delete the selected code(s).
 */
static void gg_window_callback_delete(void)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gboolean need_check, row_erased, valid;
	
	// Get the current selection.
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstCodes));
	
	// Delete all selected codes.
	need_check = true;
	while (need_check)
	{
		row_erased = false;
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
		while (valid && !row_erased)
		{
			if (gtk_tree_selection_iter_is_selected(selection, &iter))
			{
				// Delete the gg_code_t first.
				gg_code_t *stored_code;
				gtk_tree_model_get(GTK_TREE_MODEL(lmCodes), &iter, 4, &stored_code, -1);
				free(stored_code);
				
				// Delete the row.
				gtk_list_store_remove(lmCodes, &iter);
				row_erased = true;
			}
			else
			{
				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
			}
		}
		if (!valid && !row_erased)
			need_check = false;
	}
}


/**
 * gg_window_callback_deactivate_all(): Deactivate all codes.
 */
static void gg_window_callback_deactivate_all(void)
{
	GtkTreeIter iter;
	gboolean valid;
	
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
	while (valid)
	{
		gtk_list_store_set(GTK_LIST_STORE(lmCodes), &iter, 0, 0, -1);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
	}
}
