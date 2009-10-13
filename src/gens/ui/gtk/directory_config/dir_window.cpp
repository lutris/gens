/***************************************************************************
 * Gens: (GTK+) Directory Configuration Window.                            *
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

#include "dir_window.hpp"
#include "ui/common/dir_window_common.h"
#include "gens/gens_window.h"

// Plugin Manager.
#include "plugins/pluginmgr.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// C includes.
#include <string.h>

// C++ includes.
#include <string>
#include <vector>
#include <list>
using std::string;
using std::vector;
using std::list;

// GTK+ includes.
#include <gtk/gtk.h>

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_file.h"
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

// Gens includes.
#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "util/file/file.hpp"


// Window.
GtkWidget *dir_window = NULL;

// Widgets.
static GtkWidget	*btnCancel, *btnApply, *btnSave;

// Directory widgets.
typedef struct _dir_plugin_t
{
	GtkWidget	*txt;
	string		title;
	bool		is_plugin;
	int		id;
} dir_widget_t;
static vector<dir_widget_t> vectDirs;

// Widget creation functions.
static GtkWidget*	dir_window_create_dir_widgets(const char* title, GtkWidget *table, int row, int dir_id);

// Directory configuration load/save functions.
static void	dir_window_init(void);
static void	dir_window_save(void);

// Callbacks.
static gboolean	dir_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	dir_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	dir_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data);
static void	dir_window_callback_textbox_changed(GtkEditable *editable, gpointer user_data);


/**
 * dir_window_show(): Show the Directory Configuration window.
 */
void dir_window_show(void)
{
	if (dir_window)
	{
		// Directory Configuration window is already visible. Set focus.
		gtk_widget_grab_focus(dir_window);
		return;
	}
	
	// Create the window.
	dir_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(dir_window), 4);
	gtk_window_set_title(GTK_WINDOW(dir_window), "Configure Directories");
	gtk_window_set_position(GTK_WINDOW(dir_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dir_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(dir_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(dir_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)(dir_window), "delete_event",
			 G_CALLBACK(dir_window_callback_close), NULL);
	g_signal_connect((gpointer)(dir_window), "destroy_event",
			 G_CALLBACK(dir_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(dir_window), "response",
			 G_CALLBACK(dir_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(dir_window));
	gtk_widget_show(vboxDialog);
	
	// Create the internal directory entry frame.
	GtkWidget *fraInternalDirs = gtk_frame_new("<b><i>Gens/GS Directories</i></b>");
	gtk_frame_set_shadow_type(GTK_FRAME(fraInternalDirs), GTK_SHADOW_ETCHED_IN);
	gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraInternalDirs))), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(fraInternalDirs), 4);
	gtk_widget_show(fraInternalDirs);
	gtk_box_pack_start(GTK_BOX(vboxDialog), fraInternalDirs, TRUE, TRUE, 0);
	
	// Create the table for the internal directories.
	GtkWidget *tblInternalDirs = gtk_table_new(DIR_WINDOW_ENTRIES_COUNT, 3, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(tblInternalDirs), 8);
	gtk_table_set_row_spacings(GTK_TABLE(tblInternalDirs), 4);
	gtk_table_set_col_spacings(GTK_TABLE(tblInternalDirs), 8);
	gtk_widget_show(tblInternalDirs);
	gtk_container_add(GTK_CONTAINER(fraInternalDirs), tblInternalDirs);
	
	// Initialize the directory widget vector.
	vectDirs.clear();
	vectDirs.reserve(DIR_WINDOW_ENTRIES_COUNT + PluginMgr::lstDirectories.size());
	
	// Create all internal directory entry widgets.
	dir_widget_t dir_widget;
	dir_widget.is_plugin = false;
	for (unsigned int dir = 0; dir < DIR_WINDOW_ENTRIES_COUNT; dir++)
	{
		dir_widget.id = dir;
		dir_widget.title = string(dir_window_entries[dir].title);
		
		dir_widget.txt = dir_window_create_dir_widgets(
					dir_window_entries[dir].title,
					tblInternalDirs, dir, vectDirs.size());
		vectDirs.push_back(dir_widget);
	}
	
	// If any plugin directories exist, create the plugin directory entry frame.
	if (!PluginMgr::lstDirectories.empty())
	{
		// Create the plugin directory entry frame.
		GtkWidget *fraPluginDirs = gtk_frame_new("<b><i>Plugin Directories</i></b>");
		gtk_frame_set_shadow_type(GTK_FRAME(fraPluginDirs), GTK_SHADOW_ETCHED_IN);
		gtk_label_set_use_markup(GTK_LABEL(gtk_frame_get_label_widget(GTK_FRAME(fraPluginDirs))), TRUE);
		gtk_container_set_border_width(GTK_CONTAINER(fraPluginDirs), 4);
		gtk_widget_show(fraPluginDirs);
		gtk_box_pack_start(GTK_BOX(vboxDialog), fraPluginDirs, TRUE, TRUE, 0);
		
		// Create the table for the plugin directories.
		GtkWidget *tblPluginDirs = gtk_table_new(PluginMgr::lstDirectories.size(), 3, FALSE);
		gtk_container_set_border_width(GTK_CONTAINER(tblPluginDirs), 8);
		gtk_table_set_row_spacings(GTK_TABLE(tblPluginDirs), 4);
		gtk_table_set_col_spacings(GTK_TABLE(tblPluginDirs), 8);
		gtk_widget_show(tblPluginDirs);
		gtk_container_add(GTK_CONTAINER(fraPluginDirs), tblPluginDirs);
		
		// Create all plugin directory entry widgets.
		unsigned int dir = 0;
		dir_widget.is_plugin = true;
		
		for (list<mdpDir_t>::iterator iter = PluginMgr::lstDirectories.begin();
		     iter != PluginMgr::lstDirectories.end(); iter++, dir++)
		{
			dir_widget.id = (*iter).id;
			dir_widget.title = (*iter).name;
			
			dir_widget.txt = dir_window_create_dir_widgets(
						(*iter).name.c_str(),
						tblPluginDirs, dir, vectDirs.size());
			vectDirs.push_back(dir_widget);
		}
	}
	
	// Create the dialog buttons.
	btnApply  = gtk_dialog_add_button(GTK_DIALOG(dir_window), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
	btnCancel = gtk_dialog_add_button(GTK_DIALOG(dir_window), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	btnSave   = gtk_dialog_add_button(GTK_DIALOG(dir_window), GTK_STOCK_SAVE, GTK_RESPONSE_OK);
	
#if (GTK_MAJOR_VERSION > 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION >= 6))
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dir_window),
						GTK_RESPONSE_OK,
						GTK_RESPONSE_CANCEL,
						GTK_RESPONSE_APPLY,
						-1);
