/***************************************************************************
 * MDP: Blargg's NTSC renderer. (Window Code) (Win32)                      *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
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

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// C includes.
#include <math.h>
#include <stdio.h>

#include "mdp_render_blargg_ntsc.h"
#include "mdp_render_blargg_ntsc_plugin.h"
#include "ntsc_window.h"
#include "md_ntsc.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// MDP Win32 convenience functions.
#include "mdp/mdp_win32.h"

// Presets.
typedef struct _ntsc_preset_t
{
	const char *name;
	const md_ntsc_setup_t *setup;
} ntsc_preset_t;

static const ntsc_preset_t ntsc_presets[] =
{
	{"Composite",	&md_ntsc_composite},
	{"S-Video",	&md_ntsc_svideo},
	{"RGB",		&md_ntsc_rgb},
	{"Monochrome",	&md_ntsc_monochrome},
	{"Custom",	NULL},
	{NULL, NULL}
};

// Adjustment controls.
typedef struct _ntsc_ctrl_t
{
	const char *name;
	const int min;
	const int max;
	const int step;
} ntsc_ctrl_t;

#define NTSC_CTRL_COUNT 10
static const ntsc_ctrl_t ntsc_controls[NTSC_CTRL_COUNT + 1] =
{
	{"&Hue",		-180, 180, 1},
	{"&Saturation",		0, 200, 5},
	{"&Contrast",		-100, 100, 5},
	{"&Brightness",		-100, 100, 5},
	{"S&harpness",		-100, 100, 5},
	
	// "Advanced" parameters.
	{"&Gamma",		50, 150, 5},
	{"&Resolution",		-100, 100, 5},
	{"&Artifacts",		-100, 100, 5},
	{"Color &Fringing",	-100, 100, 5},
	{"Color B&leed",	-100, 100, 5},
	
	{NULL, 0, 0, 0}
};

// Window.
static HWND ntsc_window = NULL;
static WNDCLASS ntsc_window_wndclass;

// Widgets.
static HWND cboPresets;
static HWND chkScanline;
static HWND chkInterp;
static HWND lblCtrlValues[NTSC_CTRL_COUNT];
static HWND hscCtrlValues[NTSC_CTRL_COUNT];

// Widget IDs.
#define IDC_NTSC_TRACKBAR 0x1000
#define IDC_NTSC_PRESETS  0x1100

// Window Procedure.
static LRESULT CALLBACK ntsc_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Create Child Windows function.
static void ntsc_window_create_child_windows(HWND hWnd);
static BOOL ntsc_window_child_windows_created;

// Font.
static HFONT ntsc_hFont = NULL;

// Callbacks.
#if 0
static void	ntsc_window_callback_cboPresets_changed(GtkComboBox *widget, gpointer user_data);
#endif
static void	ntsc_window_callback_hscCtrlValues_value_changed(int setting, BOOL update_setup);
#if 0
static void	ntsc_window_callback_chkScanline_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	ntsc_window_callback_chkInterp_toggled(GtkToggleButton *togglebutton, gpointer user_data);
#endif

// Setting handling functions.
static void ntsc_window_load_settings(void);

// Window size.
#define NTSC_WINDOW_WIDTH  360
#define NTSC_WINDOW_HEIGHT 360

// HINSTANCE.
// TODO: Move to DllMain().
static HINSTANCE ntsc_hInstance;


/**
 * ntsc_window_show(): Show the NTSC Plugin Options window.
 * @param parent Parent window.
 */
