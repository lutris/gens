/***************************************************************************
 * MDP: VDP Debugger. (Window Code) (GTK+)                                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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
#include <stdlib.h>
#include <errno.h>

#include "vdpdbg_window.h"
#include "vdpdbg_plugin.h"
#include "vdpdbg.h"

// VDP Data.
#include "vdp_data.h"
#include "vdp_reg_m5.h"

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_mem.h"
#include "mdp/mdp_reg.h"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"
#include "libgsft/gsft_unused.h"


// Window.
static GtkWidget *vdpdbg_window = NULL;

// CRAM dump.
static GdkPixbuf *pbufCRam = NULL;
static GtkWidget *imgCRam;
static uint16_t prev_CRam[16*4];

// Register listing.
static GtkWidget *lstRegList;
static GtkListStore *lmRegList = NULL;

// Widget creation functions.
static void vdpdbg_window_create_lstRegList(GtkWidget *container);

// Callbacks.
static gboolean	vdpdbg_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	vdpdbg_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	vdpdbg_window_callback_reg_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer user_data);

// Data functions.
static void vdpdbg_window_update_lstRegList(mdp_reg_vdp_t *reg_vdp);


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
	
	// Clear previous CRam buffer.
	memset(prev_CRam, 0, sizeof(prev_CRam));
	
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
	
	// Clear the pixel buffer.
	uint32_t *pixels = (uint32_t*)gdk_pixbuf_get_pixels(pbufCRam);
	int rowstride = gdk_pixbuf_get_rowstride(pbufCRam);
	int n_channels = gdk_pixbuf_get_n_channels(pbufCRam);
	int px_count = ((rowstride * 63) + (16*16*n_channels)) / 4;
	
	for (; px_count != 0; px_count -= 4)
	{
		*pixels = 0xFF000000U;
		*(pixels + 1) = 0xFF000000U;
		*(pixels + 2) = 0xFF000000U;
		*(pixels + 3) = 0xFF000000U;
		pixels += 4;
	}
	
	
	// Create the GtkTreeView for the register listing.
	vdpdbg_window_create_lstRegList(vboxDialog);
	
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
 * vdpdbg_window_create_lstRegList(): Create the register list.
 * @param container Container for the register list.
 */
static void vdpdbg_window_create_lstRegList(GtkWidget *container)
{
	// Create the list model.
	if (lmRegList)
	{
		// List model already exists. Clear it.
		gtk_list_store_clear(GTK_LIST_STORE(lmRegList));
	}
	else
	{
		lmRegList = gtk_list_store_new(5,
				G_TYPE_INT,		// Register number.
				G_TYPE_STRING,		// Register name.
				G_TYPE_INT,		// Value.
				G_TYPE_STRING,		// Value (in hex).
				G_TYPE_STRING);		// Description.
	}
	
	// GtkTreeView containing the register list.
	lstRegList = gtk_tree_view_new();
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(lstRegList), FALSE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(lstRegList)),
				    GTK_SELECTION_SINGLE);
	gtk_widget_show(lstRegList);
	gtk_container_add(GTK_CONTAINER(container), lstRegList);
	
	// Create the renderers and columns for the treeview.
	
	// Register number.
	GtkCellRenderer  *rendRegNo = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colRegNo = gtk_tree_view_column_new_with_attributes("#", rendRegNo, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstRegList), colRegNo);
	
	// Register name.
	GtkCellRenderer  *rendRegName = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colRegName = gtk_tree_view_column_new_with_attributes("Name", rendRegName, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstRegList), colRegName);
	
	// Value.
	GtkCellRenderer  *rendValue = gtk_cell_renderer_text_new();
	GValue editable = {0, {{0}, {0}}};
	g_value_init(&editable, G_TYPE_BOOLEAN);
	g_value_set_boolean(&editable, TRUE);
	g_object_set_property(G_OBJECT(rendValue), "editable", &editable);
	g_signal_connect((gpointer)rendValue, "edited",
			 G_CALLBACK(vdpdbg_window_callback_reg_edited), NULL);
	GtkTreeViewColumn *colValue = gtk_tree_view_column_new_with_attributes("Value", rendValue, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstRegList), colValue);
	
	// Description.
	GtkCellRenderer  *rendDescription = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colDescription = gtk_tree_view_column_new_with_attributes("Description", rendDescription, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstRegList), colDescription);
	
	// Populate the register list.
	// NOTE: These names are correct for MD mode only.
	// SMS mode may be different.
	for (int reg_num = 0; reg_num < 24; reg_num++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(lmRegList, &iter);
		gtk_list_store_set(GTK_LIST_STORE(lmRegList), &iter,
					0, reg_num,			// Register number.
					1, vdp_m5_reg_name[reg_num],	// Register name.
					2, -1,				// Value.
					3, "0x00",			// Value (in hex).
					4, "", -1);			// Description.
	}
	
	// Set the GtkTreeView's list model.
	gtk_tree_view_set_model(GTK_TREE_VIEW(lstRegList), GTK_TREE_MODEL(lmRegList));
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
	gboolean CRam_needs_update = FALSE;
	for (int y = 0; y < (4*16); y++)
	{
		for (int x = 0; x < 16; x++)
		{
			const int color_index = (x + (y & 0x30));
			const uint16_t colorMD = CRam[color_index] & 0xFFF;
			if (colorMD == prev_CRam[color_index])
			{
				// Color hasn't changed.
				pixel += 16;
				continue;
			}
			
			const uint32_t colorRGB = vdp_color_MDtoSys[colorMD];
			CRam_needs_update = TRUE;
			
			// Draw 16 pixels of the current color.
			for (unsigned int ix = 16; ix != 0; ix -= 4)
			{
				*pixel = colorRGB;
				*(pixel + 1) = colorRGB;
				*(pixel + 2) = colorRGB;
				*(pixel + 3) = colorRGB;
				pixel += 4;
			}
		}
		
		// Next row.
		pixel += (rowDiff / sizeof(*pixel));
	}
	
	if (CRam_needs_update)
	{
		// Copy the current CRam to prev_CRam.
		memcpy(prev_CRam, CRam, sizeof(prev_CRam));
		
		// Update the image.
		gtk_image_set_from_pixbuf(GTK_IMAGE(imgCRam), pbufCRam);
	}
	
	// Register list.
	mdp_reg_vdp_t reg_vdp;
	ret = vdpdbg_host_srv->reg_get_all(MDP_REG_IC_VDP, &reg_vdp);
	if (ret == MDP_ERR_OK)
		vdpdbg_window_update_lstRegList(&reg_vdp);
}


