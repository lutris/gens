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

#include "about_window.hpp"
#include "gens/gens_window.hpp"

#include "emulator/g_main.hpp"

#include <gtk/gtk.h>

// TODO: Get rid of gtk-misc.h
#include "gtk-misc.h"

// Unused Parameter macro.
#include "macros/unused.h"

// C includes
#include <cstring>

// C++ includes
#include <string>
using std::string;


AboutWindow* AboutWindow::m_Instance = NULL;
AboutWindow* AboutWindow::Instance(GtkWindow *parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the About window.
		m_Instance = new AboutWindow();
	}
	else
	{
		// Instance already exists. Set focus.
		m_Instance->setFocus();
	}
	
	// Set modality of the window.
	if (!parent)
		parent = GTK_WINDOW(gens_window);
	m_Instance->setModal(parent);
	
	return m_Instance;
}


/**
 * AboutWindow(): Create the About Window.
 */
AboutWindow::AboutWindow()
{
	// Create the About window.
	m_Window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 0);
	gtk_window_set_title(GTK_WINDOW(m_Window), "About Gens");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), FALSE);
	
	// Load the Gens icon.
	GdkPixbuf *icon = create_pixbuf("Gens2.ico");
	if (icon)
	{
		gtk_window_set_icon(GTK_WINDOW(m_Window), icon);
		gdk_pixbuf_unref(icon);
	}
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)m_Window, "delete_event",
			 G_CALLBACK(AboutWindow::GTK_Close), (gpointer)this);
	g_signal_connect((gpointer)m_Window, "destroy_event",
			 G_CALLBACK(AboutWindow::GTK_Close), (gpointer)this);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(m_Window)->vbox;
	gtk_widget_show(vboxDialog);
	
	// Create the HBox for the logo and the program information.
	GtkWidget *hboxLogo = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hboxLogo);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxLogo, TRUE, TRUE, 0);
	
	cx = 0;
	m_pbufIce = NULL;
	if (ice != 3)
	{
		// Gens logo
		m_imgGensLogo = create_pixmap("gens_small.png");
	}
	else
	{
		ax = 0; bx = 0; cx = 1;
		m_pbufIce = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 0120, 0120);
		m_imgGensLogo = gtk_image_new_from_pixbuf(m_pbufIce);
		g_timeout_add(100, GTK_iceTime, (gpointer)this);
		updateIce();
	}
	
	gtk_widget_show(GTK_WIDGET(m_imgGensLogo));
	gtk_box_pack_start(GTK_BOX(hboxLogo), GTK_WIDGET(m_imgGensLogo), TRUE, TRUE, 0);
	
	// Version information
	static const string title = "<b><i>" + string(StrTitle) + "</i></b>\n<small>\n</small>" + string(StrDescription);
	GtkWidget *lblVersion = gtk_label_new(title.c_str());
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
	GtkWidget *lblCopyright = gtk_label_new(StrCopyright);
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
	GtkWidget *lblIncludedLibs = gtk_label_new(StrIncludedLibs);
	gtk_misc_set_padding(GTK_MISC(lblIncludedLibs), 8, 8);
	gtk_misc_set_alignment(GTK_MISC(lblIncludedLibs), 0.0f, 0.0f);
	gtk_widget_show(lblIncludedLibs);
	gtk_container_add(GTK_CONTAINER(fraIncludedLibs), lblIncludedLibs);
	
	// Create an accelerator group.
	m_AccelTable = gtk_accel_group_new();
	
	// Add the OK button.
	addDialogButtons(m_Window, WndBase::BAlign_Default,
			 WndBase::BUTTON_OK, WndBase::BUTTON_OK,
			 WndBase::BUTTON_ALL);
	
	// Add the accel group to the window.
	gtk_window_add_accel_group(GTK_WINDOW(m_Window), GTK_ACCEL_GROUP(m_AccelTable));
	
	// Show the window.
	gtk_widget_show_all(GTK_WIDGET(m_Window));
}

AboutWindow::~AboutWindow()
{
	cx = 0;
	
	if (GTK_IS_WINDOW(m_Window))
		gtk_widget_destroy(GTK_WIDGET(m_Window));
	
	if (m_pbufIce)
	{
		g_object_unref(G_OBJECT(m_pbufIce));
		m_pbufIce = NULL;
	}
	
	m_Window = NULL;
	m_Instance = NULL;
}


gboolean AboutWindow::GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	
	return reinterpret_cast<AboutWindow*>(user_data)->close();
}


void AboutWindow::dlgButtonPress(uint32_t button)
{
	if (button == WndBase::BUTTON_OK)
		close();
}


/**
 * Window is closed.
 */
gboolean AboutWindow::close(void)
{
	if (cx != 0)
	{
		cx = 0;
		return FALSE;
	}
	
	delete this;
	return FALSE;
}


// TODO: Verify that this works on big-endian machines.
#define ICE_RGB(r, g, b) (0xFF000000 | (r) | ((g) << 8) | (b << 16))

void AboutWindow::updateIce(void)
{
	if (!m_imgGensLogo)
		return;
	
	const unsigned char *src = &Data[ax*01440];
	const unsigned char *src2 = &DX[bx*040];
	const int r = gdk_pixbuf_get_rowstride(m_pbufIce) >> 2;
	
	unsigned int *destPixel1 = (unsigned int*)gdk_pixbuf_get_pixels(m_pbufIce);
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
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(m_imgGensLogo), m_pbufIce);
}


gboolean AboutWindow::GTK_iceTime(gpointer user_data)
{
	return reinterpret_cast<AboutWindow*>(user_data)->iceTime();
}


gboolean AboutWindow::iceTime(void)
{
	if (!cx)
	{
		delete this;
		return FALSE;
	}
	
	ax ^= 1;
	bx++;
	if (bx >= 10)
		bx = 0;
	
	updateIce();
	
	return TRUE;
}
