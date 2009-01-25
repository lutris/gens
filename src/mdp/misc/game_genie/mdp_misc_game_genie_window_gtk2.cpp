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

#include "mdp_misc_game_genie_window.hpp"
#include "mdp_misc_game_genie_plugin.h"
#include <gtk/gtk.h>

GG_window* GG_window::m_Instance = NULL;


GG_window::GG_window(MDP_Host_t *host_srv)
{
	// Create the Game Genie window.
	m_Window = gtk_dialog_new();
	gtk_widget_set_name(GTK_WIDGET(m_Window), "GG_window");
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 0);
	gtk_window_set_title(GTK_WINDOW(m_Window), "Game Genie");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), true);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), false);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(m_Window), "GG_window", m_Window);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)m_Window, "delete_event",
			 G_CALLBACK(this->GTK_closed), this);
	g_signal_connect((gpointer)m_Window, "destroy_event",
			 G_CALLBACK(this->GTK_closed), this);
	
	// Show the window.
	gtk_widget_show_all(GTK_WIDGET(m_Window));
	
	// Register the window with MDP Host Services.
	m_host_srv = host_srv;
	m_host_srv->window_register(&mdp, m_Window);
}


GG_window::~GG_window()
{
	// Delete the window.
	if (GTK_IS_WINDOW(m_Window))
	{
		gtk_widget_destroy(GTK_WIDGET(m_Window));
		
		// Unregister the window from MDP Host Services.
		m_host_srv->window_unregister(&mdp, m_Window);
	}
	
	m_Window = NULL;
	m_Instance = NULL;
}


void GG_window::setFocus(void)
{
	if (m_Window)
		gtk_widget_grab_focus(GTK_WIDGET(m_Window));
}

void GG_window::setModal(void *parent)
{
	if (m_Window)
		gtk_window_set_transient_for(GTK_WINDOW(m_Window), GTK_WINDOW(parent));
}

gboolean GG_window::GTK_closed(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	// Delete the window.
	delete reinterpret_cast<GG_window*>(user_data);
	return false;
}
