/***************************************************************************
 * Gens: [MDP] VDP Layer Options. (Window Code) (Win32)                    *
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

#include "vlopt_window.h"
#include "vlopt_options.h"
#include "vlopt_plugin.h"
#include "vlopt.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// Response ID for the "Reset" button.
#define VLOPT_RESPONSE_RESET -64

// Window.
static HWND vlopt_window = NULL;
static WNDCLASS vlopt_window_wndclass;

// Checkboxes.
static HWND vlopt_window_checkboxes[VLOPT_OPTIONS_COUNT];

// Window Procedure.
static LRESULT CALLBACK vlopt_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Create Child Windows function.
static void vlopt_window_create_child_windows(HWND hWnd);
BOOL vlopt_window_child_windows_created;

// Option handling functions.
static void vlopt_window_load_options(void);
static void vlopt_window_save_options(void);


/**
 * vlopt_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void vlopt_window_show(void *parent)
{
	if (vlopt_window)
	{
		// VDP Layer Options window is already visible.
		// Set focus.
		// TODO
		return;
	}
	
	vlopt_window_child_windows_created = FALSE;
	
	// Create the window class.
	if (vlopt_window_wndclass.lpfnWndProc != vlopt_window_wndproc)
	{
		vlopt_window_wndclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		vlopt_window_wndclass.lpfnWndProc = vlopt_window_wndproc;
		vlopt_window_wndclass.cbClsExtra = 0;
		vlopt_window_wndclass.cbWndExtra = 0;
		vlopt_window_wndclass.hInstance = GetModuleHandle(NULL);
		vlopt_window_wndclass.hIcon = NULL;
		vlopt_window_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		vlopt_window_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		vlopt_window_wndclass.lpszMenuName = NULL;
		vlopt_window_wndclass.lpszClassName = "vlopt_window_wndclass";
		
		RegisterClass(&vlopt_window_wndclass);
	}
	
	// Create the window.
	vlopt_window = CreateWindowEx(0, "vlopt_window_wndclass", "VDP Layer Options",
				      WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				      CW_USEDEFAULT, CW_USEDEFAULT,
				      320, 240,
				      (HWND)parent, NULL, GetModuleHandle(NULL), NULL);
	
#if 0
	// Dialog response callback.
	g_signal_connect((gpointer)vlopt_window, "response",
			 G_CALLBACK(vlopt_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(vlopt_window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(vlopt_window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the main VBox.
	GtkWidget *vboxMain = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxMain, "vboxMain");
	gtk_widget_show(vboxMain);
	gtk_container_add(GTK_CONTAINER(vboxDialog), vboxMain);
	g_object_set_data_full(G_OBJECT(vlopt_window), "vboxMain",
			       g_object_ref(vboxMain), (GDestroyNotify)g_object_unref);
	
	// Create the main frame.
	GtkWidget *fraMain = gtk_frame_new(NULL);
	gtk_widget_set_name(fraMain, "fraMain");
	gtk_widget_show(fraMain);
	gtk_box_pack_start(GTK_BOX(vboxMain), fraMain, FALSE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraMain), GTK_SHADOW_ETCHED_IN);
	g_object_set_data_full(G_OBJECT(vlopt_window), "fraMain",
			       g_object_ref(fraMain), (GDestroyNotify)g_object_unref);
	
	// Main frame label.
	GtkWidget *lblFrameTitle = gtk_label_new("VDP Layer Options");
	gtk_widget_set_name(lblFrameTitle, "lblInfoTitle");
	gtk_label_set_use_markup(GTK_LABEL(lblFrameTitle), TRUE);
	gtk_widget_show(lblFrameTitle);
	gtk_frame_set_label_widget(GTK_FRAME(fraMain), lblFrameTitle);
	g_object_set_data_full(G_OBJECT(vlopt_window), "lblFrameTitle",
			       g_object_ref(lblFrameTitle), (GDestroyNotify)g_object_unref);
	
	// Horizontal padding for the frame VBox.
	GtkWidget *alignVBoxFrame = gtk_alignment_new(0.0f, 0.0f, 0.0f, 0.0f);
	gtk_widget_set_name(alignVBoxFrame, "alignVBoxFrame");
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignVBoxFrame), 0, 0, 4, 4);
	gtk_container_add(GTK_CONTAINER(fraMain), alignVBoxFrame);
	g_object_set_data_full(G_OBJECT(vlopt_window), "alignVBoxFrame",
			       g_object_ref(alignVBoxFrame), (GDestroyNotify)g_object_unref);
	
	// Create the frame VBox.
	GtkWidget *vboxFrame = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_name(vboxFrame, "vboxFrame");
	gtk_widget_show(vboxFrame);
	gtk_container_add(GTK_CONTAINER(alignVBoxFrame), vboxFrame);
	g_object_set_data_full(G_OBJECT(vlopt_window), "vboxFrame",
			       g_object_ref(vboxFrame), (GDestroyNotify)g_object_unref);
	
	// Create the outer table layout for the first 9 layer options.
	GtkWidget *tblOptionsRows = gtk_table_new(4, 2, FALSE);
	gtk_widget_set_name(tblOptionsRows, "tblOptionsRows");
	gtk_widget_show(tblOptionsRows);
	gtk_box_pack_start(GTK_BOX(vboxFrame), tblOptionsRows, FALSE, FALSE, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "tblOptionsRows",
			       g_object_ref(tblOptionsRows),
			       (GDestroyNotify)g_object_unref);
	
	// Create a blank label for the first row.
	GtkWidget *lblBlankRow = gtk_label_new(NULL);
	gtk_widget_set_name(lblBlankRow, "lblBlankRow");
	gtk_widget_show(lblBlankRow);
	gtk_table_attach(GTK_TABLE(tblOptionsRows), lblBlankRow,
			 0, 1, 0, 1,
			 (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "lblBlankRow",
			       g_object_ref(lblBlankRow),
			       (GDestroyNotify)g_object_unref);
	
	// Create the inner table layout for the first 9 layer options.
	GtkWidget *tblOptions = gtk_table_new(4, 3, TRUE);
	gtk_widget_set_name(tblOptions, "tblOptions");
	gtk_widget_show(tblOptions);
	gtk_table_attach(GTK_TABLE(tblOptionsRows), tblOptions,
			 1, 2, 0, 4,
			 (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
	g_object_set_data_full(G_OBJECT(vlopt_window), "tblOptions",
			       g_object_ref(tblOptions),
			       (GDestroyNotify)g_object_unref);
	
	// Buffer for widget names.
	char buf[32];
	
	// Create column and row labels.
	for (int i = 0; i < 3; i++)
	{
		// Column label.
		sprintf(buf, "lblTblColHeader_%d", i);
		GtkWidget *lblTblColHeader = gtk_label_new(vlopt_options[i].sublayer);
		gtk_widget_set_name(lblTblColHeader, buf);
		gtk_misc_set_alignment(GTK_MISC(lblTblColHeader), 0.5f, 0.5f);
		gtk_widget_show(lblTblColHeader);
		
		gtk_table_attach(GTK_TABLE(tblOptions), lblTblColHeader,
				 i, i + 1, 0, 1,
				 (GtkAttachOptions)0, (GtkAttachOptions)0, 2, 2);
		
		g_object_set_data_full(G_OBJECT(vlopt_window), buf,
				       g_object_ref(lblTblColHeader),
				       (GDestroyNotify)g_object_unref);
		
		// Row label.
		sprintf(buf, "lblTblRowHeader_%d", i);
		GtkWidget *lblTblRowHeader = gtk_label_new(vlopt_options[i * 3].layer);
		gtk_widget_set_name(lblTblRowHeader, buf);
		gtk_misc_set_alignment(GTK_MISC(lblTblRowHeader), 0.5f, 0.5f);
		gtk_widget_show(lblTblRowHeader);
		
		gtk_table_attach(GTK_TABLE(tblOptionsRows), lblTblRowHeader,
				 0, 1, i + 1, i + 2,
				 (GtkAttachOptions)0, (GtkAttachOptions)0, 2, 2);
		
		g_object_set_data_full(G_OBJECT(vlopt_window), buf,
				       g_object_ref(lblTblRowHeader),
				       (GDestroyNotify)g_object_unref);
	}
	
	// Create the VDP Layer Options checkboxes.
	uint8_t row = 1, col = 0;
	for (unsigned int i = 0; i < 9; i++)
	{
		sprintf(buf, "vlopt_checkboxes_%d", i);
		vlopt_checkboxes[i] = gtk_check_button_new();
		gtk_widget_set_name(vlopt_checkboxes[i], buf);
		gtk_widget_show(vlopt_checkboxes[i]);
		
		gtk_table_attach(GTK_TABLE(tblOptions), vlopt_checkboxes[i],
				 col, col + 1, row, row + 1,
				 (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
		
		// Next cell.
		col++;
		if (col >= 3)
		{
			col = 0;
			row++;
		}
		
		// Set the callback.
		g_signal_connect((gpointer)vlopt_checkboxes[i], "toggled",
				 G_CALLBACK(vlopt_window_callback_checkbox_toggled),
				 GINT_TO_POINTER(i));
		
		g_object_set_data_full(G_OBJECT(vlopt_window), buf,
				       g_object_ref(vlopt_checkboxes[i]),
				       (GDestroyNotify)g_object_unref);
	}
	
	// Create the checkboxes for the remaining VDP Layer Options.
	for (unsigned int i = 9; i < VLOPT_OPTIONS_COUNT; i++)
	{
		sprintf(buf, "vlopt_checkboxes_%d", i);
		vlopt_checkboxes[i] = gtk_check_button_new_with_label(vlopt_options[i].layer);
		gtk_widget_set_name(vlopt_checkboxes[i], buf);
		gtk_widget_show(vlopt_checkboxes[i]);
		
		gtk_box_pack_start(GTK_BOX(vboxFrame), vlopt_checkboxes[i], FALSE, FALSE, 0);
		
		// Set the callback.
		g_signal_connect((gpointer)vlopt_checkboxes[i], "toggled",
				 G_CALLBACK(vlopt_window_callback_checkbox_toggled),
				 GINT_TO_POINTER(i));
		
		g_object_set_data_full(G_OBJECT(vlopt_window), buf,
				       g_object_ref(vlopt_checkboxes[i]),
				       (GDestroyNotify)g_object_unref);
	}
	
	// Create the "Reset" button.
	GtkWidget *btnReset = gtk_button_new_with_mnemonic("_Reset");
	gtk_widget_set_name(btnReset, "btnReset");
	gtk_dialog_add_action_widget(GTK_DIALOG(vlopt_window), btnReset, VLOPT_RESPONSE_RESET);
	g_object_set_data_full(G_OBJECT(vlopt_window), buf,
			       g_object_ref(btnReset),
			       (GDestroyNotify)g_object_unref);
	
	// Create the icon for the "Reset" button.
	GtkWidget *imgReset = gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_BUTTON);
	gtk_widget_set_name(imgReset, "imgReset");
	gtk_widget_show(imgReset);
	gtk_button_set_image(GTK_BUTTON(btnReset), imgReset);
	g_object_set_data_full(G_OBJECT(btnReset), "imgReset",
			       g_object_ref(imgReset),
			       (GDestroyNotify)g_object_unref);
	
	// Create the "Close" button.
	GtkWidget *btnClose = gtk_button_new_from_stock("gtk-close");
	gtk_widget_set_name(btnClose, "btnClose");
	gtk_dialog_add_action_widget(GTK_DIALOG(vlopt_window), btnClose, GTK_RESPONSE_CLOSE);
	g_object_set_data_full(G_OBJECT(vlopt_window), buf,
			       g_object_ref(btnClose),
			       (GDestroyNotify)g_object_unref);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(vlopt_window), GTK_WINDOW(parent));
	
	// Load the options.
	vlopt_window_load_options();
	
	// Show the window.
	gtk_widget_show_all(vlopt_window);
#endif
	
	UpdateWindow(vlopt_window);
	ShowWindow(vlopt_window, TRUE);
	
	// Register the window with MDP Host Services.
	vlopt_host_srv->window_register(&mdp, vlopt_window);
}


/**
 * vlopt_window_close(): Close the VDP Layer Options window.
 */
