/***************************************************************************
 * Gens: (Win32) Controller Configuration Window.                          *
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

#include "cc_window.h"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// Gens Win32 resources.
#include "ui/win32/resource.h"

// Gens input variables.
#include "gens_core/io/io.h"
#include "gens_core/io/io_teamplayer.h"

// Input Handler.
#include "input/input.h"
#include "input/input_dinput.hpp"

// TODO: Move DirectInput-specific code to input_dinput.cpp.
#include <dinput.h>

// Unused Parameter macro.
#include "macros/unused.h"


// Window.
HWND cc_window = NULL;
BOOL cc_window_is_configuring = FALSE;

// Window class.
static WNDCLASS	cc_wndclass;

// Window size.
#define CC_WINDOW_WIDTH  580
#define CC_WINDOW_HEIGHT 464
#define CC_FRAME_PORT_WIDTH  236
#define CC_FRAME_PORT_HEIGHT 140

#define CC_FRAME_INPUT_DEVICES_WIDTH  320
#define CC_FRAME_INPUT_DEVICES_HEIGHT 96

#define CC_FRAME_CONFIGURE_WIDTH  CC_FRAME_INPUT_DEVICES_WIDTH
#define CC_FRAME_CONFIGURE_HEIGHT (CC_WINDOW_HEIGHT-8-CC_FRAME_INPUT_DEVICES_HEIGHT-8-24-8-8)

// Command value bases.
#define IDC_CC_CHKTEAMPLAYER	0x1100
#define IDC_CC_CBOPADTYPE	0x1200
#define IDC_CC_OPTCONFIGURE	0x1300
#define IDC_CC_BTNCHANGE	0x1400

// Apply button.
#define IDAPPLY 0x0010

// Window procedure.
static LRESULT CALLBACK cc_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Internal key configuration, which is copied when Save is clicked.
static input_keymap_t cc_key_config[8];

// Current player number being configured.
static int cc_cur_player;

// Monospace font for the current key configuration.
static HFONT	cc_fntMonospace = NULL;

// Widgets.
static HWND	chkTeamplayer[2];
static HWND	lblPlayer[8];
static HWND	cboPadType[8];
static HWND	optConfigure[8];

// Widgets: "Input Devices" frame.
static HWND	lstInputDevices;

// Widgets: "Configure Controller" frame.
static HWND	fraConfigure;
static HWND	lblButton[12];
static HWND	lblCurConfig[12];
static HWND	btnChange[12];

// Widget creation functions.
static void	cc_window_create_child_windows(HWND hWnd);
static void	cc_window_create_controller_port_frame(HWND container, int port);
static void	cc_window_create_input_devices_frame(HWND container);
static void	cc_window_populate_input_devices(HWND lstBox);
static void	cc_window_create_configure_controller_frame(HWND container);

#if 0
// Callbacks.
static void	cc_window_callback_teamplayer_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	cc_window_callback_configure_toggled(GtkToggleButton *togglebutton, gpointer user_data);
static void	cc_window_callback_padtype_changed(GtkComboBox *widget, gpointer user_data);
static void	cc_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data);

// Configuration load/save functions.
static void	cc_window_init(void);
static void	cc_window_save(void);
static void	cc_window_show_configuration(int player);

// Blink handler. (Blinks the current button configuration label when configuring.)
static gboolean cc_window_callback_blink(gpointer data);
#endif


/**
 * cc_window_show(): Show the Controller Configuration window.
 * @param parent Parent window.
 */