void ntsc_window_show(void *parent)
{
	if (ntsc_window)
	{
		// NTSC Plugin Options window is already visible.
		// Set focus.
		// TODO
		return;
	}
	
	ntsc_window_child_windows_created = FALSE;
	
	// If no HINSTANCE was specified, use the main executable's HINSTANCE.
	if (!ntsc_hInstance)
		ntsc_hInstance = GetModuleHandle(NULL);
	
	// Create the window class.
	if (ntsc_window_wndclass.lpfnWndProc != ntsc_window_wndproc)
	{
		ntsc_window_wndclass.style = 0;
		ntsc_window_wndclass.lpfnWndProc = ntsc_window_wndproc;
		ntsc_window_wndclass.cbClsExtra = 0;
		ntsc_window_wndclass.cbWndExtra = 0;
		ntsc_window_wndclass.hInstance = ntsc_hInstance;
		ntsc_window_wndclass.hIcon = NULL; //LoadIcon(ntsc_hInstance, MAKEINTRESOURCE(IDI_NTSC));
		ntsc_window_wndclass.hCursor = NULL;
		ntsc_window_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		ntsc_window_wndclass.lpszMenuName = NULL;
		ntsc_window_wndclass.lpszClassName = TEXT("ntsc_window_wndclass");
		
		RegisterClass(&ntsc_window_wndclass);
	}
	
	// Create the font.
	ntsc_hFont = mdp_win32_get_message_font();
	
	// Create the window.
	ntsc_window = CreateWindow(TEXT("ntsc_window_wndclass"), TEXT("Blargg's NTSC Filter"),
				   WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				   CW_USEDEFAULT, CW_USEDEFAULT,
				   NTSC_WINDOW_WIDTH, NTSC_WINDOW_HEIGHT,
				   (HWND)parent, NULL, ntsc_hInstance, NULL);
	
	// Window adjustment.
	mdp_win32_set_actual_window_size(ntsc_window, NTSC_WINDOW_WIDTH, NTSC_WINDOW_HEIGHT);
	mdp_win32_center_on_window(ntsc_window, (HWND)parent);
	
	UpdateWindow(ntsc_window);
	ShowWindow(ntsc_window, TRUE);
	
	// Register the window with MDP Host Services.
	ntsc_host_srv->window_register(&mdp, ntsc_window);
}


#define NTSC_WIDGETNAME_WIDTH  88
#define NTSC_WIDGETNAME_HEIGHT 16
#define NTSC_VALUELABEL_WIDTH  32
#define NTSC_VALUELABEL_HEIGHT 16
#define NTSC_TRACKBAR_WIDTH  (NTSC_WINDOW_WIDTH-8-16-NTSC_WIDGETNAME_WIDTH-8-NTSC_VALUELABEL_WIDTH-8)
#define NTSC_TRACKBAR_HEIGHT 24
#define NTSC_TRACKBAR_STYLE  (WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_BOTTOM)
/**
 * ntsc_window_create_child_windows(): Create child windows.
 * @param hWnd Parent window.
 */
