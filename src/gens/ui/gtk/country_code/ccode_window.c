/***************************************************************************
 * Gens: (GTK+) Country Code Window.                                       *
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

#include "ccode_window.h"
#include "gens/gens_window.h"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// GTK+ includes.
#include <gtk/gtk.h>

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"


// Window.
GtkWidget *ccode_window = NULL;

// Widgets.
static GtkWidget	*lstCountryCodes;
static GtkListStore	*lmCountryCodes = NULL;
static GtkWidget	*btnCancel, *btnApply, *btnSave;

// Widget creation functions.
static void	ccode_window_create_lstCountryCodes(GtkWidget *container);
static void	ccode_window_create_up_down_buttons(GtkWidget *container);

// Configuration load/save functions.
static void	ccode_window_init(void);
static void	ccode_window_save(void);

// Callbacks.
static gboolean	ccode_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	ccode_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	ccode_window_callback_btnUp_clicked(GtkButton *button, gpointer user_data);
static void	ccode_window_callback_btnDown_clicked(GtkButton *button, gpointer user_data);
static gboolean	ccode_window_callback_lstCountryCodes_drag_drop(GtkWidget	*widget,
								GdkDragContext	*drag_context,
								gint		x,
								gint		y,
								guint		time_,
								gpointer	user_data);


/**
 * ccode_window_show(): Show the Country Code window.
 */
void ccode_window_show(void)
{
	if (ccode_window)
	{
		// Country Code window is already visible. Set focus.
		gtk_widget_grab_focus(ccode_window);
		return;
	}
	
	// Create the window.
	ccode_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(ccode_window), 4);
	gtk_window_set_title(GTK_WINDOW(ccode_window), "Country Code Order");
	gtk_window_set_position(GTK_WINDOW(ccode_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(ccode_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(ccode_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(ccode_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)ccode_window, "delete_event",
			 G_CALLBACK(ccode_window_callback_close), NULL);
	g_signal_connect((gpointer)ccode_window, "destroy_event",
			 G_CALLBACK(ccode_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(ccode_window), "response",
			 G_CALLBACK(ccode_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(ccode_window));
	gtk_widget_show(vboxDialog);
	
	// Add a frame for country code selection.
	GtkWidget *fraCountry = gtk_frame_new("<b><i>Country Code Order</i></b>");
	gtk_frame_set_shadow_type(GTK_FRAME(fraCountry), GTK_SHADOW_ETCHED_IN);
	gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraCountry))), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(fraCountry), 4);
	gtk_widget_show(fraCountry);
	gtk_box_pack_start(GTK_BOX(vboxDialog), fraCountry, TRUE, TRUE, 0);
	
	// HBox for the Country Codes treeview and up/down buttons.
	GtkWidget *hboxList = gtk_hbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(hboxList), 8);
	gtk_widget_show(hboxList);
	gtk_container_add(GTK_CONTAINER(fraCountry), hboxList);
	
	// Create the Country Code treeview.
	ccode_window_create_lstCountryCodes(hboxList);
	
	// Create the Up/Down buttons.
	ccode_window_create_up_down_buttons(hboxList);
	
	// Create the dialog buttons.
	btnApply  = gtk_dialog_add_button(GTK_DIALOG(ccode_window), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
	btnCancel = gtk_dialog_add_button(GTK_DIALOG(ccode_window), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	btnSave   = gtk_dialog_add_button(GTK_DIALOG(ccode_window), GTK_STOCK_SAVE, GTK_RESPONSE_OK);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(ccode_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						GTK_RESPONSE_APPLY,
						-1);
#endif
	
	// Initialize the internal data variables.
	ccode_window_init();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(ccode_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(ccode_window);
}


/**
 * ccode_window_create_lstCountryCodes(): Create the Country Codes treeview.
 * @param container Container for the treeview.
 */
static void ccode_window_create_lstCountryCodes(GtkWidget *container)
{
	// Scrolled Window for the treeview. (Used mainly for the border.)
	GtkWidget *scrlList = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlList), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlList),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_NEVER);
	gtk_box_pack_start(GTK_BOX(container), scrlList, TRUE, TRUE, 0);
	
	// Create the list store for the tree view.
	if (lmCountryCodes)
		gtk_list_store_clear(lmCountryCodes);
	else
		lmCountryCodes = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	
	// Create a treeview for the country codes.
	lstCountryCodes = gtk_tree_view_new_with_model(GTK_TREE_MODEL(lmCountryCodes));
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstCountryCodes), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(lstCountryCodes), FALSE);
	gtk_widget_show(lstCountryCodes);
	gtk_container_add(GTK_CONTAINER(scrlList), lstCountryCodes);
	g_signal_connect((gpointer)lstCountryCodes, "drag-drop",
			 G_CALLBACK(ccode_window_callback_lstCountryCodes_drag_drop), NULL);
	
	// Create the renderer and the columns.
	GtkCellRenderer  *rendCountry = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colCountry = gtk_tree_view_column_new_with_attributes("Country", rendCountry, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstCountryCodes), colCountry);
}