void cc_window_show(HWND parent)
{
	if (cc_window)
	{
		// Controller Configuration window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(cc_window, SW_SHOW);
		return;
	}
	
	if (cc_wndclass.lpfnWndProc != cc_window_wndproc)
	{
		// Create the window class.
		cc_wndclass.style = 0;
		cc_wndclass.lpfnWndProc = cc_window_wndproc;
		cc_wndclass.cbClsExtra = 0;
		cc_wndclass.cbWndExtra = 0;
		cc_wndclass.hInstance = ghInstance;
		cc_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
		cc_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		cc_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		cc_wndclass.lpszMenuName = NULL;
		cc_wndclass.lpszClassName = "cc_window";
		
		RegisterClass(&cc_wndclass);
	}
	
	// Create the window.
	cc_window = CreateWindow("cc_window", "Controller Configuration",
				 WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CC_WINDOW_WIDTH, CC_WINDOW_HEIGHT,
				 parent, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(cc_window, CC_WINDOW_WIDTH, CC_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	Win32_centerOnGensWindow(cc_window);
	
	UpdateWindow(cc_window);
	ShowWindow(cc_window, SW_SHOW);
	return;
}


/**
 * cc_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void cc_window_create_child_windows(HWND hWnd)
{
	// Create the controller port frames.
	cc_window_create_controller_port_frame(hWnd, 1);
	cc_window_create_controller_port_frame(hWnd, 2);
	
	// Create the "Input Devices" frame and populate the "Input Devices" listbox.
	cc_window_create_input_devices_frame(hWnd);
	cc_window_populate_input_devices(lstInputDevices);
	
	// Create the "Configure Controller" frame.
	cc_window_create_configure_controller_frame(hWnd);
	
	// Create the dialog buttons.
	
	// OK button.
	HWND btnOK = CreateWindow(WC_BUTTON, "&OK",
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  CC_WINDOW_WIDTH-8-75-8-75-8-75, CC_WINDOW_HEIGHT-8-24,
				  75, 23,
				  hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Apply button.
	HWND btnApply = CreateWindow(WC_BUTTON, "&Apply",
				     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     CC_WINDOW_WIDTH-8-75-8-75, CC_WINDOW_HEIGHT-8-24,
				     75, 23,
				     hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	// Cancel button.
	HWND btnCancel = CreateWindow(WC_BUTTON, "&Cancel",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      CC_WINDOW_WIDTH-8-75, CC_WINDOW_HEIGHT-8-24,
				      75, 23,
				      hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
#if 0
	// Initialize the internal data variables.
	cc_window_init();
	
	// Show the controller configuration for the first player.
	cc_window_show_configuration(0);
#endif
}


/**
 * cc_window_create_controller_port_frame(): Create a controller port frame.
 * @param container Container for the frame.
 * @param port Port number.
 */
static void cc_window_create_controller_port_frame(HWND container, int port)
{
	char tmp[32];
	
	// Top of the frame.
	const int fraPort_top = 8 + ((port-1)*(CC_FRAME_PORT_HEIGHT + 8));
	
	// Create the frame.
	sprintf(tmp, "Port %d", port);
	HWND fraPort = CreateWindow(WC_BUTTON, tmp,
				    WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				    8, fraPort_top, CC_FRAME_PORT_WIDTH, CC_FRAME_PORT_HEIGHT,
				    container, NULL, ghInstance, NULL);
	SetWindowFont(fraPort, fntMain, TRUE);
	
	// Checkbox for enabling teamplayer.
	chkTeamplayer[port-1] = CreateWindow(WC_BUTTON, "Use Teamplayer",
					     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					     8+8, fraPort_top+16, CC_FRAME_PORT_WIDTH-16, 16,
					     container, (HMENU)(IDC_CC_CHKTEAMPLAYER + (port-1)),
					     ghInstance, NULL);
	SetWindowFont(chkTeamplayer[port-1], fntMain, TRUE);
	
	// Player inputs.
	unsigned int i, player;
	
	for (i = 0; i < 4; i++)
	{
		sprintf(tmp, "Player %d%c", port, (i == 0 ? 0x00 : 'A' + i));
		
		// Determine the player number to use for the callback and widget pointer storage.
		if (i == 0)
			player = port - 1;
		else
		{
			if (port == 1)
				player = i + 1;
			else //if (port == 2)
				player = i + 4;
		}
		
		// Player label.
		lblPlayer[player] = CreateWindow(WC_STATIC, tmp,
						 WS_CHILD | WS_VISIBLE | SS_LEFT,
						 8+8, fraPort_top+16+16+4+(i*24)+2, 48, 16,
						 container, NULL, ghInstance, NULL);
		SetWindowFont(lblPlayer[player], fntMain, TRUE);
		
		// Pad type dropdown.
		cboPadType[player] = CreateWindow(WC_COMBOBOX, tmp,
						  WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
						  8+8+48+8, fraPort_top+16+16+4+(i*24), 80, 23*2,
						  container, (HMENU)(IDC_CC_CBOPADTYPE + player),
						  ghInstance, NULL);
		SetWindowFont(cboPadType[player], fntMain, TRUE);
		
		// Pad type dropdown entries.
		ComboBox_AddString(cboPadType[player], "3 buttons");
		ComboBox_AddString(cboPadType[player], "6 buttons");
		
		// "Configure" button.
		optConfigure[player] = CreateWindow(WC_BUTTON, "Configure",
						    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
						    8+8+48+8+80+8, fraPort_top+16+16+4+(i*24), 75, 23,
						    container, (HMENU)(IDC_CC_OPTCONFIGURE + player),
						    ghInstance, NULL);
		SetWindowFont(optConfigure[player], fntMain, TRUE);
	}
}


/**
 * cc_window_create_input_devices_frame(): Create the "Input Devices" frame.
 * @param container Container for the frame.
 */
static void cc_window_create_input_devices_frame(HWND container)
{
	// "Input Devices" frame.
	HWND fraInputDevices = CreateWindow(WC_BUTTON, "Input Devices",
					    WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					    8+CC_FRAME_PORT_WIDTH+8, 8,
					    CC_FRAME_INPUT_DEVICES_WIDTH, CC_FRAME_INPUT_DEVICES_HEIGHT,
					    container, NULL, ghInstance, NULL);
	SetWindowFont(fraInputDevices, fntMain, TRUE);
	
	// Create a listbox for the list of input devices.
	lstInputDevices = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTBOX, NULL,
					 WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_BORDER,
					 8, 16,
					 CC_FRAME_INPUT_DEVICES_WIDTH-16, CC_FRAME_INPUT_DEVICES_HEIGHT-16-8,
					 fraInputDevices, NULL, ghInstance, NULL);
	SetWindowFont(lstInputDevices, fntMain, TRUE);
}


/**
 * cc_window_populate_input_devices(): Populate the "Input Devices" listbox.
 * @param lstBox Listbox to store the input devices in.
 */
static void cc_window_populate_input_devices(HWND lstBox)
{
	// Clear the listbox.
	ListBox_ResetContent(lstBox);
	
	// Add "Keyboard" as the first entry.
	ListBox_AddString(lstBox, "Keyboard");
	
	// Add any detected joysticks to the list model.
	// TODO: This is DirectInput-specific.
	input_dinput_add_joysticks_to_listbox(lstBox);
}


/**
 * cc_window_create_configure_controller_frame(): Create the "Configure Controller" frame.
 * @param container Container for the frame.
 */
static void cc_window_create_configure_controller_frame(HWND container)
{
	// Top and left sides of the frame.
	const int fraConfigure_top = 8+CC_FRAME_INPUT_DEVICES_HEIGHT+8;
	const int fraConfigure_left = 8+CC_FRAME_PORT_WIDTH+8;
	
	// "Configure Controller" frame.
	fraConfigure = CreateWindow(WC_BUTTON, NULL,
				    WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				    fraConfigure_left, fraConfigure_top,
				    CC_FRAME_CONFIGURE_WIDTH, CC_FRAME_CONFIGURE_HEIGHT,
				    container, NULL, ghInstance, NULL);
	SetWindowFont(fraConfigure, fntMain, TRUE);
	
	// Create the widgets for the "Configure Controller" frame.
	unsigned int button;
	char tmp[16];
	for (button = 0; button < 12; button++)
	{
		// Button label.
		sprintf(tmp, "%s:", input_key_names[button]);
		lblButton[button] = CreateWindow(WC_STATIC, tmp,
						 WS_CHILD | WS_VISIBLE | SS_RIGHT,
						 fraConfigure_left+8, fraConfigure_top+16+(button*24)+2,
						 36, 16,
						 container, NULL, ghInstance, NULL);
		SetWindowFont(lblButton[button], fntMain, TRUE);
		
		// Current configuration label.
		lblCurConfig[button] = CreateWindow(WC_STATIC, "Testing 1 2 3",
						    WS_CHILD | WS_VISIBLE | SS_LEFT,
						    fraConfigure_left+8+36+8, fraConfigure_top+16+(button*24)+2,
						    CC_FRAME_CONFIGURE_WIDTH-8-36-8-75-8-8, 16,
						    container, NULL, ghInstance, NULL);
		
		// "Change" button.
		btnChange[button] = CreateWindow(WC_BUTTON, "Change",
						 WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						 fraConfigure_left+CC_FRAME_CONFIGURE_WIDTH-8-75,
						 fraConfigure_top+16+(button*24),
						 75, 23,
						 container, (HMENU)(IDC_CC_BTNCHANGE + button), ghInstance, NULL);
		SetWindowFont(btnChange[button], fntMain, TRUE);
	}
}


/**
 * cc_window_close(): Close the Controller Configuration window.
 */
void cc_window_close(void)
{
	if (!cc_window)
		return;
	
	// Clear the "Configuring" variable.
	cc_window_is_configuring = FALSE;
	
	// Destroy the window.
	DestroyWindow(cc_window);
	cc_window = NULL;
	
	// Delete the monospace font.
	if (cc_fntMonospace)
	{
		DeleteFont(cc_fntMonospace);
		cc_fntMonospace = NULL;
	}
}


#if 0
/**
 * cc_window_init(): Initialize the internal variables.
 */
static void cc_window_init(void)
{
	// Copy the current controller configuration into the internal input_keymap_t array.
	memcpy(&cc_key_config, &input_keymap, sizeof(cc_key_config));
	
	// Set the Teamplayer checkboxes.
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkTeamplayer[0]), (Controller_1_Type & 0x10));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkTeamplayer[1]), (Controller_2_Type & 0x10));
	
	// Set the pad type dropdowns.
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[0]), (Controller_1_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[1]), (Controller_2_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[2]), (Controller_1B_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[3]), (Controller_1C_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[4]), (Controller_1D_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[5]), (Controller_2B_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[6]), (Controller_2C_Type & 0x01));
	gtk_combo_box_set_active(GTK_COMBO_BOX(cboPadType[7]), (Controller_2D_Type & 0x01));
	
	// Run the teamplayer callbacks.
	cc_window_callback_teamplayer_toggled(GTK_TOGGLE_BUTTON(chkTeamplayer[0]), GINT_TO_POINTER(0));
	cc_window_callback_teamplayer_toggled(GTK_TOGGLE_BUTTON(chkTeamplayer[1]), GINT_TO_POINTER(1));
}


