/***************************************************************************
 * Gens: (GTK+) Plugin Manager Window.                                     *
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

#include "pmgr_window.hpp"
#include "ui/common/pmgr_window_common.hpp"
#include "gens/gens_window.h"

// C includes.
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// GTK+ includes.
#include <gtk/gtk.h>

// Unused Parameter macro.
#include "macros/unused.h"

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// Internal plugins.
// TODO: Figure out a better way to mark plugins as internal or external.
#include "plugins/render/normal/mdp_render_1x_plugin.h"
#include "plugins/render/double/mdp_render_2x_plugin.h"

// C++ includes
#include <string>
#include <sstream>
#include <list>
using std::endl;
using std::string;
using std::stringstream;
using std::list;


// Window.
GtkWidget *pmgr_window = NULL;

// Widgets.
static GtkWidget	*lblPluginMainInfo;
static GtkWidget	*lblPluginSecInfo;
static GtkWidget	*lblPluginDesc;
static GtkWidget	*fraPluginDesc;

// Plugin List.
typedef enum _pmgr_type_t
{
	PMGR_INTERNAL = 0,
	PMGR_EXTERNAL = 1,
	PMGR_INCOMPAT = 2,
	
	PMGR_MAX = 3
} pmgr_type_t;

static GtkWidget	*lstPluginList[PMGR_MAX];
static GtkListStore	*lmPluginList[PMGR_MAX] = {NULL, NULL, NULL};

// Widget creation functions.
static void	pmgr_window_create_plugin_list_notebook(GtkWidget *container);
static void	pmgr_window_create_plugin_list_page(GtkWidget *container, const char *title, int id);
static void	pmgr_window_create_plugin_info_frame(GtkWidget *container);
static void	pmgr_window_populate_plugin_list(void);

// Callbacks.
static gboolean	pmgr_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	pmgr_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	pmgr_window_callback_lstPluginList_cursor_changed(GtkTreeView *tree_view, gpointer user_data);
static void	pmgr_window_callback_fraPluginDesc_size_allocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);

// Width of fraPluginDesc
static gint	fraPluginDesc_width;

// Plugin icon functions and variables.
#ifdef GENS_PNG
static GtkWidget *imgPluginIcon;

static void	pmgr_window_create_plugin_icon_widget(GtkWidget *container);
static GdkPixbuf* pmgr_window_create_pixbuf_from_png(const uint8_t *icon, const unsigned int iconLength);
#endif


/**
 * pmgr_window_show(): Show the Plugin Manager window.
 */
