/***************************************************************************
 * MDP: Game Genie. (Window Code) (Win32)                                  *
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

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <list>
using std::list;

#include "gg_window.hpp"
#include "gg_plugin.h"
#include "gg.hpp"
#include "gg_code.h"
#include "gg_engine.hpp"

// Win32-specific includes.
#include "gg_dllmain.h"
#include "resource.h"

// MDP includes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"
#include "libgsft/gsft_szprintf.h"

// Window.
static HWND gg_window = NULL;
static WNDCLASS gg_window_wndclass;
static bool gg_window_child_windows_created = false;

// Window size.
#define GG_WINDOW_WIDTH  508
#define GG_WINDOW_HEIGHT 316

// Window procedure.
static LRESULT CALLBACK gg_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	txtCode;
static HWND	txtName;
static HWND	lstCodes;

// Old window procedures for txtCode and txtName.
static WNDPROC	txtCode_old_wndproc;
static WNDPROC	txtName_old_wndproc;

// Widget creation functions.
static void	gg_window_create_child_windows(HWND hWnd);
static void	gg_window_create_lstCodes(HWND container);

// Button IDs.
#define IDAPPLY			0x000F
#define IDC_BTNADDCODE		0x0010
#define IDC_BTNDELETE		0x0011
#define IDC_BTNDEACTIVATEALL	0x0012

// Win32 instance and font.
static HFONT gg_hFont = NULL;
static HFONT gg_hFont_title = NULL;

// Code load/save functions.
static void	gg_window_init(void);
static void	gg_window_save(void);

// Callbacks.
static LRESULT CALLBACK gg_window_wndproc_textbox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void	gg_window_callback_delete(void);
static void	gg_window_callback_deactivate_all(void);

// Miscellaneous.
static int	gg_window_add_code_from_textboxes(void);
static int	gg_window_add_code(const gg_code_t *gg_code, const char *name);


/**
 * gg_window_show(): Show the Game Genie window.
 * @param parent Parent window.
 */
void gg_window_show(void *parent)
{
	if (gg_window)
	{
		// Controller Configuration window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(gg_window, SW_SHOW);
		return;
	}
	
	// Initialize the Win32 Unicode Translation Layer.
	w32u_init();
	
	gg_window_child_windows_created = false;
	
	// If no HINSTANCE was specified, use the main executable's HINSTANCE.
	if (!gg_hInstance)
		gg_hInstance = pGetModuleHandleU(NULL);
	
	// Create the window class.
	if (gg_window_wndclass.lpfnWndProc != gg_window_wndproc)
	{
		gg_window_wndclass.style = 0;
		gg_window_wndclass.lpfnWndProc = gg_window_wndproc;
		gg_window_wndclass.cbClsExtra = 0;
		gg_window_wndclass.cbWndExtra = 0;
		gg_window_wndclass.hInstance = gg_hInstance;
		gg_window_wndclass.hIcon = LoadIconA(gg_hInstance, MAKEINTRESOURCE(IDI_GAME_GENIE));
		gg_window_wndclass.hCursor = NULL;
		gg_window_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		gg_window_wndclass.lpszMenuName = NULL;
		gg_window_wndclass.lpszClassName = "gg_window_wndclass";
		
		pRegisterClassU(&gg_window_wndclass);
	}
	
	// Create the fonts.
	gg_hFont = gsft_win32_gdi_get_message_font();
	gg_hFont_title = gsft_win32_gdi_get_title_font();
	
	// Create the window.
	gg_window = pCreateWindowU("gg_window_wndclass", "Game Genie",
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					GG_WINDOW_WIDTH, GG_WINDOW_HEIGHT,
					(HWND)parent, NULL, gg_hInstance, NULL);
	
	// Window adjustment.
	gsft_win32_set_actual_window_size(gg_window, GG_WINDOW_WIDTH, GG_WINDOW_HEIGHT);
	gsft_win32_center_on_window(gg_window, (HWND)parent);
	
	UpdateWindow(gg_window);
	ShowWindow(gg_window, true);
	
	// Register the window with MDP Host Services.
	gg_host_srv->window_register(&mdp, gg_window);
}