#endif
	
	// Initialize the directory entries.
	dir_window_init();
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(dir_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(dir_window);
}


/**
 * dir_window_create_dir_widgets(): Create directory widgets.
 * @param title Title of the directory.
 * @param table Table to add the directory widgets to.
 * @param row Row of the table to add the directory widget to.
 * @param dir_id Directory ID for the "Change..." callback function.
 * @return Textbox.
 */
static GtkWidget* dir_window_create_dir_widgets(const char* title, GtkWidget *table, int row, int dir_id)
{
	// Create tbe label for the directory.
	GtkWidget *lblDirectory = gtk_label_new(title);
	gtk_misc_set_alignment(GTK_MISC(lblDirectory), 0.0f, 0.5f);
	gtk_widget_show(lblDirectory);
	gtk_table_attach(GTK_TABLE(table), lblDirectory,
			 0, 1, row, row + 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	
	// Create the textbox for the directory.
	GtkWidget *txtDirectory = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(txtDirectory), GENS_PATH_MAX - 1);
	gtk_widget_set_size_request(txtDirectory, 256, -1);
	gtk_widget_show(txtDirectory);
	gtk_table_attach(GTK_TABLE(table), txtDirectory,
			 1, 2, row, row + 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	g_signal_connect((gpointer)txtDirectory, "changed",
			 G_CALLBACK(dir_window_callback_textbox_changed), NULL);
	
	// Create the "Change" button for the directory.
	// TODO: Use an icon?
	GtkWidget *btnChange = gtk_button_new_with_label("Change...");
	gtk_widget_show(btnChange);
	gtk_table_attach(GTK_TABLE(table), btnChange,
			 2, 3, row, row + 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(0), 0, 0);
	
	// Connect the "clicked" signal for the "Change" button.
	g_signal_connect(GTK_OBJECT(btnChange), "clicked",
			 G_CALLBACK(dir_window_callback_btnChange_clicked),
			 GINT_TO_POINTER(dir_id));
	
	return txtDirectory;
}


/**
 * dir_window_close(): Close the OpenGL Resolution window.
 */
void dir_window_close(void)
{
	if (!dir_window)
		return;
	
	// Clear the directory widget vector.
	vectDirs.clear();
	
	// Destroy the window.
	gtk_widget_destroy(dir_window);
	dir_window = NULL;
}


/**
 * dir_window_init(): Initialize the Directory Configuration entries.
 */
static void dir_window_init(void)
{
	char dir_buf[GENS_PATH_MAX];
	
	for (unsigned int dir = 0; dir < vectDirs.size(); dir++)
	{
		if (!vectDirs[dir].is_plugin)
		{
			// Internal directory.
			gtk_entry_set_text(GTK_ENTRY(vectDirs[dir].txt),
					   dir_window_entries[vectDirs[dir].id].entry);
		}
		else
		{
			// Plugin directory.
			mapDirItems::iterator dirIter = PluginMgr::tblDirectories.find(vectDirs[dir].id);
			if (dirIter == PluginMgr::tblDirectories.end())
				continue;
			
			list<mdpDir_t>::iterator lstDirIter = (*dirIter).second;
			const mdpDir_t& mdpDir = *lstDirIter;
			
			// Get the directory.
			if (mdpDir.get(vectDirs[dir].id, dir_buf, sizeof(dir_buf)) == MDP_ERR_OK)
			{
				// Directory retrieved.
				gtk_entry_set_text(GTK_ENTRY(vectDirs[dir].txt), dir_buf);
			}
		}
	}
	
	// Disable the "Apply" button initially.
	gtk_widget_set_sensitive(btnApply, false);
}

/**
 * dir_window_save(): Save the Directory Configuration entries.
 */
static void dir_window_save(void)
{
	size_t len;
	char dir_buf[GENS_PATH_MAX];
	
	for (unsigned int dir = 0; dir < vectDirs.size(); dir++)
	{
		if (!vectDirs[dir].is_plugin)
		{
			// Internal directory.
			char *entry = dir_window_entries[vectDirs[dir].id].entry;
			
			// Get the entry text.
			strlcpy(entry, gtk_entry_get_text(GTK_ENTRY(vectDirs[dir].txt)),
				GENS_PATH_MAX);
			
			// Make sure the end of the directory has a slash.
			// TODO: Do this in functions that use pathnames.
			len = strlen(entry);
			if (len > 0 && entry[len-1] != GSFT_DIR_SEP_CHR)
			{
				// String needs to be less than 1 minus the max path length
				// in order to be able to append the directory separator.
				if (len < (GENS_PATH_MAX-1))
				{
					entry[len] = GSFT_DIR_SEP_CHR;
					entry[len+1] = 0x00;
				}
			}
		}
		else
		{
			// Plugin directory.
			
			// Get the entry text.
			strlcpy(dir_buf, gtk_entry_get_text(GTK_ENTRY(vectDirs[dir].txt)), sizeof(dir_buf));
			
			// Make sure the end of the directory has a slash.
			// TODO: Do this in functions that use pathnames.
			len = strlen(dir_buf);
			if (len > 0 && dir_buf[len-1] != GSFT_DIR_SEP_CHR)
			{
				// String needs to be less than 1 minus the max path length
				// in order to be able to append the directory separator.
				if (len < (GENS_PATH_MAX-1))
				{
					dir_buf[len] = GSFT_DIR_SEP_CHR;
					dir_buf[len+1] = 0x00;
				}
			}
			
			mapDirItems::iterator dirIter = PluginMgr::tblDirectories.find(vectDirs[dir].id);
			if (dirIter == PluginMgr::tblDirectories.end())
				continue;
			
			list<mdpDir_t>::iterator lstDirIter = (*dirIter).second;
			const mdpDir_t& mdpDir = *lstDirIter;
			
			// Set the directory.
			mdpDir.set(vectDirs[dir].id, dir_buf);
		}
	}
	
	// Disable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, false);
}


/**
 * dir_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean dir_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	dir_window_close();
	return FALSE;
}


/**
 * dir_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void dir_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(dialog);
	GSFT_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CANCEL:
			dir_window_close();
			break;
		case GTK_RESPONSE_APPLY:
			dir_window_save();
			break;
		case GTK_RESPONSE_OK:
			dir_window_save();
			dir_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


/**
 * dir_window_callback_btnChange_clicked(): A "Change" button was clicked.
 * @param button Button that was clicked.
 * @param user_data Directory ID for this button.
 */
static void dir_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(button);
	
	if (GPOINTER_TO_INT(user_data) < 0 || GPOINTER_TO_INT(user_data) >= vectDirs.size())
		return;
	
	dir_widget_t *dir_widget = &vectDirs[GPOINTER_TO_INT(user_data)];
	
	char title[128];
	szprintf(title, sizeof(title), "Select %s Directory", dir_widget->title.c_str());
	
	// Request a new directory.
	string new_dir = GensUI::selectDir(title, gtk_entry_get_text(GTK_ENTRY(dir_widget->txt)), dir_window);
	
	// If "Cancel" was selected, don't do anything.
	if (new_dir.empty())
		return;
	
	// Make sure the end of the directory has a slash.
	if (new_dir.at(new_dir.length() - 1) != GSFT_DIR_SEP_CHR)
		new_dir += GSFT_DIR_SEP_CHR;
	
	// Set the new directory.
	gtk_entry_set_text(GTK_ENTRY(dir_widget->txt), new_dir.c_str());
}


/**
 * dir_window_callback_textbox_changed(): One of the textboxes was changed.
 * @param editable
 * @param user_data
 */
static void dir_window_callback_textbox_changed(GtkEditable *editable, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(editable);
	GSFT_UNUSED_PARAMETER(user_data);
	
	// Enable the "Apply" button.
	gtk_widget_set_sensitive(btnApply, true);
}