static void ntsc_window_create_child_windows(HWND hWnd)
{
	if (ntsc_window_child_windows_created)
		return;
	
	// Create the main frame.
	HWND grpBox = CreateWindow(WC_BUTTON, TEXT("NTSC Configuration"),
				   WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				   8, 8, NTSC_WINDOW_WIDTH-16, NTSC_WINDOW_HEIGHT-8-16-24,
				   hWnd, NULL, ntsc_hInstance, NULL);
	SetWindowFont(grpBox, ntsc_hFont, TRUE);
	
	// Add a label for the presets dropdown.
	HWND lblPresets = CreateWindow(WC_STATIC, TEXT("&Presets:"),
				       WS_CHILD | WS_VISIBLE | SS_LEFT,
				       8+8, 8+16+4, 64, 16,
				       hWnd, NULL, ntsc_hInstance, NULL);
	SetWindowFont(lblPresets, ntsc_hFont, TRUE);
	
	// Add the presets dropdown.
	cboPresets = CreateWindow(WC_COMBOBOX, NULL,
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
				  8+64+8, 8+16, 104, 23*((sizeof(ntsc_presets) / sizeof(ntsc_preset_t)) - 1),
				  hWnd, (HMENU)IDC_NTSC_PRESETS, ntsc_hInstance, NULL);
	SetWindowFont(cboPresets, ntsc_hFont, TRUE);
	
	unsigned int i = 0;
	while (ntsc_presets[i].name)
	{
		ComboBox_AddString(cboPresets, ntsc_presets[i].name);
		i++;
	}
	
	// Create the adjustment widgets.
	int hscTop = 8+16+24;
	i = 0;
	while (ntsc_controls[i].name)
	{
		// Label.
		HWND lblWidgetName = CreateWindow(WC_STATIC, ntsc_controls[i].name,
						  WS_CHILD | WS_VISIBLE | SS_LEFT,
						  8+8, hscTop+4,
						  NTSC_WIDGETNAME_WIDTH, NTSC_WIDGETNAME_HEIGHT,
						  hWnd, NULL, ntsc_hInstance, NULL);
		SetWindowFont(lblWidgetName, ntsc_hFont, TRUE);
		
		// Value Label.
		lblCtrlValues[i] = CreateWindow(WC_STATIC, NULL,
						WS_CHILD | WS_VISIBLE | SS_RIGHT,
						8+8+NTSC_WIDGETNAME_WIDTH+8, hscTop+4,
						NTSC_VALUELABEL_WIDTH, NTSC_VALUELABEL_HEIGHT,
						hWnd, NULL, ntsc_hInstance, NULL);
		SetWindowFont(lblCtrlValues[i], ntsc_hFont, TRUE);
		
		// Trackbar.
		hscCtrlValues[i] = CreateWindow(TRACKBAR_CLASS, NULL, NTSC_TRACKBAR_STYLE,
						8+8+NTSC_WIDGETNAME_WIDTH+8+NTSC_VALUELABEL_WIDTH, hscTop,
						NTSC_TRACKBAR_WIDTH, NTSC_TRACKBAR_HEIGHT,
						hWnd, (HMENU)(IDC_NTSC_TRACKBAR + i), ntsc_hInstance, NULL);
		SendMessage(hscCtrlValues[i], TBM_SETPAGESIZE, 0, ntsc_controls[i].step);
		SendMessage(hscCtrlValues[i], TBM_SETRANGE, TRUE, MAKELONG(ntsc_controls[i].min, ntsc_controls[i].max));
		SendMessage(hscCtrlValues[i], TBM_SETPOS, TRUE, 0);
		
		// Initialize the value label.
		// TODO
		//ntsc_window_callback_hscCtrlValues_value_changed(GTK_RANGE(hscCtrlValues[i]), GINT_TO_POINTER(i));
		
		// Next widget.
		hscTop += 26;
		i++;
	}
	
#if 0
	// Add an HBox for the presets dropdown.
	GtkWidget *hboxPresets = gtk_hbox_new(FALSE, 8);
	gtk_widget_show(hboxPresets);
	gtk_box_pack_start(GTK_BOX(vboxFrame), hboxPresets, FALSE, FALSE, 0);
	
	// Add a label for the presets dropdown.
	GtkWidget *lblPresets = gtk_label_new_with_mnemonic("_Presets:");
	gtk_widget_show(lblPresets);
	gtk_box_pack_start(GTK_BOX(hboxPresets), lblPresets, FALSE, FALSE, 0);
	
	// Add the presets dropdown.
	cboPresets = gtk_combo_box_new_text();
	unsigned int i = 0;
	while (ntsc_presets[i].name)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(cboPresets), ntsc_presets[i].name);
		i++;
	}
	gtk_widget_show(cboPresets);
	gtk_box_pack_start(GTK_BOX(hboxPresets), cboPresets, FALSE, FALSE, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(lblPresets), cboPresets);
	g_signal_connect((gpointer)cboPresets, "changed",
			 G_CALLBACK(ntsc_window_callback_cboPresets_changed), NULL);
	
	// Scanlines checkbox.
	chkScanline = gtk_check_button_new_with_mnemonic("S_canlines");
	gtk_widget_show(chkScanline);
	gtk_box_pack_start(GTK_BOX(hboxPresets), chkScanline, FALSE, FALSE, 0);
	g_signal_connect((gpointer)chkScanline, "toggled",
			 G_CALLBACK(ntsc_window_callback_chkScanline_toggled), NULL);
	
	// Interpolation checkbox.
	chkInterp = gtk_check_button_new_with_mnemonic("_Interpolation");
	gtk_widget_show(chkInterp);
	gtk_box_pack_start(GTK_BOX(hboxPresets), chkInterp, FALSE, FALSE, 0);
	g_signal_connect((gpointer)chkInterp, "toggled",
			 G_CALLBACK(ntsc_window_callback_chkInterp_toggled), NULL);
	
	// Create a table for the adjustment widgets.
	// First column: Name
	// Second column: Widget
	GtkWidget *tblWidgets = gtk_table_new(NTSC_CTRL_COUNT, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(tblWidgets), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tblWidgets), 8);
	gtk_box_pack_start(GTK_BOX(vboxFrame), tblWidgets, TRUE, TRUE, 0);
	
	// Create the widgets.
	i = 0;
	while (ntsc_controls[i].name)
	{
		// Label alignment.
		GtkWidget *alignWidgetName = gtk_alignment_new(0.0f, 0.5f, 0, 0);
		gtk_widget_show(alignWidgetName);
		gtk_table_attach(GTK_TABLE(tblWidgets), alignWidgetName,
				 0, 1, i, i + 1,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Label.
		GtkWidget *lblWidgetName = gtk_label_new_with_mnemonic(ntsc_controls[i].name);
		gtk_widget_show(lblWidgetName);
		gtk_container_add(GTK_CONTAINER(alignWidgetName), lblWidgetName);
		
		// Value Label alignment.
		GtkWidget *alignCtrlValue = gtk_alignment_new(1.0f, 0.5f, 0, 0);
		gtk_widget_set_size_request(alignCtrlValue, 40, -1);
		gtk_widget_show(alignWidgetName);
		gtk_table_attach(GTK_TABLE(tblWidgets), alignCtrlValue,
				 1, 2, i, i + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		
		// Value Label.
		lblCtrlValues[i] = gtk_label_new(NULL);
		gtk_widget_show(lblCtrlValues[i]);
		gtk_container_add(GTK_CONTAINER(alignCtrlValue), lblCtrlValues[i]);
		
		// Adjustment object.
		GtkObject *adjWidget = gtk_adjustment_new(0, ntsc_controls[i].min,
							  ntsc_controls[i].max,
							  ntsc_controls[i].step,
							  ntsc_controls[i].step * 2, 0);
		
		// GtkHScale
		hscCtrlValues[i] = gtk_hscale_new(GTK_ADJUSTMENT(adjWidget));
		gtk_scale_set_draw_value(GTK_SCALE(hscCtrlValues[i]), FALSE);
		gtk_widget_set_size_request(hscCtrlValues[i], 256, -1);
		gtk_widget_show(hscCtrlValues[i]);
		gtk_label_set_mnemonic_widget(GTK_LABEL(lblWidgetName), hscCtrlValues[i]);
		gtk_table_attach(GTK_TABLE(tblWidgets), hscCtrlValues[i],
				 2, 3, i, i + 1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(0), 0, 0);
		g_signal_connect((gpointer)hscCtrlValues[i], "value-changed",
				 G_CALLBACK(ntsc_window_callback_hscCtrlValues_value_changed),
				 GINT_TO_POINTER(i));
		
		// Initialize the value label.
		ntsc_window_callback_hscCtrlValues_value_changed(GTK_RANGE(hscCtrlValues[i]), GINT_TO_POINTER(i));
		
		// Next widget.
		i++;
	}
#endif
	
	// Create the "Close" button.
	HWND btnClose = CreateWindow(WC_BUTTON, TEXT("&Close"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     NTSC_WINDOW_WIDTH-75-8, NTSC_WINDOW_HEIGHT-24-8, 75, 23,
				     hWnd, (HMENU)IDCLOSE, ntsc_hInstance, NULL);
	SetWindowFont(btnClose, ntsc_hFont, TRUE);
	
	// Load the current settings.
	ntsc_window_load_settings();
	
	// Child windows created.
	ntsc_window_child_windows_created = TRUE;
}


/**
 * ntsc_wndproc(): Window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK ntsc_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int rval;
	
	switch (message)
	{
		case WM_CREATE:
			ntsc_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (wParam)
			{
				case IDCLOSE:
					// Close.
					ntsc_window_close();
					break;
				
				default:
					break;
			}
			
			break;
		
		case WM_DESTROY:
			ntsc_window_close();
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * ntsc_window_close(): Close the NTSC Plugin Options window.
 */
void ntsc_window_close(void)
{
	if (!ntsc_window)
		return;
	
	// Unregister the window from MDP Host Services.
	ntsc_host_srv->window_unregister(&mdp, ntsc_window);
	
	// Destroy the window.
	HWND tmp = ntsc_window;
	ntsc_window = NULL;
	DestroyWindow(tmp);
	
	// Delete the font.
	DeleteFont(ntsc_hFont);
	ntsc_hFont = NULL;
}


/**
 * ntsc_window_load_settings(): Load the NTSC settings.
 */
static void ntsc_window_load_settings(void)
{
	// Set the preset dropdown box.
	int i = 0;
	while (ntsc_presets[i].name)
	{
		if (!ntsc_presets[i].setup)
		{
			// "Custom". This is the last item in the predefined list.
			// Since the current setup doesn't match anything else,
			// it must be a custom setup.
			ComboBox_SetCurSel(cboPresets, i);
			break;
		}
		else
		{
			// Check if this preset matches the current setup.
			if (!memcmp(&mdp_md_ntsc_setup, ntsc_presets[i].setup, sizeof(mdp_md_ntsc_setup)))
			{
				// Match found!
				ComboBox_SetCurSel(cboPresets, i);
				break;
			}
		}
		
		// Next preset.
		i++;
	}
	
	// Scanlines / Interpolation
	Button_SetCheck(chkScanline, (mdp_md_ntsc_scanline ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkInterp, (mdp_md_ntsc_interp ? BST_CHECKED : BST_UNCHECKED));
	
	// Load all settings.
	SendMessage(hscCtrlValues[0], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.hue * 180.0));
	SendMessage(hscCtrlValues[1], TBM_SETPOS, TRUE, (int)((mdp_md_ntsc_setup.saturation + 1.0) * 100.0));
	SendMessage(hscCtrlValues[2], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.contrast * 100.0));
	SendMessage(hscCtrlValues[3], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.brightness * 100.0));
	SendMessage(hscCtrlValues[4], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.sharpness * 100.0));
	SendMessage(hscCtrlValues[5], TBM_SETPOS, TRUE, (int)(((mdp_md_ntsc_setup.gamma / 2.0) + 1.0) * 100.0));
	SendMessage(hscCtrlValues[6], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.resolution * 100.0));
	SendMessage(hscCtrlValues[7], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.artifacts * 100.0));
	SendMessage(hscCtrlValues[8], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.fringing * 100.0));
	SendMessage(hscCtrlValues[9], TBM_SETPOS, TRUE, (int)(mdp_md_ntsc_setup.bleed * 100.0));
	
	// Display all settings.
	for (i = 0; i < NTSC_CTRL_COUNT; i++)
	{
		ntsc_window_callback_hscCtrlValues_value_changed(i, FALSE);
	}
}


