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
#include "gens/gens_window.h"

// C includes.
#include <stdio.h>
#include <string.h>

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ui/win32/fonts.h"
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
#define CC_WINDOW_WIDTH  600
#define CC_WINDOW_HEIGHT 466

#define CC_FRAME_PORT_WIDTH  236
#define CC_FRAME_PORT_HEIGHT 140

#define CC_FRAME_INPUT_DEVICES_WIDTH  CC_FRAME_PORT_WIDTH
#define CC_FRAME_INPUT_DEVICES_HEIGHT 96

#define CC_FRAME_CONFIGURE_WIDTH  340
#define CC_FRAME_CONFIGURE_HEIGHT 354

#define CC_FRAME_OPTIONS_WIDTH  CC_FRAME_CONFIGURE_WIDTH
#define CC_FRAME_OPTIONS_HEIGHT 56

// Command value bases.
#define IDC_CC_CHKTEAMPLAYER	0x1100
#define IDC_CC_CBOPADTYPE	0x1200
#define IDC_CC_OPTCONFIGURE	0x1300
#define IDC_CC_BTNCHANGE	0x1400

// "Options" buttons.
#define IDC_CC_BTNCHANGEALL	0x1801
#define IDC_CC_BTNCLEARALL	0x1802

// Timer ID.
#define IDT_CONFIGURE_BLINK	0x2000

// Window procedure.
static LRESULT CALLBACK cc_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Internal key configuration, which is copied when Save is clicked.
static input_keymap_t cc_key_config[8];

// Current player number and button being configured.
static int	cc_cur_player;
static int	cc_cur_player_button;

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
static HWND	btnChangeAll;
static HWND	btnClearAll;

// Widgets: "Options" frame.
static HWND	chkRestrictInput;

// Widget creation functions.
static void	cc_window_create_child_windows(HWND hWnd);
static void	cc_window_create_controller_port_frame(HWND container, int port);
static void	cc_window_create_input_devices_frame(HWND container);
static void	cc_window_populate_input_devices(HWND lstBox);
static void	cc_window_create_configure_controller_frame(HWND container);
static void	cc_window_create_options_frame(HWND container);

// Display key name function.
static inline void cc_window_display_key_name(HWND label, uint16_t key);

// Configuration load/save functions.
static void	cc_window_init(void);
static void	cc_window_save(void);
static void	cc_window_show_configuration(int player);

// Callbacks.
static void	cc_window_callback_teamplayer_toggled(int port);
static void	cc_window_callback_padtype_changed(int player);
static void	cc_window_callback_btnChangeAll_clicked(void);
static void	cc_window_callback_btnClearAll_clicked(void);

// Configure a key.
static void	cc_window_configure_key(int player, int button);