/**
 * gg_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void gg_window_create_child_windows(HWND hWnd)
{
	if (gg_window_child_windows_created)
		return;
	
	// Strings.
	static const char strInfoTitle[] = "Information about Game Genie / Patch codes";
	static const char strInfo[] =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA:DDDD (address:data)";
	
	// Header label.
	HWND lblInfoTitle = pCreateWindowU(WC_STATIC, strInfoTitle,
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						8, 8,
						GG_WINDOW_WIDTH-16, 16,
						hWnd, NULL, gg_hInstance, NULL);
	SetWindowFontU(lblInfoTitle, gg_hFont_title, true);
	
	// Description label.
	HWND lblInfo = pCreateWindowU(WC_STATIC, strInfo,
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 8+16,
					GG_WINDOW_WIDTH-16, 68,
					hWnd, NULL, gg_hInstance, NULL);
	SetWindowFontU(lblInfo, gg_hFont, true);
	
	// "Code" label.
	HWND lblCode = pCreateWindowU(WC_STATIC, "Code",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 8+16+68+8, 32, 16,
					hWnd, NULL, gg_hInstance, NULL);
	SetWindowFontU(lblCode, gg_hFont, true);
	
	// "Code" textbox.
	txtCode = pCreateWindowExU(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
					8+32+8, 8+16+68+8,
					GG_WINDOW_WIDTH-(8+32+8+64+8+8+16), 20,
					hWnd, NULL, gg_hInstance, NULL);
	SetWindowFontU(txtCode, gg_hFont, true);
	Edit_LimitTextU(txtCode, 17);
	txtCode_old_wndproc = (WNDPROC)pSetWindowLongPtrU(
						txtCode, GWL_WNDPROC,
						(LONG_PTR)gg_window_wndproc_textbox);
	
	// "Name" label.
	HWND lblName = pCreateWindowU(WC_STATIC, "Name",
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 8+16+68+8+24, 32, 16,
					hWnd, NULL, gg_hInstance, NULL);
	SetWindowFontU(lblName, gg_hFont, true);
	
	// "Name" textbox.
	txtName = pCreateWindowExU(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
					8+32+8, 8+16+68+8+24,
					GG_WINDOW_WIDTH-(8+32+8+64+8+8+16), 20,
					hWnd, NULL, gg_hInstance, NULL);
	SetWindowFontU(txtName, gg_hFont, true);
	txtName_old_wndproc = (WNDPROC)pSetWindowLongPtrU(
						txtName, GWL_WNDPROC,
						(LONG_PTR)gg_window_wndproc_textbox);
	
	// "Add Code" button.
	HWND btnAddCode = pCreateWindowU(WC_BUTTON, "Add C&ode",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						GG_WINDOW_WIDTH-(64+8+16), 8+16+68+8,
						63+16, 20,
						hWnd, (HMENU)IDC_BTNADDCODE, gg_hInstance, NULL);
	SetWindowFontU(btnAddCode, gg_hFont, true);
	
	// Create the ListView for the Game Genie codes.
	gg_window_create_lstCodes(hWnd);
	
	// Create the dialog buttons.
	static const int btnTop = GG_WINDOW_HEIGHT-24-8;
	
	// "Deactivate All" button.
	HWND btnDeactivateAll = pCreateWindowU(WC_BUTTON, "Deac&tivate All",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						8, btnTop,
						91, 24,
						hWnd, (HMENU)IDC_BTNDEACTIVATEALL, gg_hInstance, NULL);
	SetWindowFontU(btnDeactivateAll, gg_hFont, true);
	
	// "Delete" button.
	HWND btnDelete = pCreateWindowU(WC_BUTTON, "&Delete",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					8+91+8, btnTop,
					75, 24,
					hWnd, (HMENU)IDC_BTNDELETE, gg_hInstance, NULL);
	SetWindowFontU(btnDelete, gg_hFont, true);
	
	// "OK" button.
	HWND btnOK = pCreateWindowU(WC_BUTTON, "&OK",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					GG_WINDOW_WIDTH-(8+75+8+75+8+75), btnTop,
					75, 24,
					hWnd, (HMENU)IDOK, gg_hInstance, NULL);
	SetWindowFontU(btnOK, gg_hFont, true);
	
	// "Cancel" button.
	HWND btnCancel = pCreateWindowU(WC_BUTTON, "&Cancel",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					GG_WINDOW_WIDTH-(8+75+8+75), btnTop,
					75, 24,
					hWnd, (HMENU)IDCANCEL, gg_hInstance, NULL);
	SetWindowFontU(btnCancel, gg_hFont, true);
	
	// "Apply" button.
	HWND btnApply = pCreateWindowU(WC_BUTTON, "&Apply",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					GG_WINDOW_WIDTH-(8+75), btnTop,
					75, 24,
					hWnd, (HMENU)IDAPPLY, gg_hInstance, NULL);
	SetWindowFontU(btnApply, gg_hFont, true);
	
	// Initialize the Game Genie code ListView.
	gg_window_init();
	
	// Set focus to the "Code" textbox.
	SetFocus(txtCode);
}


/**
 * gg_window_create_lstCodes(): Create the treeview for the Game Genie codes.
 * @param container Container for the ListView.
 */