void pmgr_window_show()
{
	if (pmgr_window)
	{
		// Plugin Manager window is already visible. Set focus.
		gtk_widget_grab_focus(pmgr_window);
		return;
	}
	
	// Create the window.
	pmgr_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(pmgr_window), 4);
	gtk_window_set_title(GTK_WINDOW(pmgr_window), "Plugin Manager");
	gtk_window_set_position(GTK_WINDOW(pmgr_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(pmgr_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(pmgr_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(pmgr_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)pmgr_window, "delete_event",
			  G_CALLBACK(pmgr_window_callback_close), NULL);
	g_signal_connect((gpointer)pmgr_window, "destroy_event",
			  G_CALLBACK(pmgr_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(pmgr_window), "response",
			  G_CALLBACK(pmgr_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(pmgr_window));
	gtk_widget_show(vboxDialog);
	
	// Create the plugin list frame.
	pmgr_window_create_plugin_list_notebook(vboxDialog);
	
	// Create the plugin information frame.
	pmgr_window_create_plugin_info_frame(vboxDialog);
	
	// Create the "Close" button.
	gtk_dialog_add_button(GTK_DIALOG(pmgr_window),
			      GTK_STOCK_CLOSE,
			      GTK_RESPONSE_CLOSE);
	
	// Populate the plugin list.
	pmgr_window_populate_plugin_list();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(pmgr_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(pmgr_window);
	
	// Make sure nothing is selected initially.
	for (int i = 0; i < 2; i++)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstPluginList[i]));
		gtk_tree_selection_unselect_all(selection);
		pmgr_window_callback_lstPluginList_cursor_changed(GTK_TREE_VIEW(lstPluginList[i]), GINT_TO_POINTER(i));
	}
}


/**
 * pmgr_window_create_plugin_list_notebook(): Create the plugin list notebook.
 * @param container Container for the notebook.
 */
static void pmgr_window_create_plugin_list_notebook(GtkWidget *container)
{
	// Create the plugin list notebook.
	GtkWidget *tabPluginList = gtk_notebook_new();
	gtk_container_set_border_width(GTK_CONTAINER(tabPluginList), 4);
	gtk_widget_show(tabPluginList);
	gtk_box_pack_start(GTK_BOX(container), tabPluginList, TRUE, TRUE, 0);
	
	// Create the pages.
	pmgr_window_create_plugin_list_page(tabPluginList, "_Internal", PMGR_INTERNAL);
	pmgr_window_create_plugin_list_page(tabPluginList, "_External", PMGR_EXTERNAL);
	pmgr_window_create_plugin_list_page(tabPluginList, "I_ncompatible", PMGR_INCOMPAT);
}


/**
 * pmgr_window_create_plugin_list_page(): Create a plugin list page.
 * @param container Container for the page.
 * @param title Title of the page.
 * @param id Page ID.
 */
static void pmgr_window_create_plugin_list_page(GtkWidget *container, const char *title, int id)
{
	// Create the label for the notebook page.
	GtkWidget *lblPluginList = gtk_label_new_with_mnemonic(title);
	gtk_widget_show(lblPluginList);
	
	// VBox for the plugin list.
	GtkWidget *vboxPluginList = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vboxPluginList), 8);
	gtk_widget_show(vboxPluginList);
	
	// Append the page to the notebook.
	gtk_notebook_append_page(GTK_NOTEBOOK(container), vboxPluginList, lblPluginList);
	
	// Scrolled Window for the plugin list.
	GtkWidget *scrlPluginList = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrlPluginList), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlPluginList),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrlPluginList);
	gtk_box_pack_start(GTK_BOX(vboxPluginList), scrlPluginList, TRUE, TRUE, 0);
	
	// Tree view containing the plugins.
	lstPluginList[id] = gtk_tree_view_new();
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstPluginList[id]), FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(lstPluginList[id]), FALSE);
	gtk_widget_set_size_request(lstPluginList[id], 480, 200);
	gtk_widget_show(lstPluginList[id]);
	gtk_container_add(GTK_CONTAINER(scrlPluginList), lstPluginList[id]);
	
	// Connect the treeview's "cursor-changed" signal.
	g_signal_connect((gpointer)lstPluginList[id], "cursor-changed",
			 G_CALLBACK(pmgr_window_callback_lstPluginList_cursor_changed),
			 GINT_TO_POINTER(id));
}


/**
 * pmgr_window_create_plugin_info_frame(): Create the plugin information frame.
 * @param container Container for the frame.
 */
static void pmgr_window_create_plugin_info_frame(GtkWidget *container)
{
	// Create the plugin information frame.
	GtkWidget *fraPluginInfo = gtk_frame_new("<b><i>Plugin Information</i></b>");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPluginInfo), GTK_SHADOW_ETCHED_IN);
	gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginInfo))), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(fraPluginInfo), 4);
	gtk_widget_show(fraPluginInfo);
	gtk_box_pack_start(GTK_BOX(container), fraPluginInfo, TRUE, TRUE, 0);
	
	// VBox for the plugin information frame.
	GtkWidget *vboxPluginInfo = gtk_vbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(vboxPluginInfo), 8);
	gtk_widget_show(vboxPluginInfo);
	gtk_container_add(GTK_CONTAINER(fraPluginInfo), vboxPluginInfo);
	
	// HBox for the main plugin info.
	GtkWidget *hboxPluginMainInfo = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hboxPluginMainInfo);
	gtk_box_pack_start(GTK_BOX(vboxPluginInfo), hboxPluginMainInfo, TRUE, FALSE, 0);
	
#ifdef GENS_PNG
	// Create the plugin icon widget.
	pmgr_window_create_plugin_icon_widget(hboxPluginMainInfo);
