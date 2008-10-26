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

// git version
#include "macros/git.h"

#include "emulator/g_main.hpp"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// TODO: Get rid of gtk-misc.h
#include "gtk-misc.h"

#include "ui/about_window_data.h"

// C includes
#include <cstring>

// C++ includes
#include <sstream>
using std::stringstream;


AboutWindow* AboutWindow::m_Instance = NULL;
AboutWindow* AboutWindow::Instance(GtkWidget *parent)
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
	gtk_widget_set_name(m_Window, "about_window");
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 0);
	gtk_window_set_title(GTK_WINDOW(m_Window), "About Gens");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), FALSE);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(m_Window), "Gens_About_Window", m_Window);
	
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
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the main VBox.
	GtkWidget *vboxMain = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vboxMain, "vboxMain");
	gtk_widget_show(vboxMain);
	gtk_container_add(GTK_CONTAINER(vboxDialog), vboxMain);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxMain",
			       g_object_ref(vboxMain), (GDestroyNotify)g_object_unref);
	
	// Create the HBox for the logo and the program information.
	GtkWidget *hboxLogo = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hboxLogo, "hboxLogo");
	gtk_widget_show(hboxLogo);
	gtk_box_pack_start(GTK_BOX(vboxMain), hboxLogo, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(m_Window), "hboxLogo",
			       g_object_ref(hboxLogo), (GDestroyNotify)g_object_unref);
	
	// Gens logo
	m_imgGensLogo = create_pixmap("gens_small.png");
	gtk_widget_set_name(m_imgGensLogo, "m_imgGensLogo");
	gtk_widget_show(m_imgGensLogo);
	gtk_box_pack_start(GTK_BOX(hboxLogo), m_imgGensLogo, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(m_Window), "imgGensLogo",
			       g_object_ref(m_imgGensLogo), (GDestroyNotify)g_object_unref);
	
	cx = 0;
	if (ice == 3)
	{
		ax = 0; bx = 0; cx = 1;
		g_timeout_add(100, GTK_iceTime, (gpointer)this);
		updateIce();
	}
	
	// Version information
	stringstream version;
	version << "<b><i>" << aboutTitle;
#ifdef GENS_GIT_VERSION
	version << "\n" << GENS_GIT_VERSION;
#endif /* GENS_GIT_VERSION */
	version << "</i></b>\n\n" << aboutDesc;
	
	GtkWidget *lblVersion = gtk_label_new(version.str().c_str());
	gtk_widget_set_name(lblVersion, "lblVersion");
	gtk_label_set_use_markup(GTK_LABEL(lblVersion), TRUE);
	gtk_label_set_justify(GTK_LABEL(lblVersion), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblVersion);
	gtk_box_pack_start(GTK_BOX(hboxLogo), lblVersion, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(m_Window), "lblVersion",
			       g_object_ref(lblVersion), (GDestroyNotify)g_object_unref);
	
	// Copyright frame
	GtkWidget *fraCopyright = gtk_frame_new(NULL);
	gtk_widget_set_name(fraCopyright, "fraCopyright");
	gtk_container_set_border_width(GTK_CONTAINER(fraCopyright), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(fraCopyright), GTK_SHADOW_NONE);
	gtk_widget_show(fraCopyright);
	gtk_box_pack_start(GTK_BOX(vboxMain), fraCopyright, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(m_Window), "fraCopyright",
			       g_object_ref(fraCopyright), (GDestroyNotify)g_object_unref);
	
	// Copyright label
	GtkWidget *lblCopyright = gtk_label_new(aboutCopyright);
	gtk_widget_set_name(lblCopyright, "lblCopyright");
	gtk_widget_show(lblCopyright);
	gtk_container_add(GTK_CONTAINER(fraCopyright), lblCopyright);
	g_object_set_data_full(G_OBJECT(m_Window), "lblCopyright",
			       g_object_ref(lblCopyright), (GDestroyNotify)g_object_unref);
	
#if 0
	// Get the dialog action area.
	about_dialog_action_area = GTK_DIALOG(about_window)->action_area;
	gtk_widget_set_name(about_dialog_action_area, "about_dialog_action_area");
	gtk_widget_show(about_dialog_action_area);
	GLADE_HOOKUP_OBJECT_NO_REF(about_window, about_dialog_action_area, "about_dialog_action_area");
