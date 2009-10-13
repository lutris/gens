/***************************************************************************
 * Gens: (GTK+) About Window.                                              *
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

#include "about_window.hpp"
#include "ui/common/about_window_data.h"
#include "gens/gens_window.h"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// TODO: Eliminate gtk-misc.h.
#include "gtk-misc.h"

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_szprintf.h"


// Window.
GtkWidget *about_window = NULL;

// Widgets.
static GtkWidget	*imgGensLogo;
static GdkPixbuf	*pbufIce;

// Callbacks.
static gboolean	about_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	about_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);

// ice variables and functions.
static uint8_t	ax, bx, cx;
static void	about_window_update_ice(void);
static gboolean	about_window_callback_iceTimer(gpointer data);


/**
 * about_window_show(): Show the About window.
 */
void about_window_show(void)
{
	if (about_window)
	{
		// About window is already visible. Set focus.
		gtk_widget_grab_focus(about_window);
		return;
	}
	
	// Create the window.
	about_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(about_window), 0);
	gtk_window_set_title(GTK_WINDOW(about_window), "About " GENS_APPNAME);
	gtk_window_set_position(GTK_WINDOW(about_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(about_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(about_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(about_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)about_window, "delete_event",
			 G_CALLBACK(about_window_callback_close), NULL);
	g_signal_connect((gpointer)about_window, "destroy_event",
			 G_CALLBACK(about_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)(about_window), "response",
			 G_CALLBACK(about_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(about_window));
	gtk_widget_show(vboxDialog);
	
	// Create the HBox for the logo and the program information.
	GtkWidget *hboxLogo = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxLogo);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxLogo, TRUE, TRUE, 0);
	
	cx = 0;
	pbufIce = NULL;
	if (ice != 3)
	{
		// Gens logo
		imgGensLogo = create_pixmap("gens_small.png");
	}
	else
	{
		ax = 0; bx = 0; cx = 1;
		pbufIce = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 0120, 0120);
		imgGensLogo = gtk_image_new_from_pixbuf(pbufIce);
		g_timeout_add(100, about_window_callback_iceTimer, NULL);
		about_window_update_ice();
	}
	
	gtk_widget_show(imgGensLogo);
	gtk_box_pack_start(GTK_BOX(hboxLogo), imgGensLogo, TRUE, TRUE, 0);
	
	// Create the version string.
	char buf[256];
	szprintf(buf, sizeof(buf), "<b><i>%s</i></b>\n<small>\n</small>%s",
		 about_window_title, about_window_description);
	
	// Version information.
	GtkWidget *lblVersion = gtk_label_new(buf);
	gtk_misc_set_padding(GTK_MISC(lblVersion), 8, 8);
	gtk_label_set_use_markup(GTK_LABEL(lblVersion), TRUE);
	gtk_label_set_justify(GTK_LABEL(lblVersion), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblVersion);
	gtk_box_pack_start(GTK_BOX(hboxLogo), lblVersion, FALSE, FALSE, 0);
	
	// Create a set of tabs for the main information.
	GtkWidget *tabInfo = gtk_notebook_new();
	gtk_container_set_border_width(GTK_CONTAINER(tabInfo), 4);
	gtk_widget_show(tabInfo);
	gtk_box_pack_start(GTK_BOX(vboxDialog), tabInfo, FALSE, FALSE, 0);
	
	// Label for the copyright frame tab.
	GtkWidget *lblCopyrightTab = gtk_label_new_with_mnemonic("_Copyrights");
	gtk_widget_show(lblCopyrightTab);
	
	// Copyright frame.
	GtkWidget *fraCopyright = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(fraCopyright), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(fraCopyright), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(fraCopyright);
	gtk_notebook_append_page(GTK_NOTEBOOK(tabInfo), fraCopyright, lblCopyrightTab);
	
	// Copyright label
	GtkWidget *lblCopyright = gtk_label_new(about_window_copyright);
	gtk_misc_set_padding(GTK_MISC(lblCopyright), 8, 8);
	gtk_misc_set_alignment(GTK_MISC(lblCopyright), 0.0f, 0.0f);
	gtk_widget_show(lblCopyright);
	gtk_container_add(GTK_CONTAINER(fraCopyright), lblCopyright);
	
	// Label for the Included Libraries tab.
	GtkWidget *lblIncludedLibsTab = gtk_label_new_with_mnemonic("Included _Libraries");
	gtk_widget_show(lblIncludedLibsTab);
	
	// Included Libraries frame.
	GtkWidget *fraIncludedLibs = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(fraIncludedLibs), 4);
	gtk_frame_set_shadow_type(GTK_FRAME(fraIncludedLibs), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(fraIncludedLibs);
	gtk_notebook_append_page(GTK_NOTEBOOK(tabInfo), fraIncludedLibs, lblIncludedLibsTab);
	
	// Included Libraries label
	GtkWidget *lblIncludedLibs = gtk_label_new(about_window_included_libs);
	gtk_misc_set_padding(GTK_MISC(lblIncludedLibs), 8, 8);
	gtk_misc_set_alignment(GTK_MISC(lblIncludedLibs), 0.0f, 0.0f);
	gtk_widget_show(lblIncludedLibs);
	gtk_container_add(GTK_CONTAINER(fraIncludedLibs), lblIncludedLibs);
	
	// Create the dialog button.
	GtkWidget *btnClose = gtk_dialog_add_button(GTK_DIALOG(about_window),
						    GTK_STOCK_CLOSE,
						    GTK_RESPONSE_CLOSE);
	
	// Set focus to the close button.
	gtk_widget_grab_focus(btnClose);
	
	// Set the window as transient to the main application window.
	gtk_window_set_transient_for(GTK_WINDOW(about_window), GTK_WINDOW(gens_window));
	
	// Show the window.
	gtk_widget_show_all(about_window);
}