#if 0
/**
 * ntsc_window_callback_cboPresets_changed(): The "Presets" combo box has been changed.
 * @param widget
 * @param user_data
 */
static void ntsc_window_callback_cboPresets_changed(GtkComboBox *widget, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(user_data);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	// Load the specified preset setup.
	int i = gtk_combo_box_get_active(widget);
	if (i == -1 || i >= (int)(sizeof(ntsc_presets) / sizeof(ntsc_preset_t)))
		return;
	
	if (!ntsc_presets[i].setup)
		return;
	
	// Reinitialize the NTSC filter with the new settings.
	mdp_md_ntsc_setup = *(ntsc_presets[i].setup);
	mdp_md_ntsc_reinit_setup();
	
	// Load the new settings in the window.
	ntsc_window_load_settings();
}
#endif


/**
 * ntsc_window_callback_hscCtrlValues_value_changed(): One of the adjustment controls has been changed.
 * @param setting Setting ID.
 * @param update_setup If TRUE, updates NTSC setup.
 */
static void ntsc_window_callback_hscCtrlValues_value_changed(int setting, BOOL update_setup)
{
	if (setting < 0 || setting >= NTSC_CTRL_COUNT)
		return;
	
	// Update the label for the adjustment widget.
	char tmp[16];
	double val = SendMessage(hscCtrlValues[setting], TBM_GETPOS, 0, 0);
	
	// Adjust the value to have the appropriate number of decimal places.
	if (setting == 0)
	{
		// Hue. No decimal places.
		snprintf(tmp, sizeof(tmp), "%0.0f\xB0", val);
	}
	else
	{
		// Other adjustment. 2 decimal places.
		val /= 100;
		snprintf(tmp, sizeof(tmp), "%0.2f", val);
	}
	
	Static_SetText(lblCtrlValues[setting], tmp);
	
	if (!update_setup)
		return;
	
	// Adjust the NTSC filter.
	switch (setting)
	{
		case 0:
			mdp_md_ntsc_setup.hue = val / 180.0;
			break;
		case 1:
			mdp_md_ntsc_setup.saturation = val - 1.0;
			break;
		case 2:
			mdp_md_ntsc_setup.contrast = val;
			break;
		case 3:
			mdp_md_ntsc_setup.brightness = val;
			break;
		case 4:
			mdp_md_ntsc_setup.sharpness = val;
			break;
		case 5:
			mdp_md_ntsc_setup.gamma = (val - 1.0) * 2.0;
			break;
		case 6:
			mdp_md_ntsc_setup.resolution = val;
			break;
		case 7:
			mdp_md_ntsc_setup.artifacts = val;
			break;
		case 8:
			mdp_md_ntsc_setup.fringing = val;
			break;
		case 9:
			mdp_md_ntsc_setup.bleed = val;
			break;
		default:
			return;
	}
	
	// Set the "Presets" dropdown to "Custom".
	ComboBox_SetCurSel(cboPresets, 4);
	
	// Reinitialize the NTSC filter with the new settings.
	mdp_md_ntsc_reinit_setup();
}


#if 0
/**
 * ntsc_window_callback_chkScanline_toggled): The "Scanlines" checkbox was toggled.
 * @param togglebutton
 * @param user_data
 */
static void ntsc_window_callback_chkScanline_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(user_data);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	mdp_md_ntsc_scanline = gtk_toggle_button_get_active(togglebutton);
}


/**
 * ntsc_window_callback_chkScanline_toggled): The "Interpolation" checkbox was toggled.
 * @param togglebutton
 * @param user_data
 */
static void ntsc_window_callback_chkInterp_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(user_data);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	mdp_md_ntsc_interp = gtk_toggle_button_get_active(togglebutton);
}
#endif
