/***************************************************************************
 * Gens: (GTK+) About Window.                                              *
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

#ifndef GENS_GTK_ABOUT_WINDOW_HPP
#define GENS_GTK_ABOUT_WINDOW_HPP

#ifdef __cplusplus

#include <gtk/gtk.h>

class AboutWindow
{
	public:
		static AboutWindow* Instance(GtkWidget *parent = NULL);
		
		void setFocus(void);
		void setModal(GtkWidget *parent);
		
		gboolean close(void);
		
		gboolean iceTime(void);
	
	protected:
		AboutWindow();
		~AboutWindow();
		
		GtkWidget *m_Window;
		GtkAccelGroup *m_AccelTable;
		
		GtkWidget *m_imgGensLogo;
		
		// Static functions required for GTK+ callbacks.
		static gboolean GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
		static void GTK_OK(GtkButton *button, gpointer user_data);
		
		unsigned short ax, bx, cx;
		static gboolean GTK_iceTime(gpointer user_data);
		void updateIce(void);
	
	private:
		static AboutWindow *m_Instance;
};

#endif

#endif /* GENS_GTK_ABOUT_WINDOW_HPP */
