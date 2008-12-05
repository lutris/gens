/***************************************************************************
 * Gens: (GTK+) Plugin Manager Window.                                     *
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

#include "plugin_manager_window.hpp"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// TODO: Get rid of gtk-misc.h
#include "gtk-misc.h"

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// C++ includes
#include <string>
#include <sstream>
#include <vector>
using std::endl;
using std::string;
using std::stringstream;
using std::vector;


PluginManagerWindow* PluginManagerWindow::m_Instance = NULL;
PluginManagerWindow* PluginManagerWindow::Instance(GtkWindow *parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the General Options window.
		m_Instance = new PluginManagerWindow();
	}
	else
	{
		// Instance already exists. Set focus.
		m_Instance->setFocus();
	}
	
	// Set modality of the window.
	if (!parent)
		parent = GTK_WINDOW(gens_window);
	m_Instance->setModal(parent);
	
	return m_Instance;
}


PluginManagerWindow::~PluginManagerWindow()
{
	if (m_Window)
		gtk_widget_destroy(GTK_WIDGET(m_Window));
	
	m_Instance = NULL;
}


gboolean PluginManagerWindow::GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return reinterpret_cast<PluginManagerWindow*>(user_data)->close();
}


void PluginManagerWindow::dlgButtonPress(uint32_t button)
{
	switch (button)
	{
		case WndBase::BUTTON_OK:
			close();
			break;
	}
}


/**
 * Window is closed.
 */
gboolean PluginManagerWindow::close(void)
{
	delete this;
	return FALSE;
}


/**
 * PluginManagerWindow: Create the General Options Window.
 */
PluginManagerWindow::PluginManagerWindow()
{
	m_Window = gtk_dialog_new();
	gtk_widget_set_name(GTK_WIDGET(m_Window), "PluginManagerWindow");
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 0);
	gtk_window_set_title(GTK_WINDOW(m_Window), "Plugin Manager");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), FALSE);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(m_Window), "PluginManagerWindow", m_Window);
	
	// Load the Gens icon.
	GdkPixbuf *icon = create_pixbuf("Gens2.ico");
	if (icon)
	{
		gtk_window_set_icon(GTK_WINDOW(m_Window), icon);
		gdk_pixbuf_unref(icon);
	}
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)m_Window, "delete_event",
			  G_CALLBACK(PluginManagerWindow::GTK_Close), (gpointer)this);
	g_signal_connect((gpointer)m_Window, "destroy_event",
			  G_CALLBACK(PluginManagerWindow::GTK_Close), (gpointer)this);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(m_Window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the plugin list frame.
	createPluginListFrame(GTK_BOX(vboxDialog));
	
	// Create the plugin information frame.
	createPluginInfoFrame(GTK_BOX(vboxDialog));
	
	// Create an accelerator group.
	m_AccelTable = gtk_accel_group_new();
	
	// Add the OK button.
	addDialogButtons(m_Window, WndBase::BAlign_Default,
			 WndBase::BUTTON_OK, 0,
			 WndBase::BUTTON_ALL);
	
	// Add the accel group to the window.
	gtk_window_add_accel_group(GTK_WINDOW(m_Window), GTK_ACCEL_GROUP(m_AccelTable));
	
	// Populate the plugin list.
	lmPluginList = NULL;
	populatePluginList();
	
	// Show the window.
	setVisible(true);
	
	// Make sure nothing is selected initially.
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstPluginList));
	gtk_tree_selection_unselect_all(selection);
	lstPluginList_cursor_changed(NULL);
}


void PluginManagerWindow::createPluginListFrame(GtkBox *container)
{
	// Create the plugin list frame.
	GtkWidget *fraPluginList = gtk_frame_new(NULL);
	gtk_widget_set_name(fraPluginList, "fraPluginList");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPluginList), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraPluginList), 4);
	gtk_widget_show(fraPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "fraPluginList",
			       g_object_ref(fraPluginList), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(container, fraPluginList, TRUE, TRUE, 0);
	
	// Label for the plugin list frame.
	GtkWidget *lblPluginList = gtk_label_new("<b><i>Internal Plugins</i></b>");
	gtk_widget_set_name(lblPluginList, "lblPluginList");
	gtk_label_set_use_markup(GTK_LABEL(lblPluginList), TRUE);
	gtk_widget_show(lblPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginList",
			       g_object_ref(lblPluginList), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraPluginList), lblPluginList);
	
	// Scrolled Window for the plugin list.
	GtkWidget *scrlPluginList = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_name(scrlPluginList, "scrlPluginList");
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlPluginList),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scrlPluginList);
	gtk_container_add(GTK_CONTAINER(fraPluginList), scrlPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "scrlPluginList",
			       g_object_ref(scrlPluginList), (GDestroyNotify)g_object_unref);
	
	// Tree view containing the plugins.
	lstPluginList = gtk_tree_view_new();
	gtk_widget_set_name(lstPluginList, "lstPluginList");
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstPluginList), FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(lstPluginList), FALSE);
	gtk_widget_set_size_request(lstPluginList, 480, 160);
	gtk_widget_show(lstPluginList);
	gtk_container_add(GTK_CONTAINER(scrlPluginList), lstPluginList);
	g_object_set_data_full(G_OBJECT(m_Window), "lstPluginList",
			       g_object_ref(lstPluginList), (GDestroyNotify)g_object_unref);
	
	// Connect the treeview's "cursor-changed" signal.
	g_signal_connect((gpointer)lstPluginList, "cursor-changed",
			 G_CALLBACK(lstPluginList_cursor_changed_STATIC), this);
}


