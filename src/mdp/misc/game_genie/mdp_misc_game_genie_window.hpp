/***************************************************************************
 * Gens: [MDP] Game Genie. (Window Code)                                   *
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

#ifndef _MDP_MISC_GAME_GENIE_WINDOW_HPP
#define _MDP_MISC_GAME_GENIE_WINDOW_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_UI_GTK
#include <gtk/gtk.h>
#endif

#include "mdp/mdp_host.h"

class GG_window
{
	public:
		static GG_window* Instance(void *parent, MDP_Host_t *host_srv)
		{
			if (!m_Instance)
			{
				// Instance doesn't exist. Initialize the main instance.
				m_Instance = new GG_window(host_srv);
			}
			else
			{
				// Instance already exists. Set focus.
				m_Instance->setFocus();
			}
			
			// Set modality of the window.
			if (parent)
				m_Instance->setModal(parent);
			
			// Return the window instance.
			return m_Instance;
		}
		
		void setFocus(void);
		void setModal(void *parent);
	
	protected:
		GG_window(MDP_Host_t *host_srv);
		~GG_window();
		
		#ifdef GENS_UI_GTK
			static gboolean GTK_closed(GtkWidget *widget, GdkEvent *event, gpointer user_data);
		#endif
		
		static GG_window* m_Instance;
		
		void *m_Window;
		MDP_Host_t *m_host_srv;
};

#endif /* _MDP_MISC_GAME_GENIE_WINDOW_HPP */