void vlopt_window_close(void)
{
	if (!vlopt_window)
		return;
	
	// Unregister the window from MDP Host Services.
	vlopt_host_srv->window_unregister(&mdp, vlopt_window);
	
	// Destroy the window.
	HWND tmp = vlopt_window;
	vlopt_window = NULL;
	DestroyWindow(tmp);
}


/**
 * vlopt_wndproc(): Window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK vlopt_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			vlopt_window_create_child_windows(hWnd);
			break;
		
		case WM_CLOSE:
			vlopt_window_close();
			return 0;
		
		case WM_COMMAND:
			// TODO
			break;
		
		case WM_DESTROY:
			vlopt_window_close();
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * vlopt_window_create_child_windows(): Create child windows.
 * @param hWnd Parent window.
 */
static void vlopt_window_create_child_windows(HWND hWnd)
{
	if (vlopt_window_child_windows_created)
		return;
	
	// TODO
	
	vlopt_window_child_windows_created = TRUE;
}



#if 0
/**
 * vlopt_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void vlopt_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(dialog);
	MDP_UNUSED_PARAMETER(user_data);
	
	int rval;
	
	switch (response_id)
	{
		case VLOPT_RESPONSE_RESET:
			// Reset the VDP layer options to the default value.
			rval = vlopt_host_srv->val_set(MDP_VAL_VDP_LAYER_OPTIONS, MDP_VDP_LAYER_OPTIONS_DEFAULT);
			if (rval != MDP_ERR_OK)
			{
				fprintf(stderr, "%s(): Error setting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, rval);
			}
			
			// Reload the VDP layer options.
			vlopt_window_load_options();
			break;
		
		case GTK_RESPONSE_CLOSE:
			// Close.
			vlopt_window_close();
			break;
		
		default:
			// Unknown response ID.
			break;
	}
}


/**
 * vlopt_window_load_options(): Load the options from MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_load_options(void)
{
	int vdp_layer_options = vlopt_host_srv->val_get(MDP_VAL_VDP_LAYER_OPTIONS);
	if (vdp_layer_options < 0)
	{
		fprintf(stderr, "%s(): Error getting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, vdp_layer_options);
		return;
	}
	
	// Go through the options.
	for (unsigned int i = 0; i < VLOPT_OPTIONS_COUNT; i++)
	{
		gboolean flag_enabled = (vdp_layer_options & vlopt_options[i].flag);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(vlopt_checkboxes[i]), flag_enabled);
	}
}


/**
 * vlopt_window_save_options(): Save the options to MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_save_options(void)
{
	int vdp_layer_options = 0;
	
	// Go through the options.
	for (unsigned int i = 0; i < VLOPT_OPTIONS_COUNT; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(vlopt_checkboxes[i])))
			vdp_layer_options |= vlopt_options[i].flag;
	}
	
	// Set the new options.
	int rval = vlopt_host_srv->val_set(MDP_VAL_VDP_LAYER_OPTIONS, vdp_layer_options);
	if (rval != MDP_ERR_OK)
	{
		fprintf(stderr, "%s(): Error setting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, vdp_layer_options);
	}
}


/**
 * vlopt_window_callback_checkbox_toggled(): A checkbox was toggled.
 */
static void vlopt_window_callback_checkbox_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(togglebutton);
	MDP_UNUSED_PARAMETER(user_data);
	
	// Save the current layer options.
	vlopt_window_save_options();
}
#endif
