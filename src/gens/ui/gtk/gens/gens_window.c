/***************************************************************************
 * Gens: (GTK+) Main Window.                                               *
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

#include "gens_window.h"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include "emulator/g_main.hpp"

// Menus.
#include "gens_menu.hpp"
#include "ui/common/gens/gens_menu.h"
#include "video/vdraw.h"

// Message logging.
#include "macros/log_msg.h"

// GTK+ includes.
#include <gdkconfig.h>
#include <gtk/gtk.h>

// MacOS X menu bar integration.
#ifdef GDK_WINDOWING_QUARTZ
#include <ige-mac-integration.h>
#endif

// Widgets.
GtkWidget *gens_window = NULL;
GtkWidget *gens_window_sdlsock = NULL;
GtkWidget *gens_menu_bar = NULL;
static GtkWidget *gens_vbox_main = NULL;


/**
 * gens_window_create(): Create the Gens window.
 */
void gens_window_create(void)
{
	// Create the icon for the Gens window, and set it as default for all windows.
	// TODO: Move this to a common C file.
	
	// TODO: Use the standard icon directory layout.
	GList *gens_icon_list = NULL;
	static const char* const gens_icon_filename[3] =
	{
		GENS_DATADIR "/gensgs_48x48.png",
		GENS_DATADIR "/gensgs_32x32.png",
		GENS_DATADIR "/gensgs_16x16.png"
	};
	
	// Add the icons.
	for (int i = 0; i < 3; i++)
	{
		if (!g_file_test(gens_icon_filename[i], G_FILE_TEST_EXISTS))
		{
			// File not found.
			LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
				"Image file not found: %s",
				gens_icon_filename[i]);
			continue;
		}
		
		// Load the icon.
		GError *error = NULL;
		GdkPixbuf *gens_icon = gdk_pixbuf_new_from_file(gens_icon_filename[i], &error);
		if (!gens_icon)
		{
			LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
				"Error loading image file %s: %s",
				gens_icon_filename[i], error->message);
			g_error_free(error);
			continue;
		}
		
		// Add the icon to the icon list.
		gens_icon_list = g_list_append(gens_icon_list, gens_icon);
	}
	
	// Set the icon list as the default icon list.
	gtk_window_set_default_icon_list(gens_icon_list);
	
	// Free the icon list.
	g_list_free(gens_icon_list);
	
	// Create the Gens window.
	gens_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(gens_window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(gens_window), 0);
	gtk_window_set_position(GTK_WINDOW(gens_window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(gens_window), "Gens");
	
	// Create the main VBox.
	gens_vbox_main = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(gens_vbox_main);
	gtk_container_add(GTK_CONTAINER(gens_window), gens_vbox_main);
	
	// Create the HBox for the SDL "socket".
	GtkWidget *hboxSocket = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxSocket);
	gtk_box_pack_end(GTK_BOX(gens_vbox_main), hboxSocket, TRUE, TRUE, 0);
	
	// Create the SDL "socket".
	gens_window_sdlsock = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX(hboxSocket), gens_window_sdlsock, TRUE, FALSE, 0);
	
	// Set the background color of the SDL "socket" to black.
	GdkColor bg = {0, 0, 0, 0};
	gtk_widget_modify_bg(gens_window_sdlsock, GTK_STATE_NORMAL, &bg);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)gens_window, "delete-event",
			 G_CALLBACK(on_gens_window_close), NULL);
	g_signal_connect((gpointer)gens_window, "destroy-event",
			 G_CALLBACK(on_gens_window_close), NULL);
	
	// Callbacks for Auto Pause.
	g_signal_connect((gpointer)gens_window, "focus-in-event",
			 G_CALLBACK(gens_window_focus_in), NULL);
	g_signal_connect((gpointer)gens_window, "focus-out-event",
			 G_CALLBACK(gens_window_focus_out), NULL);
	
	// Enable drag & drop for ROM loading.
	static const GtkTargetEntry target_list[] =
	{
		{"text/plain", 0, 0},
		{"text/uri-list", 0, 1},
	};
	
	gtk_drag_dest_set
	(
		gens_window_sdlsock,
		(GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT),
		target_list,
		G_N_ELEMENTS(target_list),
		(GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_PRIVATE | GDK_ACTION_ASK)
	);
	
	// Set drag & drop callbacks.
	g_signal_connect(gens_window_sdlsock, "drag-data-received",
			 G_CALLBACK(gens_window_drag_data_received), NULL);
	g_signal_connect(gens_window_sdlsock, "drag-drop",
			 G_CALLBACK(gens_window_drag_drop), NULL);
	
	// Expose event.
	g_signal_connect(gens_window_sdlsock, "expose-event",
			 G_CALLBACK(gens_window_sdlsock_expose), NULL);
	
	// Button Press event.
	g_signal_connect(gens_window_sdlsock, "button-press-event",
			 G_CALLBACK(gens_window_sdlsock_button_press), NULL);
	
	// Window State event.
	g_signal_connect(gens_window, "window-state-event",
			 G_CALLBACK(gens_window_window_state_event), NULL);
	
	// Window is active.
	Settings.Active = 1;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 */
void gens_window_create_menubar(void)
{
	if (gens_menu_bar != NULL)
	{
		// Menu bar already exists. Delete it.
		gtk_widget_destroy(gens_menu_bar);
		gens_menu_clear();
	}
	
	if (!vdraw_get_fullscreen() && Settings.showMenuBar)
	{
		// Create a menu bar.
		gens_menu_bar = gtk_menu_bar_new();
		gtk_widget_show(gens_menu_bar);
		gtk_box_pack_start(GTK_BOX(gens_vbox_main), gens_menu_bar, FALSE, FALSE, 0);
	}
	else
	{
		// Create a popup menu.
		gens_menu_bar = gtk_menu_new();
	}
	
	// Accelerator Group.
	static GtkAccelGroup *gens_menu_accel_group = NULL;
	if (gens_menu_accel_group != NULL)
	{
		// Delete the accelerator group.
		gtk_window_remove_accel_group(GTK_WINDOW(gens_window), gens_menu_accel_group);
		g_object_unref(gens_menu_accel_group);
	}
	
	// Create an accelerator group.
	gens_menu_accel_group = gtk_accel_group_new();
	
	// Parse the menus.
	gens_menu_parse(&gmiMain[0], gens_menu_bar, gens_menu_accel_group);
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(gens_window), gens_menu_accel_group);
	
	// Synchronize the menus.
	Sync_Gens_Window();
	
#ifdef GDK_WINDOWING_QUARTZ
	// Set the menu bar as the MacOS X menu bar.
	if (!vdraw_get_fullscreen() && Settings.showMenuBar)
		ige_mac_menu_set_menu_bar(GTK_MENU_SHELL(gens_menu_bar));
#endif
}