#endif /* GENS_PNG */
	
	// VBox for the main plugin info.
	GtkWidget *vboxPluginMainInfo = gtk_vbox_new(FALSE, 4);
	gtk_widget_show(vboxPluginMainInfo);
	gtk_box_pack_start(GTK_BOX(hboxPluginMainInfo), vboxPluginMainInfo, TRUE, TRUE, 0);
	
	// Label for the main plugin info.
	lblPluginMainInfo = gtk_label_new("\n\n\n\n\n");
	gtk_label_set_selectable(GTK_LABEL(lblPluginMainInfo), TRUE);
	gtk_misc_set_alignment(GTK_MISC(lblPluginMainInfo), 0.0f, 0.0f);
	gtk_widget_show(lblPluginMainInfo);
	gtk_box_pack_start(GTK_BOX(vboxPluginMainInfo), lblPluginMainInfo, TRUE, FALSE, 0);
	
	// Label for secondary plugin info.
	lblPluginSecInfo = gtk_label_new("\n");
	gtk_label_set_selectable(GTK_LABEL(lblPluginSecInfo), TRUE);
	gtk_misc_set_alignment(GTK_MISC(lblPluginSecInfo), 0.0f, 0.0f);
	gtk_widget_show(lblPluginSecInfo);
	gtk_container_add(GTK_CONTAINER(vboxPluginInfo), lblPluginSecInfo);
	
	// Frame for the plugin description.
	fraPluginDesc_width = 0;
	fraPluginDesc = gtk_frame_new(" ");
	g_signal_connect((gpointer)fraPluginDesc, "size-allocate",
			  G_CALLBACK(pmgr_window_callback_fraPluginDesc_size_allocate), NULL);	
	gtk_frame_set_shadow_type(GTK_FRAME(fraPluginDesc), GTK_SHADOW_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(fraPluginDesc), 4);
	gtk_widget_show(fraPluginDesc);
	gtk_box_pack_start(GTK_BOX(vboxPluginInfo), fraPluginDesc, TRUE, TRUE, 0);
	
	// Label for the plugin description.
	lblPluginDesc = gtk_label_new(NULL);
	gtk_label_set_selectable(GTK_LABEL(lblPluginDesc), TRUE);
	gtk_label_set_line_wrap(GTK_LABEL(lblPluginDesc), TRUE);
	gtk_widget_set_size_request(lblPluginDesc, -1, 64);
	gtk_misc_set_alignment(GTK_MISC(lblPluginDesc), 0.0f, 0.0f);
	gtk_widget_show(lblPluginDesc);
	gtk_container_add(GTK_CONTAINER(fraPluginDesc), lblPluginDesc);
	gtk_widget_show_all(pmgr_window);
}


/**
 * pmgr_window_populate_plugin_list(): Populate the plugin list.
 */