static void gg_window_create_lstCodes(HWND container)
{
	// ListView containing the Game Genie codes.
	lstCodes = pCreateWindowExU(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS,
					8, 8+16+68+8+24+24,
					GG_WINDOW_WIDTH-(8+8), 128,
					container, NULL, gg_hInstance, NULL);
	SetWindowFontU(lstCodes, gg_hFont, true);
	ListView_SetExtendedListViewStyleU(lstCodes, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	
	// Create the ListView columns.
	LV_COLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	// Code (Hex)
	lvCol.pszText = "Code (Hex)";
	lvCol.cx = 144;
	pListView_InsertColumnU(lstCodes, 0, &lvCol);
	
	// Code (GG)
	lvCol.pszText = "Code (GG)";
	lvCol.cx = 104;
	pListView_InsertColumnU(lstCodes, 1, &lvCol);
	
	// CPU
	lvCol.pszText = "CPU";
	lvCol.cx = 48;
	pListView_InsertColumnU(lstCodes, 2, &lvCol);
	
	// Name
	lvCol.pszText = "Name";
	lvCol.cx = 192;
	pListView_InsertColumnU(lstCodes, 3, &lvCol);
}


/**
 * gg_window_close(): Close the Game Genie window.
 */
void gg_window_close(void)
{
	if (!gg_window)
		return;
	
	// Unregister the window from MDP Host Services.
	gg_host_srv->window_unregister(&mdp, gg_window);
	
	// Delete any gg_code pointers that may be stored in the ListView.
	const int lvItems = ListView_GetItemCountU(lstCodes);
	
	LVITEM lvItem;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	
	for (int i = lvItems - 1; i >= 0; i--)
	{
		lvItem.iItem = i;
		if (pListView_GetItemU(lstCodes, &lvItem))
		{
			// Item retrieved. Delete the associated gg_code_t*.
			free((gg_code_t*)(lvItem.lParam));
		}
	}
	
	// Destroy the window.
	HWND tmp = gg_window;
	gg_window = NULL;
	DestroyWindow(tmp);
	
	// Delete the fonts.
	DeleteFont(gg_hFont);
	DeleteFont(gg_hFont_title);
	gg_hFont = NULL;
	gg_hFont_title = NULL;
	
	// Shut down the Win32 Unicode Translation Layer.
	w32u_end();
}


/**
 * gg_window_init(): Initialize the Game Genie code ListView.
 */
static void gg_window_init(void)
{
	// Add all loaded codes to the ListView.
	for (list<gg_code_t>::iterator iter = gg_code_list.begin();
	     iter != gg_code_list.end(); iter++)
	{
		gg_window_add_code(&(*iter), NULL);
	}
}


/**
 * gg_window_save(): Save the Game Genie codes.
 */
static void gg_window_save(void)
{
	// Unapply ROM codes.
	gg_engine_unapply_rom_codes();
	
	// Clear the code list.
	gg_code_list.clear();
	
	gg_code_t gg_code;
	
	const int lvItems = ListView_GetItemCountU(lstCodes);
	
	LVITEM lvItem;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_STATE | LVIF_PARAM;
	
	for (int i = 0; i < lvItems; i++)
	{
		lvItem.iItem = i;
		if (pListView_GetItemU(lstCodes, &lvItem))
		{
			// Item retrieved.
			
			// Copy the code.
			memcpy(&gg_code, (gg_code_t*)(lvItem.lParam), sizeof(gg_code));
			
			// Copy the "enabled" value.
			gg_code.enabled = (ListView_GetCheckStateU(lstCodes, i) ? 1 : 0);
			
			// Add the code to the list of codes.
			gg_code_list.push_back(gg_code);
		}
	}
	
	// Apply ROM codes.
	gg_engine_apply_rom_codes();
}


/**
 * gg_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK gg_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			gg_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					gg_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					gg_window_save();
					break;
				case IDC_BTNADDCODE:
					gg_window_add_code_from_textboxes();
					break;
				case IDC_BTNDELETE:
					gg_window_callback_delete();
					break;
				case IDC_BTNDEACTIVATEALL:
					gg_window_callback_deactivate_all();
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_DESTROY:
			gg_window_close();
			break;
	}
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
}


/**
 * gg_window_wndproc_textbox(): Window procedure for textboxes. (Subclassed)
 * @param hWnd
 * @param message
 * @param wParam
 * @param lParam
 */
static LRESULT CALLBACK gg_window_wndproc_textbox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Game Genie textbox subclassing
	// This is only used to capture the Enter keypress.
	
	// Note: WM_GETDLGCODE may show up instead of WM_KEYDOWN,
	// since this window handles dialog messages.
	if (wParam == VK_RETURN)
	{
		if (message == WM_GETDLGCODE)
		{
			// Dialog code. Request all keys.
			return DLGC_WANTALLKEYS;
		}
		else if (message == WM_KEYDOWN)
		{
			// Enter is pressed. Add the code.
			gg_window_add_code_from_textboxes();
			return true;
		}
	}
	
	// Not Enter. Run the regular procedure.
	if (hWnd == txtCode)
		return CallWindowProc(txtCode_old_wndproc, hWnd, message, wParam, lParam);
	else if (hWnd == txtName)
		return CallWindowProc(txtName_old_wndproc, hWnd, message, wParam, lParam);
	
	// Unknown control.
	return false;
}