/**
 * cc_window_save(): Save the controller configuration.
 */
static void cc_window_save(void)
{
	// Copy the modified controller configuration into the Gens keymap array.
	memcpy(&input_keymap, &cc_key_config, sizeof(input_keymap));
	
	// Clear the Controller Type variables.
	Controller_1_Type = 0;
	Controller_2_Type = 0;
	Controller_1B_Type = 0;
	Controller_1C_Type = 0;
	Controller_1D_Type = 0;
	Controller_2B_Type = 0;
	Controller_2C_Type = 0;
	Controller_2D_Type = 0;
	
	// Save the Teamplayer settings.
	Controller_1_Type |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkTeamplayer[0])) ? 0x10 : 0x00);
	Controller_2_Type |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkTeamplayer[1])) ? 0x10 : 0x00);
	
	// Save the pad type settings.
	Controller_1_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[0])) ? 0x01 : 0x00);
	Controller_2_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[1])) ? 0x01 : 0x00);
	Controller_1B_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[2])) ? 0x01 : 0x00);
	Controller_1C_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[3])) ? 0x01 : 0x00);
	Controller_1D_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[4])) ? 0x01 : 0x00);
	Controller_2B_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[5])) ? 0x01 : 0x00);
	Controller_2C_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[6])) ? 0x01 : 0x00);
	Controller_2D_Type |= (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[7])) ? 0x01 : 0x00);
}


