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
#include "mdp/mdp_reg.h"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"


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
	GtkTreeViewColumn *colValue = gtk_tree_view_column_new_with_attributes("Value", rendValue, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstRegList), colValue);
	
	// Description.
	GtkCellRenderer  *rendDescription = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *colDescription = gtk_tree_view_column_new_with_attributes("Description", rendDescription, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(lstRegList), colDescription);
	
	// Populate the register list.
	// NOTE: These names are correct for MD mode only.
	// SMS mode may be different.
	static const char *reg_name[24+1] =
	{
		"Mode Set 1",			// 0
		"Mode Set 2",			// 1
		"Scroll A Pattern Addr",	// 2
		"Window Pattern Addr",		// 3
		"Scroll B Pattern Addr",	// 4
		"Sprite Attr Table Addr",	// 5
		"unused",			// 6
		"Background Color",		// 7
		"unused",			// 8
		"unused",			// 9
		"H Interrupt",			// 10
		"Mode Set 3",			// 11
		"Mode Set 4",			// 12
		"H Scroll Addr",		// 13
		"unused",			// 14
		"Auto Increment",		// 15
		"Scroll Size",			// 16
		"Window H Pos",			// 17
		"Window V Pos",			// 18
		"DMA Length Low",		// 19
		"DMA Length High",		// 20
		"DMA Src Low",			// 21
		"DMA Src Mid",			// 22
		"DMA Src High",			// 23
		NULL
	};
	
	for (int i = 0; i < 24; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(lmRegList, &iter);
		gtk_list_store_set(GTK_LIST_STORE(lmRegList), &iter,
					0, i,			// Register number.
					1, reg_name[i],		// Register name.
					2, -1,			// Value.
					3, "0x00",		// Value (in hex).
					4, "", -1);		// Description.
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
	GtkTreeIter iter;
	int i;
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
		gtk_tree_model_get(GTK_TREE_MODEL(lmRegList), &iter, 0, &i, 2, &prev_value, -1);
		if (i < 0 || i >= 24)
		{
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmRegList), &iter);
			continue;
		}
		
		if (i == 19)
			iter_DMA_Len = iter;
		else if (i == 21)
			iter_DMA_Src = iter;
		
		// Get the register value.
		// (prev_value == -1) means the register hasn't been updated yet.
		// TODO: DMA Length and DMA Src Addr are multibyte values.
		// With this method, they're only updated if the low byte is changed.
		// Maybe they should be displayed in a different area...
		reg_value = reg_vdp->data[i];
		if (prev_value != -1 && ((uint8_t)prev_value == reg_value))
		{
			// Register hasn't been changed. Skip it.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmRegList), &iter);
			continue;
		}
		
		// Create the hexadecimal value.
		szprintf(hex_value, sizeof(hex_value), "0x%02X", reg_value);
		
		// Create the description.
		switch (i)
		{
			case 0:
				// Mode Set 1.
				szprintf(desc, sizeof(desc), "H Int %s; HV counter %s",
						(reg_value & 0x10 ? "ON" : "OFF"),
						(reg_value & 0x02 ? "ON" : "OFF"));
				break;
			
			case 1:
				// Mode Set 2.
				szprintf(desc, sizeof(desc), "Disp %s; V Int %s; DMA %s; V%d",
						(reg_value & 0x40 ? "ON" : "OFF"),
						(reg_value & 0x20 ? "ON" : "OFF"),
						(reg_value & 0x10 ? "ON" : "OFF"),
						(reg_value & 0x08 ? 30 : 28));
				break;
			
			case 2:
				// Scroll A Pattern Address.
				szprintf(desc, sizeof(desc), "0x%01X000",
						(reg_value & 0x38) >> 2);
				break;
			
			case 3:
				// Window Pattern Address.
				// NOTE: In H40 mode, bit 1 should be 0.
				// Should this be enforced?
				szprintf(desc, sizeof(desc), "0x%03X0",
						(reg_value & 0x3E) << 6);
				break;
			
			case 4:
				// Scroll B Pattern Address.
				szprintf(desc, sizeof(desc), "0x%01X000",
						(reg_value & 0x07) << 1);
				break;
			
			case 5:
				// Sprite Attribute Table Address.
				// NOTE: In H40 mode, bit 0 should be 0.
				// Should this be enforced?
				szprintf(desc, sizeof(desc), "0x%02X00",
						(reg_value & 0x7F) << 1);
				break;
			
			case 7:
				// Background Color.
				szprintf(desc, sizeof(desc), "Palette %d, Color %d",
						((reg_value >> 4) & 0x03),
						(reg_value & 0xF));
				break;
			
			case 10:
				// H Interrupt.
				// If this value is >= vertical resolution, then it has no effect.
				
				if (reg_value >= (reg_vdp->regs.mode_set2 & 0x08 ? (240-1) : (224-1)))
				{
					// No effect.
					szprintf(desc, sizeof(desc), "%d lines (disabled)", reg_value + 1);
				}
				else
				{
					// Has effect.
					szprintf(desc, sizeof(desc), "%d line%s",
							reg_value + 1,
							(reg_value == 0 ? "" : "s"));
				}
				break;
			
			case 11:
			{
				// Mode Set 3.
				static const char HScroll_Desc[][8] = {"Full", "Invalid", "1Cell", "1Line"};
				szprintf(desc, sizeof(desc), "Ext Int %s; VScroll %s; HScroll %s",
						(reg_value & 0x04 ? "ON" : "OFF"),
						(reg_value & 0x04 ? "2Cell" : "Full"),
						HScroll_Desc[reg_value & 0x03]);
				break;
			}
			
			case 12:
			{
				// Mode Set 4.
				// NOTE: Bits 7 and 0 both control H32/H40 mode.
				// We're only looking at Bit 0.
				// Should we look at both?
				static const char *Interlace_Desc[] = {"OFF", "ON (1x res)", "Invalid", "ON (2x res)"};
				szprintf(desc, sizeof(desc), "H%d; S/H %s; Interlace: %s",
						(reg_value & 0x01 ? 40 : 32),
						(reg_value & 0x08 ? "ON" : "OFF"),
						Interlace_Desc[(reg_value & 0x06) >> 1]);
				break;
			}
			
			case 13:
				// H Scroll Address.
				szprintf(desc, sizeof(desc), "0x%02X00",
						(reg_value & 0x3F) << 2);
				break;
			
			case 15:
				// Auto Increment.
				if (reg_value == 0)
					strlcpy(desc, "Disabled", sizeof(desc));
				else
					szprintf(desc, sizeof(desc), "+%d bytes", reg_value);
				break;
			
			case 16:
			{
				// Scroll Size.
				static const char *ScrlSize_Desc[] = {"32 cells", "64 cells", "Invalid", "128 cells"};
				szprintf(desc, sizeof(desc), "H: %s; V: %s",
						ScrlSize_Desc[reg_value & 0x03],
						ScrlSize_Desc[(reg_value >> 4) & 0x03]);
				break;
			}
			
			case 17:
				// Window H Pos.
				szprintf(desc, sizeof(desc), "%d cell%s in %s side from base point",
						(reg_value & 0x1F),
						(reg_value & 1 ? "" : "s"),
						(reg_value & 0x80 ? "right" : "left"));
				break;
			
			case 18:
				// Window V Pos.
				szprintf(desc, sizeof(desc), "%d cell%s in %s side from base point",
						(reg_value & 0x1F),
						(reg_value & 1 ? "" : "s"),
						(reg_value & 0x80 ? "lower" : "upper"));
				break;
			
			case 19:
			case 20:
				// DMA Length.
				DMA_Len_NeedsUpdate = TRUE;
				desc[0] = 0x00;
				break;
				
			case 21:
			case 22:
			case 23:
				// DMA Src Address.
				DMA_Src_NeedsUpdate = TRUE;
				desc[0] = 0x00;
				break;
				
			default:
				// Unused register.
				strlcpy(desc, "unused", sizeof(desc));
				break;
		}
		
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
		szprintf(desc, sizeof(desc), "Length: 0x%04X words",
				(reg_vdp->regs.dma_len_l |
				 (reg_vdp->regs.dma_len_h << 8)));
		gtk_list_store_set(GTK_LIST_STORE(lmRegList), &iter_DMA_Len, 4, desc, -1);
	}
	
	if (DMA_Src_NeedsUpdate)
	{
		// DMA Src Address needs to be updated.
		uint32_t dma_src = reg_vdp->regs.dma_src_l |
				   (reg_vdp->regs.dma_src_m << 8) |
				   (reg_vdp->regs.dma_src_h << 16);
		
		if (!(reg_vdp->regs.dma_src_h & 0x80))
		{
			// Memory to VRAM copy.
			szprintf(desc, sizeof(desc), "Mem to VRAM: 0x%06X",
					(dma_src & 0x7FFFFF) << 1);
		}
		else
		{
			szprintf(desc, sizeof(desc), "VRAM %s: 0x%04X",
					(dma_src & 0x400000 ? "Copy" : "Fill"),
					(dma_src & 0x3FFFFF));
		}
		
		gtk_list_store_set(GTK_LIST_STORE(lmRegList), &iter_DMA_Src, 4, desc, -1);
	}
}