// Blink handler. (Blinks the current button configuration label when configuring.)
static void CALLBACK cc_window_callback_blink(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


/**
 * cc_window_show(): Show the Controller Configuration window.
 */
void cc_window_show(void)
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
		cc_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC));
		cc_wndclass.hCursor = NULL;
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
				 gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(cc_window, CC_WINDOW_WIDTH, CC_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	Win32_centerOnGensWindow(cc_window);
	
	UpdateWindow(cc_window);
	ShowWindow(cc_window, SW_SHOW);
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
	
	// Create the "Options" frame.
	cc_window_create_options_frame(hWnd);
	
	// Create the dialog buttons.
	
	// OK button.
	HWND btnOK = CreateWindow(WC_BUTTON, "&OK",
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  CC_WINDOW_WIDTH-8-75-8-75-8-75, CC_WINDOW_HEIGHT-8-24,
				  75, 23,
				  hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Cancel button.
	HWND btnCancel = CreateWindow(WC_BUTTON, "&Cancel",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      CC_WINDOW_WIDTH-8-75-8-75, CC_WINDOW_HEIGHT-8-24,
				      75, 23,
				      hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Apply button.
	HWND btnApply = CreateWindow(WC_BUTTON, "&Apply",
				     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     CC_WINDOW_WIDTH-8-75, CC_WINDOW_HEIGHT-8-24,
				     75, 23,
				     hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	// Initialize the internal data variables.
	cc_window_init();
	
	// Set focus to the "Use Teamplayer" checkbox for Port 1.
	SetFocus(chkTeamplayer[0]);
	
	// Show the controller configuration for the first player.
	Button_SetCheck(optConfigure[0], BST_CHECKED);
	cc_window_show_configuration(0);
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
	const char *tp_label;
	if (port == 1)
		tp_label = "Use Teamplayer / 4-Way Play";
	else
		tp_label = "Use Teamplayer";
	chkTeamplayer[port-1] = CreateWindow(WC_BUTTON, tp_label,
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
	static const int fraInputDevices_top = 8+CC_FRAME_PORT_HEIGHT+8+CC_FRAME_PORT_HEIGHT+8;
	
	// "Input Devices" frame.
	HWND fraInputDevices = CreateWindow(WC_BUTTON, "Detected Input Devices",
					    WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					    8, fraInputDevices_top,
					    CC_FRAME_INPUT_DEVICES_WIDTH, CC_FRAME_INPUT_DEVICES_HEIGHT,
					    container, NULL, ghInstance, NULL);
	SetWindowFont(fraInputDevices, fntMain, TRUE);
	
	// Create a listbox for the list of input devices.
	lstInputDevices = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTBOX, NULL,
					 WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
					 8+8, fraInputDevices_top+16,
					 CC_FRAME_INPUT_DEVICES_WIDTH-16, CC_FRAME_INPUT_DEVICES_HEIGHT-16-8,
					 container, NULL, ghInstance, NULL);
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
	static const int fraConfigure_top = 8;
	static const int fraConfigure_left = 8+CC_FRAME_PORT_WIDTH+8;
	
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
		lblCurConfig[button] = CreateWindow(WC_STATIC, NULL,
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
	
	// Separator between the table and the miscellaneous buttons.
	CreateWindow(WC_STATIC, NULL,
		     WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
		     fraConfigure_left+8, fraConfigure_top+16+12*24+8,
		     CC_FRAME_CONFIGURE_WIDTH-16, 2,
		     container, NULL, ghInstance, NULL);
	
	// "Change All Buttons" button.
	btnChangeAll = CreateWindow(WC_BUTTON, "Change All Buttons",
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				    fraConfigure_left+8, fraConfigure_top+16+12*24+8+2+8,
				    127, 23,
				    container, (HMENU)(IDC_CC_BTNCHANGEALL), ghInstance, NULL);
	SetWindowFont(btnChangeAll, fntMain, TRUE);
	
	// "Clear All Buttons" button.
	btnClearAll = CreateWindow(WC_BUTTON, "Clear All Buttons",
				   WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				   fraConfigure_left+CC_FRAME_CONFIGURE_WIDTH-8-128,
				   fraConfigure_top+16+12*24+8+2+8,
				   127, 23,
				   container, (HMENU)(IDC_CC_BTNCLEARALL), ghInstance, NULL);
	SetWindowFont(btnClearAll, fntMain, TRUE);
}


/**
 * cc_window_create_options_frame(): Create the "Options" frame.
 * @param container Container for the frame.
 */
static void cc_window_create_options_frame(HWND container)
{
	// Top and left sides of the frame.
	static const int fraOptions_top = 8+CC_FRAME_CONFIGURE_HEIGHT+8;
	static const int fraOptions_left = 8+CC_FRAME_PORT_WIDTH+8;
	
	// "Configure Controller" frame.
	HWND fraOptions = CreateWindow(WC_BUTTON, "Options",
				       WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				       fraOptions_left, fraOptions_top,
				       CC_FRAME_OPTIONS_WIDTH, CC_FRAME_OPTIONS_HEIGHT,
				       container, NULL, ghInstance, NULL);
	SetWindowFont(fraOptions, fntMain, TRUE);
	
	// "Restrict Input" checkbox.
	chkRestrictInput = CreateWindow(WC_BUTTON, "&Restrict Input\n(Disables Up+Down, Left+Right)",
					WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_MULTILINE,
					8, 16,
					CC_FRAME_OPTIONS_WIDTH-16, 32,
					fraOptions, NULL, ghInstance, NULL);
	SetWindowFont(chkRestrictInput, fntMain, TRUE);
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


/**
 * cc_window_init(): Initialize the internal variables.
 */
static void cc_window_init(void)
{
	// Copy the current controller configuration into the internal input_keymap_t array.
	memcpy(&cc_key_config, &input_keymap, sizeof(cc_key_config));
	
	// Set the Teamplayer checkboxes.
	Button_SetCheck(chkTeamplayer[0], ((Controller_1_Type & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkTeamplayer[1], ((Controller_2_Type & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	
	// Set the pad type dropdowns.
	ComboBox_SetCurSel(cboPadType[0], (Controller_1_Type & 0x01));
	ComboBox_SetCurSel(cboPadType[1], (Controller_2_Type & 0x01));
	ComboBox_SetCurSel(cboPadType[2], (Controller_1B_Type & 0x01));
	ComboBox_SetCurSel(cboPadType[3], (Controller_1C_Type & 0x01));
	ComboBox_SetCurSel(cboPadType[4], (Controller_1D_Type & 0x01));
	ComboBox_SetCurSel(cboPadType[5], (Controller_2B_Type & 0x01));
	ComboBox_SetCurSel(cboPadType[6], (Controller_2C_Type & 0x01));
	ComboBox_SetCurSel(cboPadType[7], (Controller_2D_Type & 0x01));
	
	// Run the teamplayer callbacks.
	cc_window_callback_teamplayer_toggled(0);
	cc_window_callback_teamplayer_toggled(1);
	
	// Restrict Input.
	Button_SetCheck(chkRestrictInput, (Settings.restrict_input ? BST_CHECKED : BST_UNCHECKED));
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
	Controller_1_Type  |= ((Button_GetCheck(chkTeamplayer[0]) == BST_CHECKED) ? 0x10 : 0x00);
	Controller_2_Type  |= ((Button_GetCheck(chkTeamplayer[1]) == BST_CHECKED) ? 0x10 : 0x00);
	
	// Save the pad type settings.
	Controller_1_Type  |= (ComboBox_GetCurSel(cboPadType[0]) ? 0x01 : 0x00);
	Controller_2_Type  |= (ComboBox_GetCurSel(cboPadType[1]) ? 0x01 : 0x00);
	Controller_1B_Type |= (ComboBox_GetCurSel(cboPadType[2]) ? 0x01 : 0x00);
	Controller_1C_Type |= (ComboBox_GetCurSel(cboPadType[3]) ? 0x01 : 0x00);
	Controller_1D_Type |= (ComboBox_GetCurSel(cboPadType[4]) ? 0x01 : 0x00);
	Controller_2B_Type |= (ComboBox_GetCurSel(cboPadType[5]) ? 0x01 : 0x00);
	Controller_2C_Type |= (ComboBox_GetCurSel(cboPadType[6]) ? 0x01 : 0x00);
	Controller_2D_Type |= (ComboBox_GetCurSel(cboPadType[7]) ? 0x01 : 0x00);
	
	// Restrict Input.
	Settings.restrict_input = ((Button_GetCheck(chkRestrictInput) == BST_CHECKED) ? TRUE : FALSE);
	
	// Rebuild the Teamplayer I/O table.
	Make_IO_Table();
}


/**
 * cc_window_display_key_name(): Display a key name.
 * @param label Label widget.
 * @param key Key value.
 */
static inline void cc_window_display_key_name(HWND label, uint16_t key)
{
	char key_name[32];
	
	input_get_key_name(key, &key_name[0], sizeof(key_name));
	
	#ifdef GENS_DEBUG
		char tmp[64];
		sprintf(tmp, "0x%04X: %s", key, key_name);
		Static_SetText(label, tmp);
	#else
		Static_SetText(label, key_name);
	#endif
	
	SetWindowFont(label, fntMono, TRUE);
}


/**
 * cc_window_show_configuration(): Show controller configuration.
 * @param player Player number.
 */
static void cc_window_show_configuration(int player)
{
	if (player < 0 || player > 8)
		return;
	
	char tmp[64];
	
	// Set the current player number.
	cc_cur_player = player;
	
	// Set the "Configure Controller" frame title.
	sprintf(tmp, "Configure Player %s", &input_player_names[player][1]);
	Button_SetText(fraConfigure, tmp);
	
	// Make sure the "Change All Buttons" and "Clear All Buttons" buttons aren't
	// obscured by the frame label. Not sure if this is intended behavior or
	// if it's simply a bug in Wine.
	// TODO: Test this on Windows.
	InvalidateRect(btnChangeAll, NULL, TRUE);
	InvalidateRect(btnClearAll, NULL, TRUE);
	
	// Show the key configuration.
	unsigned int button;
	for (button = 0; button < 12; button++)
	{
		cc_window_display_key_name(lblCurConfig[button], cc_key_config[player].data[button]);
	}
	
	// Enable/Disable the Mode/X/Y/Z buttons, depending on whether the pad is set to 3-button or 6-button.
	BOOL is6button = (ComboBox_GetCurSel(cboPadType[player]) == 1);
	for (button = 8; button < 12; button++)
	{
		Static_Enable(lblButton[button], is6button);
		Static_Enable(lblCurConfig[button], is6button);
		Button_Enable(btnChange[button], is6button);
	}
}


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
					cc_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					cc_window_save();
					break;
				case IDC_CC_BTNCHANGEALL:
					cc_window_callback_btnChangeAll_clicked();
					break;
				case IDC_CC_BTNCLEARALL:
					cc_window_callback_btnClearAll_clicked();
					break;
				default:
					switch (LOWORD(wParam) & 0xFF00)
					{
						case IDC_CC_CHKTEAMPLAYER:
							cc_window_callback_teamplayer_toggled(LOWORD(wParam) & 0xFF);
							break;
						case IDC_CC_CBOPADTYPE:
							if (HIWORD(wParam) == CBN_SELCHANGE)
								cc_window_callback_padtype_changed(LOWORD(wParam) & 0xFF);
							break;
						case IDC_CC_OPTCONFIGURE:
							cc_window_show_configuration(LOWORD(wParam) & 0xFF);
							break;
						case IDC_CC_BTNCHANGE:
							cc_window_is_configuring = TRUE;
							cc_window_configure_key(cc_cur_player, LOWORD(wParam) & 0xFF);
							cc_window_is_configuring = FALSE;
							break;
						default:
							// Unknown command identifier.
							break;
					}
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


/**
 * cc_window_callback_teamplayer_toggled(): "Teamplayer" checkbox was toggled.
 * @param port Port number of the checkbox that was toggled.
 */
static void cc_window_callback_teamplayer_toggled(int port)
{
	if (port < 0 || port > 1)
		return;
	
	BOOL active = (Button_GetCheck(chkTeamplayer[port]) == BST_CHECKED);
	int startPort = (port == 0 ? 2 : 5);
	
	// If new state is "Disabled", check if any of the buttons to be disabled are currently toggled.
	if (!active)
	{
		if ((cc_cur_player >= startPort) && (cc_cur_player < startPort + 3))
		{
			// One of the teamplayer players is selected.
			// Select the main player for the port.
			Button_SetCheck(optConfigure[port], BST_CHECKED);
			cc_window_show_configuration(port);
		}
	}
	
	// Enable/Disable teamplayer ports for this port.
	int i;
	for (i = startPort; i < startPort + 3; i++)
	{
		Static_Enable(lblPlayer[i], active);
		ComboBox_Enable(cboPadType[i], active);
		Button_Enable(optConfigure[i], active);
	}
}


/**
 * cc_window_callback_padtype_changed(): Pad type for a player was changed.
 * @param player Player number.
 */
static void cc_window_callback_padtype_changed(int player)
{
	if (player < 0 || player > 8)
		return;
	
	// Check if this player is currently being configured.
	if (Button_GetCheck(optConfigure[player]) != BST_CHECKED)
	{
		// Player is not currently being configured.
		return;
	}
	
	// Player is currently being configured.
	// Enable/Disable the appropriate widgets in the table.
	unsigned int button;
	BOOL is6button = (ComboBox_GetCurSel(cboPadType[player]) == 1);
	for (button = 8; button < 12; button++)
	{
		Static_Enable(lblButton[button], is6button);
		Static_Enable(lblCurConfig[button], is6button);
		Button_Enable(btnChange[button], is6button);
	}
}


/**
 * cc_window_configure_key(): Configure a key.
 * @param player Player to configure.
 * @param button Button ID.
 */
static void cc_window_configure_key(int player, int button)
{
	if (!cc_window_is_configuring)
		return;
	
	if (button < 0 || button >= 12)
		return;
	
	// If pad type is set to 3 buttons, don't allow button IDs >= 8.
	if (ComboBox_GetCurSel(cboPadType[player]) == 0)
	{
		if (button >= 8)
			return;
	}
	
	// Set the current button that is being configured.
	cc_cur_player_button = button;
	
	// Set the current configure text.
	Static_SetText(lblCurConfig[button], "Press a Key...");
	
	// Set the blink timer for 500 ms.
	SetTimer(cc_window, IDT_CONFIGURE_BLINK, 500, cc_window_callback_blink);
	
	// Get a key value.
	cc_key_config[player].data[button] = input_get_key();
	
	// Set the text of the label with the key name.
	cc_window_display_key_name(lblCurConfig[button], cc_key_config[player].data[button]);
	
	// Key is no longer being configured.
	cc_cur_player_button = -1;
	
	// Kill the timer.
	KillTimer(cc_window, IDT_CONFIGURE_BLINK);
	
	// Make sure the label is visible now.
	ShowWindow(lblCurConfig[button], SW_SHOW);
	
	// Remove various dialog messages from the window message queue.
	MSG msg;
	while (PeekMessage(&msg, cc_window, WM_KEYDOWN, WM_CHAR, PM_REMOVE)) { }
	while (PeekMessage(&msg, cc_window, WM_COMMAND, WM_COMMAND, PM_REMOVE)) { }
	while (PeekMessage(&msg, cc_window, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) { }
}


/**
 * cc_window_callback_blink(): Blink handler.
 * Blinks the current button configuration label when configuring.
 * @param hWnd HWND of the window.
 * @param uMsg WM_TIMER.
 * @param idEvent Timer identifier.
 * @param dwTime Time elapsed since the system has started.
 */
static void CALLBACK cc_window_callback_blink(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	GENS_UNUSED_PARAMETER(uMsg);
	GENS_UNUSED_PARAMETER(dwTime);
	
	if (idEvent != IDT_CONFIGURE_BLINK)
		return;
	
	if (!cc_window_is_configuring)
	{
		// Not configuring. Show the label and disable the timer.
		ShowWindow(lblCurConfig[cc_cur_player_button], SW_SHOW);
		KillTimer(hWnd, idEvent);
		return;
	}
	
	// Invert the label visibility.
	if (IsWindowVisible(lblCurConfig[cc_cur_player_button]))
		ShowWindow(lblCurConfig[cc_cur_player_button], SW_HIDE);
	else
		ShowWindow(lblCurConfig[cc_cur_player_button], SW_SHOW);
}


/**
 * cc_window_callback_btnChangeAll_clicked(): "Change All Buttons" button was clicked.
 */
static void cc_window_callback_btnChangeAll_clicked(void)
{
	if (cc_window_is_configuring)
		return;
	
	if (cc_cur_player < 0 || cc_cur_player > 8)
		return;
	
	// Number of buttons to configure.
	int btnCount = (ComboBox_GetCurSel(cboPadType[cc_cur_player]) == 1 ? 12 : 8);
	
	// Set the "Configuring" flag.
	cc_window_is_configuring = TRUE;
	
	int i;
	MSG msg;
	for (i = 0; i < btnCount; i++)
	{
		// Sleep for 250 ms between button presses.
		if (i != 0)
		{
			// Process WM_PAINT messages.
			while (PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
			{
				if (!GetMessage(&msg, NULL, 0, 0))
					close_gens();
				
				// Process the message.
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			
			// Sleep for 250 ms.
			Sleep(250);
		}
		
		cc_window_configure_key(cc_cur_player, i);
		
		if (!cc_window)
		{
			// Window has closed.
			break;
		}
	}
	
	// Unset the "Configuring" flag.
	cc_window_is_configuring = FALSE;
}


/**
 * cc_window_callback_btnClearAll_clicked(): "Clear All Buttons" button was clicked.
 */
static void cc_window_callback_btnClearAll_clicked(void)
{
	if (cc_cur_player < 0 || cc_cur_player > 8)
		return;
	
	// Clear all buttons for the current player.
	memset(&cc_key_config[cc_cur_player], 0x00, sizeof(cc_key_config[cc_cur_player]));
	
	// Show the cleared configuration.
	cc_window_show_configuration(cc_cur_player);
}