/**
 * cc_window_show_configuration(): Show controller configuration.
 * @param player Player number.
 */
static void cc_window_show_configuration(int player)
{
	if (player < 0 || player > 8)
		return;
	
	char tmp[64], key_name[64];
	
	// Set the current player number.
	cc_cur_player = player;
	
	// Set the "Configure Controller" frame title.
	sprintf(tmp, "<b><i>Configure Player %s</i></b>", &input_player_names[player][1]);
	gtk_label_set_text(GTK_LABEL(lblConfigure), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblConfigure), TRUE);
	
	// Show the key configuration.
	unsigned int button;
	for (button = 0; button < 12; button++)
	{
		input_get_key_name(cc_key_config[player].data[button], &key_name[0], sizeof(key_name));
		sprintf(tmp, "<tt>0x%04X: %s</tt>", cc_key_config[player].data[button], key_name);
		gtk_label_set_text(GTK_LABEL(lblCurConfig[button]), tmp);
		gtk_label_set_use_markup(GTK_LABEL(lblCurConfig[button]), TRUE);
	}
	
	// Enable/Disable the Mode/X/Y/Z buttons, depending on whether the pad is set to 3-button or 6-button.
	gboolean is6button = (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[player])) == 1);
	for (button = 8; button < 12; button++)
	{
		gtk_widget_set_sensitive(lblButton[button], is6button);
		gtk_widget_set_sensitive(lblCurConfig[button], is6button);
		gtk_widget_set_sensitive(btnChange[button], is6button);
	}
}
#endif


