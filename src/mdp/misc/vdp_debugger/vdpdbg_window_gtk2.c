/***************************************************************************
 * MDP: VDP Debugger. (Window Code) (GTK+)                                 *
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

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// C includes.
#include <stdio.h>
#include <stdint.h>

#include "vdpdbg_window.h"
#include "vdpdbg_plugin.h"
#include "vdpdbg.h"

// VDP Data.
#include "vdp_data.h"

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_mem.h"

// Window.
static GtkWidget *vdpdbg_window = NULL;

// CRAM dump.
static GdkPixbuf *pbufCRam = NULL;
static GtkWidget *imgCRam;

// Callbacks.
static gboolean	vdpdbg_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	vdpdbg_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);


/**
 * vdpdbg_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void MDP_FNCALL vdpdbg_window_show(void *parent)
{
	if (vdpdbg_window)
	{
		// VDP Debugger window is already visible. Set focus.
		gtk_widget_grab_focus(vdpdbg_window);
		return;
	}
	
	// Initialize VDP Data.
	vdpdbg_data_init();
	
	// Create the window.
	vdpdbg_window = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(vdpdbg_window), 4);
	gtk_window_set_title(GTK_WINDOW(vdpdbg_window), "VDP Debugger");
	gtk_window_set_position(GTK_WINDOW(vdpdbg_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(vdpdbg_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(vdpdbg_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(vdpdbg_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)vdpdbg_window, "delete_event",
			 G_CALLBACK(vdpdbg_window_callback_close), NULL);
	g_signal_connect((gpointer)vdpdbg_window, "destroy_event",
			 G_CALLBACK(vdpdbg_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)vdpdbg_window, "response",
			 G_CALLBACK(vdpdbg_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = gtk_bin_get_child(GTK_BIN(vdpdbg_window));
	gtk_box_set_spacing(GTK_BOX(vboxDialog), 8);
	gtk_widget_show(vboxDialog);
	
	// Create the CRam image. (each color will be 16x16)
	pbufCRam = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 16*16, 4*16);
	imgCRam = gtk_image_new_from_pixbuf(pbufCRam);
	gtk_widget_show(imgCRam);
	gtk_box_pack_start(GTK_BOX(vboxDialog), imgCRam, FALSE, FALSE, 0);
	
	// Create the dialog buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(vdpdbg_window),
			       "gtk-close", GTK_RESPONSE_CLOSE,
			       NULL);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(vdpdbg_window), GTK_WINDOW(parent));
	
	// Update the window.
	vdpdbg_window_update();
	
	// Show the window.
	gtk_widget_show_all(vdpdbg_window);
	
	// Register the window with MDP Host Services.
	vdpdbg_host_srv->window_register(&mdp, vdpdbg_window);
}


/**
 * vdpdbg_window_close(): Close the VDP Layer Options window.
 */
void MDP_FNCALL vdpdbg_window_close(void)
{
	if (!vdpdbg_window)
		return;
	
	// Unregister the window from MDP Host Services.
	vdpdbg_host_srv->window_unregister(&mdp, vdpdbg_window);
	
	// Destroy the window.
	gtk_widget_destroy(vdpdbg_window);
	vdpdbg_window = NULL;
	
	// Destroy the pixbufs.
	if (pbufCRam)
	{
		g_object_unref(pbufCRam);
		pbufCRam = NULL;
	}
}


/**
 * vdpdbg_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean vdpdbg_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	vdpdbg_window_close();
	return FALSE;
}


/**
 * vdpdbg_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void vdpdbg_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(dialog);
	MDP_UNUSED_PARAMETER(user_data);
	
	switch (response_id)
	{
		case GTK_RESPONSE_CLOSE:
			// Close.
			vdpdbg_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other response ID. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}


void vdpdbg_window_update(void)
{
	if (!vdpdbg_window)
		return;
	
	// Get the CRam.
	uint16_t CRam[16*4];
	
	int ret = vdpdbg_host_srv->mem_read_block_16(MDP_MEM_MD_CRAM, 0, &CRam[0], sizeof(CRam)/sizeof(CRam[0]));
	if (ret != MDP_ERR_OK)
	{
		// Couldn't read CRam. Assume that CRam is empty.
		memset(CRam, 0x00, sizeof(CRam));
	}
	
	// Copy the CRam to the pixbuf.
	// TODO: Are GDK Pixbufs always 32-bit color?
	uint32_t *pixel = (uint32_t*)gdk_pixbuf_get_pixels(pbufCRam);
	const int rowDiff = gdk_pixbuf_get_rowstride(pbufCRam) - (16*16*sizeof(uint32_t));
	
	// Each row has 16 colors. (Each color occupies a 16x16 area.)
	for (int y = 0; y < (4*16); y++)
	{
		for (int x = 0; x < 16; x++)
		{
			const uint16_t colorMD = CRam[x + (y & 0x30)] & 0xFFF;
			const uint32_t colorRGB = vdp_color_MDtoSys[colorMD];
			
			// Draw 16 pixels of the current color.
			for (unsigned int ix = 16; ix != 0; ix--)
			{
				*pixel++ = colorRGB;
			}
		}
		
		// Next row.
		pixel += (rowDiff / sizeof(*pixel));
	}
	
	// Update the image.
	gtk_image_set_from_pixbuf(GTK_IMAGE(imgCRam), pbufCRam);
}