/**
 * about_window_close(): Close the About window.
 */
void about_window_close(void)
{
	if (!about_window)
		return;
	
	// Destroy the window.
	gtk_widget_destroy(about_window);
	about_window = NULL;
	
	cx = 0;
	
	// Destroy the pixbuf, if it's still around.
	if (pbufIce)
	{
		g_object_unref(G_OBJECT(pbufIce));
		pbufIce = NULL;
	}
}


/**
 * about_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean about_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(widget);
	GSFT_UNUSED_PARAMETER(event);
	GSFT_UNUSED_PARAMETER(user_data);
	
	about_window_close();
	return FALSE;
}


/**
 * about_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void about_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(dialog);
	GSFT_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CLOSE:
			about_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other event. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


// TODO: Verify that this works on big-endian machines.
#define ICE_RGB(r, g, b) (0xFF000000 | (r) | ((g) << 8) | (b << 16))

static void about_window_update_ice(void)
{
	if (!imgGensLogo)
		return;
	
	const unsigned char *src = &about_window_data[ax*01440];
	const unsigned char *src2 = &about_window_dx[bx*040];
	const int r = gdk_pixbuf_get_rowstride(pbufIce) >> 2;
	
	unsigned int *destPixel1 = (unsigned int*)gdk_pixbuf_get_pixels(pbufIce);
	unsigned int *destPixel2 = destPixel1 + 0120;
	
	for (int y = 0120; y != 0; y -= 2)
	{
		for (int x = 0120; x != 0; x -= 4)
		{
			unsigned int pxc;
			
			unsigned char px1 = (*src & 0360) >> 3;
			unsigned char px2 = (*src & 0017) << 1;
			
			pxc = (!px1 ? 0 : ICE_RGB(((src2[px1 + 1] & 0017) << 4),
						   (src2[px1 + 1] & 0360),
						  ((src2[px1 + 0] & 0017) << 4)));
			*destPixel1++ = pxc;
			*destPixel1++ = pxc;
			*destPixel2++ = pxc;
			*destPixel2++ = pxc;
			
			pxc = (!px2 ? 0 : ICE_RGB(((src2[px2 + 1] & 0017) << 4),
						   (src2[px2 + 1] & 0360),
						  ((src2[px2 + 0] & 0017) << 4)));
			*destPixel1++ = pxc;
			*destPixel1++ = pxc;
			*destPixel2++ = pxc;
			*destPixel2++ = pxc;
			
			src++;
		}
		
		destPixel1 += r;
		destPixel2 += r;
	}
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(imgGensLogo), pbufIce);
}


static gboolean about_window_callback_iceTimer(gpointer data)
{
	GSFT_UNUSED_PARAMETER(data);
	
	if (!cx)
		return FALSE;
	
	ax ^= 1;
	bx++;
	if (bx >= 10)
		bx = 0;
	
	about_window_update_ice();
	
	return TRUE;
}