static void pmgr_window_populate_plugin_list(void)
{
	// Check if the list models are already created.
	// If they are, clear them; otherwise, create them.
	
	GtkTreeViewColumn *colPlugin;
	GtkCellRenderer *renderer;
	
#ifdef GENS_PNG
	GtkTreeViewColumn *colIcon;
#endif
	
	for (int i = 0; i < PMGR_MAX; i++)
	{
		if (lmPluginList[i])
			gtk_list_store_clear(lmPluginList[i]);
		else
		{
#ifdef GENS_PNG
			lmPluginList[i] = gtk_list_store_new(3,
				G_TYPE_STRING,		// Plugin name.
				G_TYPE_POINTER,		// mdp_t* pointer.
				GDK_TYPE_PIXBUF);	// Plugin icon.
#else
			lmPluginList[i] = gtk_list_store_new(2,
				G_TYPE_STRING,		// Plugin name.
				G_TYPE_POINTER);	// mdp_t* pointer.
#endif
		}
		
		// Set the view model of the treeview.
		gtk_tree_view_set_model(GTK_TREE_VIEW(lstPluginList[i]),
					GTK_TREE_MODEL(lmPluginList[i]));
		
		// Delete any existing columns.
		do
		{
			colPlugin = gtk_tree_view_get_column(GTK_TREE_VIEW(lstPluginList[i]), 0);
			if (colPlugin)
				gtk_tree_view_remove_column(GTK_TREE_VIEW(lstPluginList[i]), colPlugin);
		} while (colPlugin != NULL);
		
		// Create the renderers and columns.
		
#ifdef GENS_PNG
		// Icon.
		renderer = gtk_cell_renderer_pixbuf_new();
		colIcon = gtk_tree_view_column_new_with_attributes("Icon", renderer, "pixbuf", 2, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(lstPluginList[i]), colIcon);
#endif
		
		// Plugin name.
		renderer = gtk_cell_renderer_text_new();
		colPlugin = gtk_tree_view_column_new_with_attributes("Plugin", renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(lstPluginList[i]), colPlugin);
	}
	
	// Add all plugins to the treeviews.
	// TODO: Use the appropriate treeviews, depending on the plugin type/state.
	char tmp[64];
	GtkTreeIter iter;
	pmgr_type_t pmtype;
	
	for (list<mdp_t*>::iterator curPlugin = PluginMgr::lstMDP.begin();
	     curPlugin != PluginMgr::lstMDP.end(); curPlugin++)
	{
		mdp_t *plugin = (*curPlugin);
		const char *pluginName;
		if (plugin->desc && plugin->desc->name)
		{
			pluginName = plugin->desc->name;
		}
		else
		{
			// No description or name.
			// TODO: For external plugins, indicate the external file.
			sprintf(tmp, "[No name: 0x%08X]", (unsigned int)plugin);
			pluginName = tmp;
		}
		
		// Check if this is internal or external.
		// TODO: "Normal" and "Double" are currently hard-coded.
		// Use a better method for determining internal vs. external.
		if (plugin == &mdp_render_1x ||
		    plugin == &mdp_render_2x)
		{
			// Internal plugin.
			pmtype = PMGR_INTERNAL;
		}
		else
		{
			// External plugin.
			pmtype = PMGR_EXTERNAL;
		}
		
		// Add an entry to the list store.
		gtk_list_store_append(lmPluginList[pmtype], &iter);
#ifdef GENS_PNG
		// Create the pixbuf for the plugin icon.
		GdkPixbuf *pbufIcon = NULL;
		if (plugin->desc)
		{
			pbufIcon = pmgr_window_create_pixbuf_from_png(plugin->desc->icon, plugin->desc->iconLength);
		}
		gtk_list_store_set(GTK_LIST_STORE(lmPluginList[pmtype]), &iter, 0, pluginName, 1, plugin, 2, pbufIcon, -1);
		
		if (pbufIcon)
		{
			// Unreference the pixbuf.
			g_object_unref(pbufIcon);
		}
#else
		gtk_list_store_set(GTK_LIST_STORE(lmPluginList[pmtype]), &iter, 0, pluginName, 1, plugin, -1);
#endif /* GENS_PNG */
	}
}


/**
 * pmgr_window_close(): Close the Plugin Manager window.
 */
void pmgr_window_close(void)
{
	if (!pmgr_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(pmgr_window);
	pmgr_window = NULL;
	
#ifdef GENS_PNG
	// Free all pixbufs in the list model.
	GtkTreeIter iter;
	GdkPixbuf *pbufIcon;
	
	for (int i = 0; i < PMGR_MAX; i++)
	{
		gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmPluginList[i]), &iter);
		while (valid)
		{
			gtk_tree_model_get(GTK_TREE_MODEL(lmPluginList[i]), &iter, 2, &pbufIcon, -1);
			if (pbufIcon)
				g_object_unref(pbufIcon);
			
			// Get the next list element.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmPluginList[i]), &iter);
		}
	}
#endif
	
	// Clear the plugin lists.
	for (int i = 0; i < PMGR_MAX; i++)
		gtk_list_store_clear(lmPluginList[i]);
}


/**
 * pmgr_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean pmgr_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	pmgr_window_close();
	return FALSE;
}


/**
 * pmgr_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void pmgr_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(dialog);
	GENS_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CLOSE:
			pmgr_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * pmgr_window_callback_lstPluginList_cursor_changed(): Cursor position has changed.
 * @param tree_view Tree view.
 * @param user_data User data.
 */
static void pmgr_window_callback_lstPluginList_cursor_changed(GtkTreeView *tree_view, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(tree_view);
	
	int id = GPOINTER_TO_INT(user_data);
	if (id < 0 || id >= PMGR_MAX)
		return;
	
	// Check which plugin is clicked.
	GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
	
	GtkTreeIter iter;
	GtkTreeModel *gtm = GTK_TREE_MODEL(lmPluginList[id]);
	
	if (!gtk_tree_selection_get_selected(selection, &gtm, &iter))
	{
		// No plugin selected.
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "No plugin selected.\n\n\n\n\n");
		gtk_label_set_text(GTK_LABEL(lblPluginSecInfo), "\n");
		gtk_label_set_text(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginDesc))), " ");
		gtk_label_set_text(GTK_LABEL(lblPluginDesc), NULL);
