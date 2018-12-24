/***************************************************************************
 * Gens: (GTK+) Main Window - Callback Functions.                          *
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

#include "emulator/g_main.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window.h"
#include "gens_window_sync.hpp"

#include "util/file/rom.hpp"

#include "ui/gens_ui.hpp"
#include "ui/gtk/gtk-uri.h"
#include "ui/gtk/gtk-compat.h"

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_file.h"

// File management functions.
#include "util/file/file.hpp"

// Video and Audio Handler.
#include "video/vdraw.h"
#include "audio/audio.h"

// SDL input event handler.
#include "input/input_sdl_events.hpp"

// C++ includes
#include <string>
using std::string;


/**
 * Window is closed.
 */
gboolean on_gens_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	close_gens();
	
	// True tells GTK+ not to close the window. This is needed
	// in order to prevent an X11 error from occurring due to
	// the embedded SDL window.
	return true;
}


/** Drag & Drop callbacks **/


void gens_window_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
				    GtkSelectionData *selection_data, guint target_type, guint time,
				    gpointer data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(x);
	GSFT_UNUSED_PARAMETER(y);
	GSFT_UNUSED_PARAMETER(target_type);
	GSFT_UNUSED_PARAMETER(data);
	
	if (selection_data == NULL || gtk_selection_data_get_length(selection_data) == 0)
	{
		// No selection data.
		gtk_drag_finish(context, false, false, time);
		return;
	}
	
	gboolean dnd_success = true;
	
	gchar *sel_text = (gchar*)gtk_selection_data_get_data(selection_data);
	if (!sel_text)
	{
		// Selection data was not text.
		gtk_drag_finish(context, false, false, time);
		return;
	}
	
	string filename = string(sel_text);
	
	if (filename.length() >= 8 && filename.substr(0, 8) == "file:///")
	{
		// "file:///" prefix. Remove the prefix.
		filename = filename.substr(7);
	}
	else if (filename.length() >= 7 && filename.substr(0, 7) == "file://")
	{
		// "file://" prefix. Remove the prefix.
		filename = filename.substr(6);
	}
	else if (filename.length() >= 6 && filename.substr(0, 6) == "file:/")
	{
		// "file:/" prefix. Remove the prefix.
		filename = filename.substr(5);
	}
	else if (filename.length() >= 10 && filename.substr(0, 9) == "desktop:/")
	{
		// "desktop:/" prefix. Remove the prefix and prepend the user's desktop directory.
		filename = string(getenv("HOME")) + "/Desktop/" + filename.substr(9);
	}
	
	// Unescape the URI.
	char* unescaped = gens_g_uri_unescape_string(filename.c_str(), NULL);
	filename = string(unescaped);
	g_free(unescaped);
	
	// Check that the file actually exists.
	if (gsft_file_exists(filename.c_str()))
	{
		// File exists. Open it as a ROM image.
		ROM::openROM(filename);
		Sync_Gens_Window();
	}
	else
	{
		// File does not exist.
		dnd_success = false;
	}
	
	gtk_drag_finish(context, dnd_success, false, time);
}


gboolean gens_window_drag_drop(GtkWidget *widget, GdkDragContext *context,
			       gint x, gint y, guint time, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(x);
	GSFT_UNUSED_PARAMETER(y);
	GSFT_UNUSED_PARAMETER(user_data);
	
	if (context->targets)
	{
		GdkAtom target_type = GDK_POINTER_TO_ATOM(g_list_nth_data(context->targets, 0));
		gtk_drag_get_data(widget, context, target_type, time);
		return true;
	}
	return false;
}


/** Focus callbacks **/


/**
 * gens_window_focus_in(): Gens window has received focus.
 * @param widget GTK+ widget.
 * @param event GDK event.
 * @param user_data User data.
 * @return True to stop other handlers from being invoked; false to allow the event to propagate.
 */
gboolean gens_window_focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	if (vdraw_get_fullscreen())
		return false;
	
	Settings.Active = 1;
	
	// Reset the SDL key modifier state.
	input_sdl_reset_modifiers();
	
	return false;
}


/**
 * gens_window_focus_in(): Gens window has lost focus.
 * @param widget GTK+ widget.
 * @param event GDK event.
 * @param user_data User data.
 * @return True to stop other handlers from being invoked; false to allow the event to propagate.
 */
gboolean gens_window_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	if (vdraw_get_fullscreen())
		return false;
	
	if (Auto_Pause && Settings.Active)
	{
		Settings.Active = 0;
		audio_clear_sound_buffer();
	}
	
	// Reset the SDL key modifier state.
	input_sdl_reset_modifiers();
	
	return false;
}


/**
 * gens_window_sdlsock_expose(): SDL socket expose event.
 * @param widget GTK+ widget.
 * @param event GDK expose event.
 * @param user_data User data.
 * @return TRUE to stop other handlers from being invoked; FALSE to allow the event to propagate.
 */
gboolean gens_window_sdlsock_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	GensUI::wakeup();
	return false;
}


/**
 * gens_window_sdlsock_button_press(): SDL socket mouse button event.
 * @param widget GTK+ widget.
 * @param event GDK button event.
 * @param user_data User data.
 * @return TRUE to stop other handlers from being invoked; FALSE to allow the event to propagate.
 */
gboolean gens_window_sdlsock_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	
	if (!GTK_IS_MENU(gens_menu_bar))
	{
		// gens_menu_bar is not a GtkMenu.
		// Don't do anything.
		return false;
	}
	
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		// Right-click.
		if (!vdraw_get_fullscreen() && !Settings.showMenuBar)
		{
			// Show the GTK+ menu.
			gtk_menu_popup(GTK_MENU(gens_menu_bar), NULL, NULL, NULL, NULL,
			       	event->button, event->time);
			return true;
		}
	}
	
	// Unhandled event
	return false;
}


/**
 * gens_window_window_state_event(): Window state event.
 * @param wigdet GTK+ widget.
 * @param event GDK window state event.
 * @param user_data
 * @return TRUE to stop other handlers from being invoked; FALSE to allow the event to propagate.
 */
gboolean gens_window_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(user_data);
	
	if (widget != gens_window)
		return FALSE;
	
	if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
	{
		// Iconification state was changed.
		if (!(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED))
		{
			// Window was restored.
			// Call fsRestore() for Alt-Tab processing.
			GensUI::fsRestore(GensUI::FSMINIMIZE_ALTTAB);
		}
	}
	
	return FALSE;
}