/**
 * vdpdbg_window_update_lstRegList(): Update the VDP register list.
 * @param reg_vdp VDP registers.
 */
static void vdpdbg_window_update_lstRegList(mdp_reg_vdp_t *reg_vdp)
{
	// Go through the list and update the registers.
	// TODO: If a user is editing a register, don't update that register.
	GtkTreeIter iter;
	int reg_num;
	char hex_value[16];
	char desc[1024];
	uint8_t reg_value;
	int prev_value;
	
	// DMA Length and DMA Src Addr stuff.
	GtkTreeIter iter_DMA_Len;
	GtkTreeIter iter_DMA_Src;
	gboolean DMA_Len_NeedsUpdate = FALSE;
	gboolean DMA_Src_NeedsUpdate = FALSE;
	
	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmRegList), &iter);
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(lmRegList), &iter, 0, &reg_num, 2, &prev_value, -1);
		if (reg_num < 0 || reg_num >= 24)
		{
			// Invalid register number. Go to the next entry.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmRegList), &iter);
			continue;
		}
		
		// TODO: If these aren't found but 20/22/23 are, an error will occur.
		if (reg_num == 19)
			iter_DMA_Len = iter;
		else if (reg_num == 21)
			iter_DMA_Src = iter;
		
		// Get the register value.
		// (prev_value == -1) means the register hasn't been updated yet.
		// TODO: DMA Length and DMA Src Addr are multibyte values.
		// With this method, they're only updated if the low byte is changed.
		// Maybe they should be displayed in a different area...
		reg_value = reg_vdp->data[reg_num];
		if (prev_value != -1 && ((uint8_t)prev_value == reg_value))
		{
			// Register hasn't been changed. Skip it.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmRegList), &iter);
			continue;
		}
		
		// Create the hexadecimal value.
		szprintf(hex_value, sizeof(hex_value), "0x%02X", reg_value);
		
		// Get the description.
		vdpdbg_get_m5_reg_desc(reg_num, reg_value, reg_vdp, desc, sizeof(desc));
		if (reg_num == 19 || reg_num == 20)
			DMA_Len_NeedsUpdate = TRUE;
		else if (reg_num >= 21 && reg_num <= 23)
			DMA_Src_NeedsUpdate = TRUE;
		
		// Set the value and description.
		gtk_list_store_set(GTK_LIST_STORE(lmRegList), &iter,
					2, reg_value,	// Value.
					3, hex_value,	// Value (in hex).
					4, desc, -1);	// Description.
		
		// Get the next list element.
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmRegList), &iter);
	}
	
	// Check if DMA Length or DMA Src Address need to be updated.
	if (DMA_Len_NeedsUpdate)
	{
		// DMA Length needs to be updated.
		vdpdbg_get_m5_dma_len_desc(reg_vdp, desc, sizeof(desc));
		gtk_list_store_set(GTK_LIST_STORE(lmRegList), &iter_DMA_Len, 4, desc, -1);
	}
	
	if (DMA_Src_NeedsUpdate)
	{
		// DMA Src Address needs to be updated.
		vdpdbg_get_m5_dma_src_desc(reg_vdp, desc, sizeof(desc));
		gtk_list_store_set(GTK_LIST_STORE(lmRegList), &iter_DMA_Src, 4, desc, -1);
	}
}


/**
 * vdpdbg_window_callback_reg_edited(): A register was edited by the user.
 * @param renderer Cell renderer.
 * @param path Path in the list store.
 * @param new_text New text entry.
 * @param user_data User data.
 */
static void vdpdbg_window_callback_reg_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer user_data)
{
	GSFT_UNUSED_PARAMETER(renderer);
	GSFT_UNUSED_PARAMETER(user_data);
	
	// Convert the text to a number.
	errno = 0;
	int new_value = strtol(new_text, NULL, 0);
	if (errno != 0 || (new_value < 0 || new_value > 0xFF))
		return;
	
	// Get the register number.
	GtkTreePath *treePath;
	GtkTreeIter iter;
	
	treePath = gtk_tree_path_new_from_string(path);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(lmRegList), &iter, treePath);
	
	int reg_number;
	gtk_tree_model_get(GTK_TREE_MODEL(lmRegList), &iter, 0, &reg_number, -1);
	if (reg_number < 0 || reg_number >= 24)
	{
		gtk_tree_path_free(treePath);
		return;
	}
	
	// Set the VDP register.
	vdpdbg_host_srv->reg_set(&mdp, MDP_REG_IC_VDP, reg_number, new_value);
}