#ifdef GENS_PNG
		gtk_image_clear(GTK_IMAGE(imgPluginIcon));
#endif /* GENS_PNG */
		return;
	}
	
	// Found a selected plugin.
#ifdef GENS_PNG
	mdp_t *plugin;
	GdkPixbuf *pbufIcon;
	
	// Get the plugin icon and mdp_t*.
	gtk_tree_model_get(gtm, &iter, 1, &plugin, 2, &pbufIcon, -1);
#else
	// Get the mdp_t*.
	gtk_tree_model_get(gtm, &iter, 1, &plugin, -1);
#endif
	
	// Get the plugin information.
	if (!plugin)
	{
		// Invalid plugin.
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "Invalid plugin selected.\n\n\n\n\n");
		gtk_label_set_text(GTK_LABEL(lblPluginSecInfo), "\n");
		gtk_label_set_text(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginDesc))), " ");
		gtk_label_set_text(GTK_LABEL(lblPluginDesc), NULL);
#ifdef GENS_PNG
		gtk_image_clear(GTK_IMAGE(imgPluginIcon));
#endif /* GENS_PNG */
		return;
	}
	
	if (!plugin->desc)
	{
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "This plugin does not have a valid description field.\n\n\n\n\n");
		gtk_label_set_text(GTK_LABEL(lblPluginSecInfo), "\n");
		gtk_label_set_text(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginDesc))), " ");
		gtk_label_set_text(GTK_LABEL(lblPluginDesc), NULL);
#ifdef GENS_PNG
		gtk_image_clear(GTK_IMAGE(imgPluginIcon));
#endif /* GENS_PNG */
		return;
	}
	
	// Fill in the descriptions.
	mdp_desc_t *desc = plugin->desc;
	stringstream ssMainInfo;
	int lines = 4;			// Name, MDP Author, Version, and License are always printed.
	const int linesReserved = 6;	// Number of lines reserved.
	
	// Plugin name.
	ssMainInfo << "Name: " << (desc->name ? string(desc->name) : "(none)") << endl;
	
	// Plugin version.
	ssMainInfo << "Version: " << MDP_VERSION_MAJOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_MINOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_REVISION(plugin->pluginVersion) << endl;
	
	// Plugin author.
	ssMainInfo << "MDP Author: " + (desc->author_mdp ? string(desc->author_mdp) : "(none)") << endl;
	
	// Original code author.
	if (desc->author_orig)
	{
		ssMainInfo << "Original Author: " << string(desc->author_orig) << endl;
		lines++;
	}
	
	// Website.
	if (desc->website)
	{
		ssMainInfo << "Website: " << string(desc->website) << endl;
		lines++;
	}
	
	// License.
	ssMainInfo << "License: " + ((desc->license && desc->license[0]) ? string(desc->license) : "(none)");
	
	// Linebreaks needed.
	const int linesNeeded = linesReserved - lines;
	for (int i = 0; i < linesNeeded; i++)
	{
		ssMainInfo << endl;
	}
	
	// Set the main plugin information.
	gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), ssMainInfo.str().c_str());
	
	// UUID.
	string sUUID = UUIDtoString(plugin->uuid);
	
	// Secondary plugin information.
	// Includes UUID and CPU flags.
	stringstream ssSecInfo;
	ssSecInfo << "UUID: " << sUUID << endl
		  << GetCPUFlags_string(plugin->cpuFlagsRequired, plugin->cpuFlagsSupported, true);
	
	// Set the secondary information label.
	gtk_label_set_text(GTK_LABEL(lblPluginSecInfo), ssSecInfo.str().c_str());
	gtk_label_set_use_markup(GTK_LABEL(lblPluginSecInfo), TRUE);
	
	// Plugin description.
	gtk_label_set_text(GTK_LABEL(lblPluginDesc), desc->description);
	if (desc->description)
	{
		gtk_label_set_text(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginDesc))), "<b><i>Description:</i></b>");
		gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginDesc))), TRUE);
		gtk_widget_set_size_request(lblPluginDesc, fraPluginDesc_width - 12, 64);
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginDesc))), " ");
	}
	