void PluginManagerWindow::createPluginInfoFrame(GtkBox *container)
{
	// Create the plugin information frame.
	GtkWidget *fraPluginInfo = gtk_frame_new(NULL);
	gtk_widget_set_name(fraPluginInfo, "fraPluginInfo");
	gtk_frame_set_shadow_type(GTK_FRAME(fraPluginInfo), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraPluginInfo), 4);
	gtk_widget_show(fraPluginInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "fraPluginInfo",
			       g_object_ref(fraPluginInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(container, fraPluginInfo, TRUE, TRUE, 0);
	
	// Label for the plugin information frame.
	GtkWidget *lblPluginInfo = gtk_label_new("<b><i>Plugin Information</i></b>");
	gtk_widget_set_name(lblPluginInfo, "lblPluginInfo");
	gtk_label_set_use_markup(GTK_LABEL(lblPluginInfo), TRUE);
	gtk_widget_show(lblPluginInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginInfo",
			       g_object_ref(lblPluginInfo), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraPluginInfo), lblPluginInfo);
	
	// VBox for the plugin information frame.
	GtkWidget *vboxPluginInfo = gtk_vbox_new(FALSE, 8);
	gtk_container_set_border_width(GTK_CONTAINER(vboxPluginInfo), 8);
	gtk_widget_set_name(vboxPluginInfo, "vboxPluginInfo");
	gtk_widget_show(vboxPluginInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxPluginInfo",
			       g_object_ref(vboxPluginInfo), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(fraPluginInfo), vboxPluginInfo);
	
	// HBox for the main plugin info.
	GtkWidget *hboxPluginMainInfo = gtk_hbox_new(FALSE, 8);
	gtk_widget_set_name(hboxPluginMainInfo, "hboxPluginMainInfo");
	gtk_widget_show(hboxPluginMainInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "hboxPluginMainInfo",
			       g_object_ref(hboxPluginMainInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxPluginInfo), hboxPluginMainInfo, TRUE, FALSE, 0);
	
	// Plugin icon pixbuf.
	pbufPluginIcon = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 32, 32);
	g_object_set_data_full(G_OBJECT(m_Window), "pbufPluginIcon",
			       g_object_ref(pbufPluginIcon), (GDestroyNotify)g_object_unref);
	
	// Clear the pixbuf.
	guchar *pixels = gdk_pixbuf_get_pixels(pbufPluginIcon);
	int rowstride = gdk_pixbuf_get_rowstride(pbufPluginIcon);
	int height = gdk_pixbuf_get_height(pbufPluginIcon);
	memset(pixels, 0x80, rowstride * height);
	
	// Plugin icon widget.
	GtkWidget *imgPluginIcon = gtk_image_new_from_pixbuf(pbufPluginIcon);
	gtk_widget_set_name(imgPluginIcon, "imgPluginIcon");
	gtk_misc_set_alignment(GTK_MISC(imgPluginIcon), 0.0f, 0.0f);
	gtk_widget_show(imgPluginIcon);
	g_object_set_data_full(G_OBJECT(m_Window), "imgPluginIcon",
			       g_object_ref(imgPluginIcon), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(hboxPluginMainInfo), imgPluginIcon, FALSE, FALSE, 0);
	
	// VBox for the main plugin info.
	GtkWidget *vboxPluginMainInfo = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_name(vboxPluginMainInfo, "vboxPluginMainInfo");
	gtk_widget_show(vboxPluginMainInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxPluginMainInfo",
			       g_object_ref(vboxPluginMainInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(hboxPluginMainInfo), vboxPluginMainInfo, TRUE, TRUE, 0);
	
	// Label for the main plugin info.
	lblPluginMainInfo = gtk_label_new("\n\n\n\n");
	gtk_widget_set_name(lblPluginMainInfo, "lblPluginMainInfo");
	gtk_label_set_selectable(GTK_LABEL(lblPluginMainInfo), TRUE);
	gtk_misc_set_alignment(GTK_MISC(lblPluginMainInfo), 0.0f, 0.0f);
	gtk_widget_show(lblPluginMainInfo);
	g_object_set_data_full(G_OBJECT(m_Window), "lblPluginMainInfo",
			       g_object_ref(lblPluginMainInfo), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxPluginMainInfo), lblPluginMainInfo, TRUE, FALSE, 0);
}


