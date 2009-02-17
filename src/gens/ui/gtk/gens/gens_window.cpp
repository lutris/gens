/***************************************************************************
 * Gens: (GTK+) Main Window.                                               *
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

#include "gens_window.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

// Menus.
#include "gens_menu.hpp"
#include "ui/common/gens/gens_menu.h"

// Debug messages.
#include "macros/debug_msg.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Gens GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "emulator/g_main.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

GtkWidget *gens_window = NULL;
GtkWidget *MenuBar;


GtkAccelGroup *gens_window_accel_group;

static void create_gens_window_menubar(GtkWidget *container);


GtkWidget* create_gens_window(void)
{
	GtkWidget *vbox1;
	GtkWidget *sdlsock;
	
	// Create the icon for the Gens window, and set it as default for all windows.
	// TODO: Move this to a common C file.
	
	GList *gens_icon_list = NULL;
	static const char* const gens_icon_filename[2] =
	{
		GENS_DATADIR "/gens_32x32.png",
		GENS_DATADIR "/gens_16x16.png"
	};
	
	// Add the icons.
	for (int i = 0; i < 2; i++)
	{
		if (!g_file_test(gens_icon_filename[i], G_FILE_TEST_EXISTS))
		{
			// File not found.
			DEBUG_MSG(gens, 1, "Image file not found: %s",
				  gens_icon_filename[i]);
			continue;
		}
		
		// Load the icon.
		GError *error = NULL;
		GdkPixbuf *gens_icon = gdk_pixbuf_new_from_file(gens_icon_filename[i], &error);
		if (!gens_icon)
		{
			DEBUG_MSG(gens, 1, "Error loading image file %s: %s",
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
	
	// Create the accelerator group.
	gens_window_accel_group = gtk_accel_group_new();
	
	// Create the Gens window.
	gens_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(gens_window), 0);
	gtk_window_set_title(GTK_WINDOW(gens_window), "Gens");
	gtk_window_set_position(GTK_WINDOW(gens_window), GTK_WIN_POS_CENTER);
	
	// Layout objects.
	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vbox1, "vbox1");
	gtk_widget_show(vbox1);
	gtk_container_add(GTK_CONTAINER(gens_window), vbox1);
	GLADE_HOOKUP_OBJECT(gens_window, vbox1, "vbox1");
	
	// Create the menu bar.
	create_gens_window_menubar(vbox1);
	
	// Create the SDL socket.
	sdlsock = gtk_event_box_new();
	gtk_widget_set_name(sdlsock, "sdlsock");
	gtk_box_pack_end(GTK_BOX(lookup_widget(gens_window, "vbox1")), sdlsock, 0, 0, 0);
	GLADE_HOOKUP_OBJECT(gens_window, sdlsock, "sdlsock");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(gens_window), gens_window_accel_group);
	
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
	const GtkTargetEntry target_list[] =
	{
		{"text/plain", 0, 0},
		{"text/uri-list", 0, 1},
	};
	
	gtk_drag_dest_set
	(
		sdlsock,
		(GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT),
		target_list,
		G_N_ELEMENTS(target_list),
		(GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_PRIVATE | GDK_ACTION_ASK)
	);
	
	// Set drag & drop callbacks.
	g_signal_connect(sdlsock, "drag-data-received",
			 G_CALLBACK(gens_window_drag_data_received), NULL);
	g_signal_connect(sdlsock, "drag-drop",
			 G_CALLBACK(gens_window_drag_drop), NULL);
	
	return gens_window;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 * @param container Container for the menu bar.
 */
static void create_gens_window_menubar(GtkWidget *container)
{
	MenuBar = gtk_menu_bar_new();
	gtk_widget_set_name(MenuBar, "MenuBar");
	gtk_widget_show(MenuBar);
	gtk_container_add(GTK_CONTAINER(container), MenuBar);
	
	// Parse the menus.
	gens_menu_parse(&gmiMain[0], MenuBar, gens_window_accel_group);
}