#ifdef GENS_PNG
	// Set the plugin icon.
	if (pbufIcon)
	{
		// Plugin icon found. Set it.
		gtk_image_set_from_pixbuf(GTK_IMAGE(imgPluginIcon), pbufIcon);
		g_object_unref(pbufIcon);
	}
	else
	{
		// No plugin icon found. Clear the icon.
		gtk_image_clear(GTK_IMAGE(imgPluginIcon));
	}
#endif
}


/**
 * pmgr_window_callback_fraPluginDesc_size_allocate(): fraPluginDesc's size has been allocated.
 * @param widget
 * @param allocation
 * @param user_data
 */
static void pmgr_window_callback_fraPluginDesc_size_allocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	fraPluginDesc_width = allocation->width;
}


#ifdef GENS_PNG
/**
 * pmgr_window_create_plugin_icon_widget(): Create the GTK+ plugin icon widget and pixbuf.
 * @param container Container for the plugin icon widget.
 */
static void pmgr_window_create_plugin_icon_widget(GtkWidget *container)
{
	// Plugin icon widget.
	imgPluginIcon = gtk_image_new();
	gtk_misc_set_alignment(GTK_MISC(imgPluginIcon), 0.0f, 0.0f);
	gtk_widget_show(imgPluginIcon);
	gtk_box_pack_start(GTK_BOX(container), imgPluginIcon, FALSE, FALSE, 0);
	gtk_widget_set_size_request(imgPluginIcon, 32, 32);
	
	// Clear the icon.
	gtk_image_clear(GTK_IMAGE(imgPluginIcon));
}


/**
 * pmgr_window_create_pixbuf_from_png(): Create a pixbuf from a PNG image.
 * @param icon Icon data. (PNG format)
 * @param iconLength Length of the icon data.
 * @return Pixbuf containing the icon, or NULL on error.
 */
static GdkPixbuf* pmgr_window_create_pixbuf_from_png(const uint8_t *icon, const unsigned int iconLength)
{
	static const unsigned char pngMagicNumber[8] = {0x89, 'P', 'N', 'G',0x0D, 0x0A, 0x1A, 0x0A};
	
	if (!icon || iconLength < sizeof(pngMagicNumber))
		return NULL;
	
	// Check that the icon is in PNG format.
	if (memcmp(icon, pngMagicNumber, sizeof(pngMagicNumber)))
	{
		// Not in PNG format.
		return NULL;
	}

	// Initialize libpng.
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return NULL;
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// An error occurred while attepmting to decode the PNG image.
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Initialize the custom read function.
	pmgr_window_png_dataptr = icon;
	pmgr_window_png_datalen = iconLength;
	pmgr_window_png_datapos = 0;
	
	void *read_io_ptr = png_get_io_ptr(png_ptr);
	png_set_read_fn(png_ptr, read_io_ptr, &pmgr_window_png_user_read_data);
	
	// Get the PNG information.
	png_read_info(png_ptr, info_ptr);
	
	// Get the PNG information.
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	bool has_alpha = false;
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_method);
	
	if (width != 32 || height != 32)
	{
		// Not 32x32.
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Make sure RGB color is used.
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_GRAY)
		png_set_gray_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		has_alpha = true;
	
	// GTK+ expects RGBA format.
	// TODO: Check if this is the same on big-endian machines.
	
	// Convert tRNS to alpha channel.
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
		has_alpha = true;
	}
	
	// Convert 16-bit per channel PNG to 8-bit.
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	
	// Get the new PNG information.
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_method);
	
	// Check if the PNG image has an alpha channel.
	if (!has_alpha)
	{
		// No alpha channel specified.
		// Use filler instead.
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}
	
	// Update the PNG info.
	png_read_update_info(png_ptr, info_ptr);
	
	// Create the row pointers.
	GdkPixbuf *pbufIcon = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 32, 32);
	
	int rowstride = gdk_pixbuf_get_rowstride(pbufIcon);
	guchar *pixels = gdk_pixbuf_get_pixels(pbufIcon);
	png_bytep row_pointers[32];
	for (unsigned int i = 0; i < 32; i++)
	{
		row_pointers[i] = pixels;
		pixels += rowstride;
	}
	
	// Read the image data.
	png_read_image(png_ptr, row_pointers);
	
	// Close the PNG image.
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	
	// Return the pixbuf.
	return pbufIcon;
}
#endif /* GENS_PNG */