/**
 * gg_window_add_code_from_textboxes(): Add a code to the treeview from the textboxes.
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code_from_textboxes(void)
{
	if (Edit_GetTextLengthU(txtCode) == 0)
		return 1;
	
	// Decode the code.
	gg_code_t gg_code;
	gg_code.name[0] = 0x00;
	gg_code.enabled = 0;
	
	char code_txt[32];
	Edit_GetTextU(txtCode, code_txt, sizeof(code_txt));
	GG_CODE_ERR gcp_rval = gg_code_parse(code_txt, &gg_code, CPU_M68K);
	
	if (gcp_rval != GGCE_OK)
	{
		// Error parsing the code.
		char err_msg_full[1024];
		const char *err_msg;
		
		switch (gcp_rval)
		{
			case GGCE_UNRECOGNIZED:
				err_msg = "The code could not be parsed correctly.";
				break;
			case GGCE_ADDRESS_RANGE:
				// TODO: Show range depending on the selected CPU.
				err_msg = "The address for this code is out of range for the specified CPU.\n"
					  "Valid range for MC68000 CPUs: 0x000000 - 0xFFFFFF";
				break;
			case GGCE_ADDRESS_ALIGNMENT:
				// TODO: Show range and alignment info based on CPU and datasize.
				err_msg = "The address is not aligned properly for the specified data.\n"
					  "For MC68000, 16-bit data must be stored at even addresses.";
				break;
			case GGCE_DATA_TOO_LARGE:
				err_msg = "The data value is too large. Usually, this means that you\n"
					  "entered too many characters.";
				break;
			default:
				// Other error.
				err_msg = "Unknown error code.";
				break;
		}
		
		szprintf(err_msg_full, sizeof(err_msg_full),
				"The specified code, \"%s\", could not be added due to an error:\n\n%s",
				code_txt, err_msg);
		
		// Show an error message.
		pMessageBoxU(gg_window, err_msg_full, "Game Genie: Code Error", MB_ICONSTOP);
		
		// Set focus to the "Code" textbox.
		SetFocus(txtCode);
		return gcp_rval;
	}
	
	// Get the name entry.
	char s_name[128];
	Edit_GetTextU(txtName, s_name, sizeof(s_name));
	
	// Add the code.
	int ggw_ac_rval = gg_window_add_code(&gg_code, s_name);
	if (ggw_ac_rval == 0)
	{
		// Code added successfully.
		// Clear the textboxes and set focus to the "Code" textbox.
		Edit_SetTextU(txtCode, NULL);
		Edit_SetTextU(txtName, NULL);
		SetFocus(txtCode);
	}
	
	return ggw_ac_rval;
}


/**
 * gg_window_add_code(): Add a code to the ListView.
 * @param gg_code Pointer to gg_code_t containing the code to add.
 * @param name Name of the code. (If NULL, the name in gg_code is used.)
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code(const gg_code_t *gg_code, const char *name)
{
	if (!gg_code)
		return 1;
	
	// Create the hex version of the code.
	char s_code_hex[32];
	if (gg_code_format_hex(gg_code, s_code_hex, sizeof(s_code_hex)))
	{
		// TODO: Show an error message.
		return 1;
	}
	
	// CPU string.
	static const char *const s_cpu_list[8] =
	{
		NULL, TEXT("M68K"), TEXT("S68K"), TEXT("Z80"),
		TEXT("MSH2"), TEXT("SSH2"), NULL, NULL
	};
	
	const char *const s_cpu = s_cpu_list[(int)(gg_code->cpu) & 0x07];
	if (!s_cpu)
		return 1;
	
	// Determine what should be used for the Game Genie code.
	const char *s_code_gg;
	if (!gg_code->game_genie[0])
	{
		// The code can't be converted to Game Genie.
		s_code_gg = "N/A";
	}
	else
	{
		// The code can be converted to Game Genie.
		s_code_gg = gg_code->game_genie;
	}
	
	// Store the gg_code_t in the list store.
	gg_code_t *lst_code = (gg_code_t*)(malloc(sizeof(gg_code_t)));
	memcpy(lst_code, gg_code, sizeof(gg_code_t));
	
	// If no name is given, use the name in the gg_code.
	if (!name)
		name = &gg_code->name[0];
	else
	{
		// Copy the specified name into lst_code.
		memcpy(lst_code->name, name, sizeof(lst_code->name));
		lst_code->name[sizeof(lst_code->name)-1] = 0x00;
	}
	
	// Code is decoded. Add it to the ListView.
	LVITEM lviCode;
	memset(&lviCode, 0x00, sizeof(lviCode));
	lviCode.mask = LVIF_TEXT | LVIF_PARAM;
	lviCode.cchTextMax = 256;
	lviCode.iItem = ListView_GetItemCountU(lstCodes);
	lviCode.lParam = (LPARAM)lst_code;
	
	// First column: Code (Hex)
	lviCode.iSubItem = 0;
	lviCode.pszText = s_code_hex;
	pListView_InsertItemU(lstCodes, &lviCode);
	
	// lParam doesn't need to be set for the subitems.
	lviCode.mask = LVIF_TEXT;
	lviCode.lParam = NULL;
	
	// Second column: Code (GG)
	lviCode.iSubItem = 1;
	lviCode.pszText = const_cast<LPTSTR>(s_code_gg);
	pListView_SetItemU(lstCodes, &lviCode);
	
	// Third column: CPU
	lviCode.iSubItem = 2;
	lviCode.pszText = const_cast<LPTSTR>(s_cpu);
	pListView_SetItemU(lstCodes, &lviCode);
	
	// Fourth column: Name
	lviCode.iSubItem = 3;
	lviCode.pszText = const_cast<LPTSTR>(name);
	pListView_SetItemU(lstCodes, &lviCode);
	
	// Set the "Enabled" state.
	ListView_SetCheckStateU(lstCodes, lviCode.iItem, gg_code->enabled);
	
	// Code added successfully.
	return 0;
}


/**
 * gg_window_callback_delete(): Delete the selected code(s).
 */
static void gg_window_callback_delete(void)
{
	// Delete all selected codes.
	const int lvItems = ListView_GetItemCountU(lstCodes);
	
	LVITEM lvItem;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_STATE | LVIF_PARAM;
	lvItem.stateMask = LVIS_SELECTED;
	
	for (int i = lvItems - 1; i >= 0; i--)
	{
		lvItem.iItem = i;
		if (pListView_GetItemU(lstCodes, &lvItem))
		{
			// Item retrieved. Check if it should be deleted.
			if (lvItem.state & LVIS_SELECTED)
			{
				// Selected item. Delete it.
				
				// lvItem.lParam contains the gg_code_t* pointer.
				free((gg_code_t*)(lvItem.lParam));
				
				// Delete the item from the ListView.
				ListView_DeleteItemU(lstCodes, i);
			}
		}
	}
}


/**
 * gg_window_callback_deactivate_all(): Deactivate all codes.
 */
static void gg_window_callback_deactivate_all(void)
{
	const int lvItems = ListView_GetItemCountU(lstCodes);
	
	for (int i = 0; i < lvItems; i++)
	{
		ListView_SetCheckStateU(lstCodes, lvItems, false);
	}
}
