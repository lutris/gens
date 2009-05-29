/***************************************************************************
 * Gens: Message Logging.                                                  *
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

#include "log_msg.h"

#if defined(GENS_UI_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
#elif defined(GENS_UI_GTK)
	#include <gtk/gtk.h>
#else
	#warning No UI defined. log_msgbox() will do nothing.
#endif

// Needed for the parent window.
#include "gens/gens_window.h"


/**
 * log_msgbox(): Function used to show a message box for messages with level LOG_MSG_LEVEL_CRITICAL.
 * @param msg Message.
 * @param title Title.
 */
void log_msgbox(const char* msg, const char* title)
{
	#if defined(GENS_UI_WIN32)
		MessageBox(gens_window, msg, title, MB_ICONSTOP);
	#elif defined(GENS_UI_GTK)
		// Make sure GTK+ is initialized.
		if (!gtk_init_check(NULL, NULL))
		{
			// Could not initialize GTK+. Print the message to the console.
			return;
		}
		
		// GTK+ initialized.
		GtkWidget *msgbox = gtk_message_dialog_new(
					GTK_WINDOW(gens_window),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s", msg);
		gtk_window_set_title(GTK_WINDOW(msgbox), title);
		gtk_dialog_run(GTK_DIALOG(msgbox));
		gtk_widget_destroy(msgbox);
	#endif
}