/**
 * cc_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK cc_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			cc_window_create_child_windows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				// Set the DirectInput cooperative level.
				input_set_cooperative_level(hWnd);
				
				// Initialize joysticks.
				input_dinput_init_joysticks(hWnd);
			}
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					// TODO
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					// TODO
					break;
				default:
					// TODO
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != cc_window)
				break;
			
			cc_window_is_configuring = FALSE;
			cc_window = NULL;
			
			// Delete the monospace font.
			if (cc_fntMonospace)
			{
				DeleteFont(cc_fntMonospace);
				cc_fntMonospace = NULL;
			}
			
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


#if 0
/**
 * cc_window_callback_teamplayer_toggled(): "Teamplayer" checkbox was toggled.
 * @param togglebutton Button that was toggled.
 * @param user_data Player number.
 */
static void cc_window_callback_teamplayer_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	gboolean active = gtk_toggle_button_get_active(togglebutton);
	
	int port = GPOINTER_TO_INT(user_data);
	if (port < 0 || port > 1)
		return;
	
	int startPort = (port == 0 ? 2: 5);
	
	// If new state is "Disabled", check if any of the buttons to be disabled are currently toggled.
	if (!active)
	{
		if ((cc_cur_player >= startPort) && (cc_cur_player < startPort + 3))
		{
			// One of the teamplayer players is selected.
			// Select the main player for the port.
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optConfigure[port]), TRUE);
		}
	}
	
	// Enable/Disable teamplayer ports for this port.
	int i;
	for (i = startPort; i < startPort + 3; i++)
	{
		gtk_widget_set_sensitive(lblPlayer[i], active);
		gtk_widget_set_sensitive(cboPadType[i], active);
		gtk_widget_set_sensitive(optConfigure[i], active);
	}
}


/**
 * cc_window_callback_configure_toggled(): "Configure" button for a player was toggled.
 * @param togglebutton Button that was toggled.
 * @param user_data Player number.
 */
