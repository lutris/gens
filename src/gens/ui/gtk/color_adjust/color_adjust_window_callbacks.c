/***************************************************************************
 * Gens: (GTK+) Color Adjustment Window - Callback Functions.              *
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


#include <string.h>
#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"

#include "gtk-misc.h"


/**
 * Window is closed.
 */
gboolean on_color_adjust_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
	return FALSE;
}


/**
 * Cancel
 */
void on_button_ca_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
}


/**
 * Apply
 */
void on_button_ca_Apply_clicked(GtkButton *button, gpointer user_data)
{
	CA_Save();
}


/**
 * Save
 */
void on_button_ca_Save_clicked(GtkButton *button, gpointer user_data)
{
	CA_Save();
	gtk_widget_destroy(color_adjust_window);
	color_adjust_window = NULL;
}