#endif
	
	// Create an accelerator group.
	GtkAccelGroup *accel_group = gtk_accel_group_new();
	
	// Add the OK button to the dialog action area.
	GtkWidget *btnOK = gtk_button_new_from_stock("gtk-ok");
	gtk_widget_set_name(btnOK, "btnOK");
	GTK_WIDGET_SET_FLAGS(btnOK, GTK_CAN_DEFAULT);
	gtk_widget_show(btnOK);
	gtk_dialog_add_action_widget(GTK_DIALOG(m_Window), btnOK, GTK_RESPONSE_OK);
	
	g_signal_connect((gpointer)btnOK, "clicked",
			 G_CALLBACK(AboutWindow::GTK_OK), this);
	
	gtk_widget_add_accelerator(btnOK, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(btnOK, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	
	g_object_set_data_full(G_OBJECT(m_Window), "btnOK",
			       g_object_ref(btnOK), (GDestroyNotify)g_object_unref);
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(m_Window), accel_group);
	
	// Show the window.
	gtk_widget_show_all(m_Window);
}

AboutWindow::~AboutWindow()
{
	cx = 0;
	
	if (m_Window)
		gtk_widget_destroy(m_Window);
	
	m_Instance = NULL;
}


void AboutWindow::setFocus(void)
{
	if (m_Window)
		gtk_widget_grab_focus(m_Window);
}


void AboutWindow::setModal(GtkWidget *parent)
{
	if (m_Window)
		gtk_window_set_transient_for(GTK_WINDOW(m_Window), GTK_WINDOW(parent));
}


gboolean AboutWindow::GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return reinterpret_cast<AboutWindow*>(user_data)->close();
}


void AboutWindow::GTK_OK(GtkButton *button, gpointer user_data)
{
	reinterpret_cast<AboutWindow*>(user_data)->close();
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


void AboutWindow::updateIce(void)
{
	if (!m_imgGensLogo)
		return;
	
	GdkPixbuf *icebuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 0120, 0120);
	int x, y, r;
	const unsigned char *src = &about_data[ax*01440];
	const unsigned char *src2 = &about_dx[bx*040];
	unsigned char px1, px2;
	guchar *pixels = gdk_pixbuf_get_pixels(icebuf);
	r = gdk_pixbuf_get_rowstride(icebuf);
	
	memset(pixels, 0, 062000);
	for (y = 0; y < 0120; y += 2)
	{
		for (x = 0; x < 0120; x += 4)
		{
			px1 = (*src & 0360) >> 3;
			px2 = (*src & 0017) << 1;
			
			pixels[y*r + x*4 + 0] = (src2[px1 + 1] & 0017) << 4;
			pixels[y*r + x*4 + 1] = (src2[px1 + 1] & 0360);
			pixels[y*r + x*4 + 2] = (src2[px1 + 0] & 0017) << 4;
			pixels[y*r + x*4 + 3] = (px1 ? '\377' : '\000');
			
			pixels[y*r + x*4 + 4] = (src2[px1 + 1] & 0017) << 4;
			pixels[y*r + x*4 + 5] = (src2[px1 + 1] & 0360);
			pixels[y*r + x*4 + 6] = (src2[px1 + 0] & 0017) << 4;
			pixels[y*r + x*4 + 7] = (px1 ? '\377' : '\000');
			
			pixels[y*r + x*4 + 8] = (src2[px2 + 1] & 0017) << 4;
			pixels[y*r + x*4 + 9] = (src2[px2 + 1] & 0360);
			pixels[y*r + x*4 + 10] = (src2[px2 + 0] & 0017) << 4;
			pixels[y*r + x*4 + 11] = (px2 ? '\377' : '\000');
			
			pixels[y*r + x*4 + 12] = (src2[px2 + 1] & 0017) << 4;
			pixels[y*r + x*4 + 13] = (src2[px2 + 1] & 0360);
			pixels[y*r + x*4 + 14] = (src2[px2 + 0] & 0017) << 4;
			pixels[y*r + x*4 + 15] = (px2 ? '\377' : '\000');
			
			memcpy(&pixels[(y+1)*r + x*4], &pixels[y*r + x*4], 16);
			
			src++;
		}
	}
	
	if (m_imgGensLogo)
		gtk_image_set_from_pixbuf(GTK_IMAGE(m_imgGensLogo), icebuf);
	
	g_object_unref(G_OBJECT(icebuf));
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