/**
 * ccode_window_create_up_down_buttons(): Create the Up/Down buttons.
 * @param container Container for the buttons.
 */
static void ccode_window_create_up_down_buttons(GtkWidget *container)
{
	// VBox for the Up/Down buttons.
	GtkWidget *vboxUpDown = gtk_vbox_new(TRUE, 8);
	gtk_widget_show(vboxUpDown);
	gtk_box_pack_start(GTK_BOX(container), vboxUpDown, FALSE, FALSE, 0);
	
	// "Up" button.
	GtkWidget *btnUp = gtk_button_new();
	gtk_widget_show(btnUp);
	gtk_box_pack_start(GTK_BOX(vboxUpDown), btnUp, TRUE, TRUE, 0);
	g_signal_connect((gpointer)btnUp, "clicked",
			 G_CALLBACK(ccode_window_callback_btnUp_clicked), NULL);
	
	// "Up" button icon.
	GtkWidget *iconUp = gtk_image_new_from_stock(GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(iconUp);
	gtk_button_set_image(GTK_BUTTON(btnUp), iconUp);
	
	// "Down" button.
	GtkWidget *btnDown = gtk_button_new();
	gtk_widget_show(btnDown);
	gtk_box_pack_start(GTK_BOX(vboxUpDown), btnDown, TRUE, TRUE, 0);
	g_signal_connect((gpointer)btnDown, "clicked",
			 G_CALLBACK(ccode_window_callback_btnDown_clicked), NULL);
	
	// "Down" button icon.
	GtkWidget *iconDown = gtk_image_new_from_stock(GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(iconDown);
	gtk_button_set_image(GTK_BUTTON(btnDown), iconDown);
}


/**
 * ccode_window_close(): Close the About window.
 */
void ccode_window_close(void)
{
	if (!ccode_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(ccode_window);
	ccode_window = NULL;
}


/**
 * ccode_window_init(): Initialize the file text boxes.
 */
static void ccode_window_init(void)
{
	// Set up the country order treeview.
	// Elements in Country_Order[3] can have one of three values:
	// - 0 [USA]
	// - 1 [JAP]
	// - 2 [EUR]
	
	// Make sure the country code order is valid.
	Check_Country_Order();
	
	// Clear the list store.
	gtk_list_store_clear(lmCountryCodes);
	
	// Country codes.
	// TODO: Move this to a common file.
	static const char* const ccodes[3] = {"USA", "Japan", "Europe"};
	
	// Add the country codes to the treeview in the appropriate order.
	GtkTreeIter iter;
	int i;
	for (i = 0; i < 3; i++)
	{
		gtk_list_store_append(lmCountryCodes, &iter);
		gtk_list_store_set(GTK_LIST_STORE(lmCountryCodes), &iter,
				   0, ccodes[Country_Order[i]],
				   1, Country_Order[i], -1);
	}
	
	// Disable the "Apply" button initially.
	gtk_widget_set_sensitive(btnApply, FALSE);
}


/**
 * ccode_window_save(): Save the Country Code order.
 */
static void ccode_window_save(void)
{
	// Save settings.
	gboolean valid;
	GtkTreeIter iter;
	int countryID;
	
	// Copy each item in the listview to the array.
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCountryCodes), &iter);
	int i;
	for (i = 0; valid && i < 3; i++)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(lmCountryCodes), &iter, 1, &countryID, -1);
		Country_Order[i] = countryID;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCountryCodes), &iter);
	}
	
	// Validate the country code order.
	Check_Country_Order();
	
	// Disable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, FALSE);
}