static void cc_window_callback_configure_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if (!gtk_toggle_button_get_active(togglebutton))
		return;
	
	// Show the controller configuration.
	cc_window_show_configuration(GPOINTER_TO_INT(user_data));
}


/**
 * cc_window_callback_padtype_changed(): Pad Type for a player was changed.
 * @param widget Combo box that was changed.
 * @param user_data Player number.
 */
static void cc_window_callback_padtype_changed(GtkComboBox *widget, gpointer user_data)
{
	int player = GPOINTER_TO_INT(user_data);
	if (player < 0 || player > 8)
		return;
	
	// Check if this player is currently being configured.
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optConfigure[player])))
	{
		// Player is not currently being configured.
		return;
	}
	
	// Player is currently being configured.
	// Enable/Disable the appropriate widgets in the table.
	unsigned int button;
	gboolean is6button = (gtk_combo_box_get_active(widget) == 1);
	for (button = 8; button < 12; button++)
	{
		gtk_widget_set_sensitive(lblButton[button], is6button);
		gtk_widget_set_sensitive(lblCurConfig[button], is6button);
		gtk_widget_set_sensitive(btnChange[button], is6button);
	}
}


/**
 * cc_window_callback_btnChange_clicked(): A "Change" button was clicked.
 * @param button Button that was clicked.
 * @param user_data Button number.
 */
static void cc_window_callback_btnChange_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	
	if (cc_window_is_configuring)
		return;
	
	int btnID = GPOINTER_TO_INT(user_data);
	if (btnID < 0 || btnID >= 12)
		return;
	
	// If pad type is set to 3 buttons, don't allow button IDs >= 8.
	if (gtk_combo_box_get_active(GTK_COMBO_BOX(cboPadType[cc_cur_player])) == 0)
	{
		if (btnID >= 8)
			return;
	}
	
	// Set cc_window_is_configuring to indicate that the key is being configured.
	cc_window_is_configuring = TRUE;
	
	// Set the current configure text.
	gtk_label_set_text(GTK_LABEL(lblCurConfig[btnID]), "<tt>Press a Key...</tt>");
	gtk_label_set_use_markup(GTK_LABEL(lblCurConfig[btnID]), TRUE);
	
	// Set the blink timer for 500 ms.
	g_timeout_add(500, cc_window_callback_blink, GINT_TO_POINTER(btnID));
	
	// Get a key value.
	cc_key_config[cc_cur_player].data[btnID] = input_get_key();
	
	// Set the text of the label with the key name.
	char tmp[64], key_name[64];
	input_get_key_name(cc_key_config[cc_cur_player].data[btnID], &key_name[0], sizeof(key_name));
	sprintf(tmp, "<tt>0x%04X: %s</tt>", cc_key_config[cc_cur_player].data[btnID], key_name);
	gtk_label_set_text(GTK_LABEL(lblCurConfig[btnID]), tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblCurConfig[btnID]), TRUE);
	
	// Key is no longer being configured.
	cc_window_is_configuring = FALSE;
	
	// Make sure the label is visible now.
	gtk_widget_show(lblCurConfig[btnID]);
}


/**
 * cc_window_callback_blink(): Blink handler.
 * Blinks the current button configuration label when configuring.
 * @param data Data set when setting up the timer.
 * @return FALSE to disable the timer; TRUE to continue the timer.
 */
static gboolean cc_window_callback_blink(gpointer data)
{
	int btnID = GPOINTER_TO_INT(data);
	if (btnID < 0 || btnID > 12)
		return FALSE;
	
	if (!cc_window_is_configuring)
	{
		// Not configuring. Show the label and disable the timer.
		gtk_widget_show(lblCurConfig[btnID]);
		return FALSE;
	}
	
	// Invert the label visibility.
	if (GTK_WIDGET_VISIBLE(lblCurConfig[btnID]))
		gtk_widget_hide(lblCurConfig[btnID]);
	else
		gtk_widget_show(lblCurConfig[btnID]);
	
	// If the window is still configuring, keep the timer going.
	return cc_window_is_configuring;
}
#endif