void PluginManagerWindow::populatePluginList(void)
{
	// Check if the list model is already created.
	// If it is, clear it; if not, create a new one.
	if (lmPluginList)
		gtk_list_store_clear(lmPluginList);
	else
		lmPluginList = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	
	// Set the view model of the treeview.
	gtk_tree_view_set_model(GTK_TREE_VIEW(lstPluginList), GTK_TREE_MODEL(lmPluginList));
	
	GtkTreeViewColumn *colPlugin;
	
	// Delete any existing columns.
	do
	{
		colPlugin = gtk_tree_view_get_column(GTK_TREE_VIEW(lstPluginList), 0);
		if (colPlugin)
			gtk_tree_view_remove_column(GTK_TREE_VIEW(lstPluginList), colPlugin);
	} while (colPlugin != NULL);
	
	// Create the renderer and columns.
	GtkCellRenderer *textRenderer = gtk_cell_renderer_text_new();
	colPlugin = gtk_tree_view_column_new_with_attributes("Plugin", textRenderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstPluginList), colPlugin);
	
	// Add all plugins to the treeview.
	vector<MDP_t*>::iterator curPlugin;
	for (curPlugin = PluginMgr::vRenderPlugins.begin();
	     curPlugin != PluginMgr::vRenderPlugins.end(); curPlugin++)
	{
		GtkTreeIter iter;
		
		gtk_list_store_append(lmPluginList, &iter);
		
		MDP_t *plugin = (*curPlugin);
		if (plugin->desc && plugin->desc->name)
		{
			gtk_list_store_set(GTK_LIST_STORE(lmPluginList), &iter, 0, plugin->desc->name, 1, plugin, -1);
		}
		else
		{
			// No description or name.
			// TODO: For external plugins, indicate the external file.
			char tmp[64];
			sprintf(tmp, "[No name: 0x%08X]", (unsigned int)plugin);
			gtk_list_store_set(GTK_LIST_STORE(lmPluginList), &iter, 0, tmp, 1, plugin, -1);
		}
	}
}


void PluginManagerWindow::lstPluginList_cursor_changed_STATIC(GtkTreeView *tree_view, gpointer user_data)
{
	reinterpret_cast<PluginManagerWindow*>(user_data)->lstPluginList_cursor_changed(tree_view);
}


void PluginManagerWindow::lstPluginList_cursor_changed(GtkTreeView *tree_view)
{
	// Check which plugin is clicked.
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstPluginList));
	
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(selection, (GtkTreeModel**)(&lmPluginList), &iter))
	{
		// No plugin selected.
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "No plugin selected.\n\n\n\n");
		return;
	}
	
	// Found a selected plugin.
	GValue gVal = { 0 };
	MDP_t *plugin;
	
	gtk_tree_model_get_value(GTK_TREE_MODEL(lmPluginList), &iter, 1, &gVal);
	plugin = (MDP_t*)g_value_peek_pointer(&gVal);
	g_value_unset(&gVal);
	
	// Get the plugin information.
	if (!plugin)
	{
		// Invalid plugin.
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "Invalid plugin selected.\n\n\n\n");
		return;
	}
	
	if (!plugin->desc)
	{
		gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), "This plugin does not have a valid description field.\n\n\n\n");
		return;
	}
	
	// Fill in the descriptions.
	MDP_Desc_t *desc = plugin->desc;
	stringstream ssMainDesc;
	int lines = 3;			// Name, MDP Author, and License are always printed.
	const int linesReserved = 5;	// Number of lines reserved.
	
	ssMainDesc << "Name: " << (desc->name ? string(desc->name) : "(none)") << endl
		   << "MDP Author: " + (desc->author_mdp ? string(desc->author_mdp) : "(none)") << endl;
	if (desc->author_orig)
	{
		ssMainDesc << "Original Author: " << string(desc->author_orig) << endl;
		lines++;
	}
	if (desc->website)
	{
		ssMainDesc << "Website: " << string(desc->website) << endl;
		lines++;
	}
	ssMainDesc << "License: " + (desc->license ? string(desc->license) : "(none)");
	
	// Linebreaks needed.
	const int linesNeeded = linesReserved - lines;
	for (int i = 0; i < linesNeeded; i++)
	{
		ssMainDesc << endl;
	}
	
	gtk_label_set_text(GTK_LABEL(lblPluginMainInfo), ssMainDesc.str().c_str());
}