/**
 * ccode_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean ccode_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	ccode_window_close();
	return FALSE;
}


/**
 * ccode_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void ccode_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(dialog);
	GSFT_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			ccode_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			ccode_window_save();
			break;
		case GTK_RESPONSE_OK:
			ccode_window_save();
			ccode_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * ccode_window_callback_btnUp_clicked(): "Up" button was clicked.
 * @param button
 * @param user_data
 */
static void ccode_window_callback_btnUp_clicked(GtkButton *button, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(button);
	GSFT_UNUSED_PARAMETER(user_data);
	
	GtkTreeSelection *selection;
	GtkTreeIter iter, prevIter;
	gboolean notFirst, valid;
	
	// Get the current selection.
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstCountryCodes));
	
	// Find the selection and swap it with the item immediately before it.
	notFirst = FALSE;
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCountryCodes), &iter);
	while (valid)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			// Found the selection.
			if (notFirst)
			{
				// Not the first item. Swap it with the previous item.
				gtk_list_store_swap(lmCountryCodes, &iter, &prevIter);
			
				// Enable the "Apply" button.
				gtk_widget_set_sensitive(btnApply, TRUE);
			}
			break;
		}
		else
		{
			// Not selected. Store this iter as prevIter.
			prevIter = iter;
			// Since this isn't the first item, set notFirst.
			notFirst = TRUE;
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCountryCodes), &iter);
		}
	}

}


/**
 * ccode_window_callback_btnDown_clicked(): "Down" button was clicked.
 * @param button
 * @param user_data
 */
static void ccode_window_callback_btnDown_clicked(GtkButton *button, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(button);
	GSFT_UNUSED_PARAMETER(user_data);
	
	GtkTreeSelection *selection;
	GtkTreeIter iter, nextIter;
	gboolean valid;
	
	// Get the current selection.
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstCountryCodes));
	
	// Find the selection and swap it with the item immediately after it.
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCountryCodes), &iter);
	while (valid)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &iter))
		{
			// Found the selection. Check if there's another item after it.
			nextIter = iter;
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCountryCodes), &nextIter);
			if (valid)
			{
				// Not the last item. Swap it with the next item.
				gtk_list_store_swap(lmCountryCodes, &iter, &nextIter);
				
				// Enable the "Apply" button.
				gtk_widget_set_sensitive(btnApply, TRUE);
			}
			break;
		}
		else
		{
			// Not selected.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCountryCodes), &iter);
		}
	}
}


static gboolean ccode_window_callback_lstCountryCodes_drag_drop(GtkWidget	*widget,
								GdkDragContext	*drag_context,
								gint		x,
								gint		y,
								guint		time_,
								gpointer	user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(drag_context);
	GSFT_UNUSED_PARAMETER(x);
	GSFT_UNUSED_PARAMETER(y);
	GSFT_UNUSED_PARAMETER(time_);
	GSFT_UNUSED_PARAMETER(user_data);
	
	// Enable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, TRUE);
	
	return FALSE;
}
