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
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>

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

// For whatever reason, Wine's headers don't include the ListView_(Set|Get)CheckState macros.
#ifndef ListView_SetCheckState
#define ListView_SetCheckState(hwnd,iIndex,bCheck) \
	ListView_SetItemState(hwnd,iIndex,INDEXTOSTATEIMAGEMASK((bCheck)+1),LVIS_STATEIMAGEMASK)
#endif
#ifndef ListView_GetCheckState
#define ListView_GetCheckState(hwnd,iIndex) \
	((((UINT)(ListView_GetItemState(hwnd,iIndex,LVIS_STATEIMAGEMASK)))>>12)-1)
#endif

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
static int	gg_window_add_code(const gg_code_t *gg_code, const char* name);


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
	
	gg_window_child_windows_created = false;
	
	// If no HINSTANCE was specified, use the main executable's HINSTANCE.
	if (!gg_hInstance)
		gg_hInstance = GetModuleHandle(NULL);
	
	// Create the window class.
	if (gg_window_wndclass.lpfnWndProc != gg_window_wndproc)
	{
		gg_window_wndclass.style = 0;
		gg_window_wndclass.lpfnWndProc = gg_window_wndproc;
		gg_window_wndclass.cbClsExtra = 0;
		gg_window_wndclass.cbWndExtra = 0;
		gg_window_wndclass.hInstance = gg_hInstance;
		gg_window_wndclass.hIcon = LoadIcon(gg_hInstance, MAKEINTRESOURCE(IDI_GAME_GENIE));
		gg_window_wndclass.hCursor = NULL;
		gg_window_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		gg_window_wndclass.lpszMenuName = NULL;
		gg_window_wndclass.lpszClassName = TEXT("gg_window_wndclass");
		
		RegisterClass(&gg_window_wndclass);
	}
	
	// Create the fonts.
	gg_hFont = gsft_win32_gdi_get_message_font();
	gg_hFont_title = gsft_win32_gdi_get_title_font();
	
	// Create the window.
	gg_window = CreateWindow(TEXT("gg_window_wndclass"), TEXT("Game Genie"),
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
	static const TCHAR* const strInfoTitle = TEXT("Information about Game Genie / Patch codes");
	static const TCHAR* const strInfo = TEXT(
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA:DDDD (address:data)");
	
	// Header label.
	HWND lblInfoTitle = CreateWindow(WC_STATIC, strInfoTitle,
					 WS_CHILD | WS_VISIBLE | SS_LEFT,
					 8, 8,
					 GG_WINDOW_WIDTH-16, 16,
					 hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblInfoTitle, gg_hFont_title, true);
	
	// Description label.
	HWND lblInfo = CreateWindow(WC_STATIC, strInfo,
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8+16,
				    GG_WINDOW_WIDTH-16, 68,
				    hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblInfo, gg_hFont, true);
	
	// "Code" label.
	HWND lblCode = CreateWindow(WC_STATIC, TEXT("Code"),
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8+16+68+8, 32, 16,
				    hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblCode, gg_hFont, true);
	
	// "Code" textbox.
	txtCode = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				 8+32+8, 8+16+68+8,
				 GG_WINDOW_WIDTH-(8+32+8+64+8+8+16), 20,
				 hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(txtCode, gg_hFont, true);
	SendMessage(txtCode, EM_LIMITTEXT, 17, 0);
	txtCode_old_wndproc = (WNDPROC)SetWindowLongPtr(
			       txtCode, GWL_WNDPROC,
			       (LONG_PTR)gg_window_wndproc_textbox);
	
	// "Name" label.
	HWND lblName = CreateWindow(WC_STATIC, TEXT("Name"),
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8+16+68+8+24, 32, 16,
				    hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblName, gg_hFont, true);
	
	// "Name" textbox.
	txtName = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				 8+32+8, 8+16+68+8+24,
				 GG_WINDOW_WIDTH-(8+32+8+64+8+8+16), 20,
				 hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(txtName, gg_hFont, true);
	txtName_old_wndproc = (WNDPROC)SetWindowLongPtr(
			       txtName, GWL_WNDPROC,
			       (LONG_PTR)gg_window_wndproc_textbox);
	
	// "Add Code" button.
	HWND btnAddCode = CreateWindow(WC_BUTTON, TEXT("Add C&ode"),
				       WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				       GG_WINDOW_WIDTH-(64+8+16), 8+16+68+8,
				       63+16, 20,
				       hWnd, (HMENU)IDC_BTNADDCODE, gg_hInstance, NULL);
	SetWindowFont(btnAddCode, gg_hFont, true);
	
	// Create the ListView for the Game Genie codes.
	gg_window_create_lstCodes(hWnd);
	
	// Create the dialog buttons.
	static const int btnTop = GG_WINDOW_HEIGHT-24-8;
	
	// "Deactivate All" button.
	HWND btnDeactivateAll = CreateWindow(WC_BUTTON, TEXT("Deac&tivate All"),
						WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						8, btnTop,
						91, 24,
						hWnd, (HMENU)IDC_BTNDEACTIVATEALL, gg_hInstance, NULL);
	SetWindowFont(btnDeactivateAll, gg_hFont, true);
	
	// "Delete" button.
	HWND btnDelete = CreateWindow(WC_BUTTON, TEXT("&Delete"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					8+91+8, btnTop,
					75, 24,
					hWnd, (HMENU)IDC_BTNDELETE, gg_hInstance, NULL);
	SetWindowFont(btnDelete, gg_hFont, true);
	
	// "OK" button.
	HWND btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					GG_WINDOW_WIDTH-(8+75+8+75+8+75), btnTop,
					75, 24,
					hWnd, (HMENU)IDOK, gg_hInstance, NULL);
	SetWindowFont(btnOK, gg_hFont, true);
	
	// "Cancel" button.
	HWND btnCancel = CreateWindow(WC_BUTTON, TEXT("&Cancel"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					GG_WINDOW_WIDTH-(8+75+8+75), btnTop,
					75, 24,
					hWnd, (HMENU)IDCANCEL, gg_hInstance, NULL);
	SetWindowFont(btnCancel, gg_hFont, true);
	
	// "Apply" button.
	HWND btnApply = CreateWindow(WC_BUTTON, TEXT("&Apply"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					GG_WINDOW_WIDTH-(8+75), btnTop,
					75, 24,
					hWnd, (HMENU)IDAPPLY, gg_hInstance, NULL);
	SetWindowFont(btnApply, gg_hFont, true);
	
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
	lstCodes = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS,
				  8, 8+16+68+8+24+24,
				  GG_WINDOW_WIDTH-(8+8), 128,
				  container, NULL, gg_hInstance, NULL);
	SetWindowFont(lstCodes, gg_hFont, true);
	ListView_SetExtendedListViewStyle(lstCodes, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	
	// Create the ListView columns.
	LV_COLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	// Code (Hex)
	lvCol.pszText = TEXT("Code (Hex)");
	lvCol.cx = 144;
	ListView_InsertColumn(lstCodes, 0, &lvCol);
	
	// Code (GG)
	lvCol.pszText = TEXT("Code (GG)");
	lvCol.cx = 104;
	ListView_InsertColumn(lstCodes, 1, &lvCol);
	
	// CPU
	lvCol.pszText = TEXT("CPU");
	lvCol.cx = 48;
	ListView_InsertColumn(lstCodes, 2, &lvCol);
	
	// Name
	lvCol.pszText = TEXT("Name");
	lvCol.cx = 192;
	ListView_InsertColumn(lstCodes, 3, &lvCol);
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
	const int lvItems = ListView_GetItemCount(lstCodes);
	
	LVITEM lvItem;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	
	for (int i = lvItems - 1; i >= 0; i--)
	{
		lvItem.iItem = i;
		if (ListView_GetItem(lstCodes, &lvItem))
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
	
	const int lvItems = ListView_GetItemCount(lstCodes);
	
	LVITEM lvItem;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_STATE | LVIF_PARAM;
	
	for (int i = 0; i < lvItems; i++)
	{
		lvItem.iItem = i;
		if (ListView_GetItem(lstCodes, &lvItem))
		{
			// Item retrieved.
			
			// Copy the code.
			memcpy(&gg_code, (gg_code_t*)(lvItem.lParam), sizeof(gg_code));
			
			// Copy the "enabled" value.
			gg_code.enabled = (ListView_GetCheckState(lstCodes, i) ? 1 : 0);
			
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
	
	return DefWindowProc(hWnd, message, wParam, lParam);
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
	if (Edit_GetTextLength(txtCode) == 0)
		return 1;
	
	// Decode the code.
	gg_code_t gg_code;
	gg_code.name[0] = 0x00;
	gg_code.enabled = 0;
	
	TCHAR code_txt[32];
	Edit_GetText(txtCode, code_txt, (sizeof(code_txt)/sizeof(TCHAR)));
	GG_CODE_ERR gcp_rval = gg_code_parse(code_txt, &gg_code, CPU_M68K);
	
	if (gcp_rval != GGCE_OK)
	{
		// Error parsing the code.
		TCHAR err_msg_full[1024];
		LPCTSTR err_msg;
		
		switch (gcp_rval)
		{
			case GGCE_UNRECOGNIZED:
				err_msg = TEXT("The code could not be parsed correctly.");
				break;
			case GGCE_ADDRESS_RANGE:
				// TODO: Show range depending on the selected CPU.
				err_msg = TEXT("The address for this code is out of range for the specified CPU.\n"
					       "Valid range for MC68000 CPUs: 0x000000 - 0xFFFFFF");
				break;
			case GGCE_ADDRESS_ALIGNMENT:
				// TODO: Show range and alignment info based on CPU and datasize.
				err_msg = TEXT("The address is not aligned properly for the specified data.\n"
					       "For MC68000, 16-bit data must be stored at even addresses.");
				break;
			case GGCE_DATA_TOO_LARGE:
				err_msg = TEXT("The data value is too large. Usually, this means that you\n"
					       "entered too many characters.");
				break;
			default:
				// Other error.
				err_msg = TEXT("Unknown error code.");
				break;
		}
		
		_sntprintf(err_msg_full, (sizeof(err_msg_full)/sizeof(TCHAR)),
			   TEXT("The specified code, \"%s\", could not be added due to an error:\n\n%s"),
			   code_txt, err_msg);
		err_msg_full[(sizeof(err_msg_full)/sizeof(TCHAR))-1] = 0x00;
		
		// Show an error message.
		MessageBox(gg_window, err_msg_full, TEXT("Game Genie: Code Error"), MB_ICONSTOP);
		
		// Set focus to the "Code" textbox.
		SetFocus(txtCode);
		
		return gcp_rval;
	}
	
	// Get the name entry.
	TCHAR s_name[128];
	Edit_GetText(txtName, s_name, (sizeof(s_name)/sizeof(TCHAR)));
	
	// Add the code.
	int ggw_ac_rval = gg_window_add_code(&gg_code, s_name);
	if (ggw_ac_rval == 0)
	{
		// Code added successfully.
		// Clear the textboxes and set focus to the "Code" textbox.
		Edit_SetText(txtCode, NULL);
		Edit_SetText(txtName, NULL);
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
static int gg_window_add_code(const gg_code_t *gg_code, const char* name)
{
	if (!gg_code)
		return 1;
	
	// Create the hex version of the code.
	TCHAR s_code_hex[32];
	if (gg_code_format_hex(gg_code, s_code_hex, sizeof(s_code_hex)))
	{
		// TODO: Show an error message.
		return 1;
	}
	
	// CPU string.
	static LPCTSTR const s_cpu_list[8] = {NULL, TEXT("M68K"), TEXT("S68K"), TEXT("Z80"), TEXT("MSH2"), TEXT("SSH2"), NULL, NULL};
	const LPCTSTR s_cpu = s_cpu_list[(int)(gg_code->cpu) & 0x07];
	if (!s_cpu)
		return 1;
	
	// Determine what should be used for the Game Genie code.
	LPCTSTR s_code_gg;
	if (!gg_code->game_genie[0])
	{
		// The code can't be converted to Game Genie.
		s_code_gg = TEXT("N/A");
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
	lviCode.iItem = ListView_GetItemCount(lstCodes);
	lviCode.lParam = (LPARAM)lst_code;
	
	// First column: Code (Hex)
	lviCode.iSubItem = 0;
	lviCode.pszText = s_code_hex;
	ListView_InsertItem(lstCodes, &lviCode);
	
	// lParam doesn't need to be set for the subitems.
	lviCode.mask = LVIF_TEXT;
	lviCode.lParam = NULL;
	
	// Second column: Code (GG)
	lviCode.iSubItem = 1;
	lviCode.pszText = const_cast<LPTSTR>(s_code_gg);
	ListView_SetItem(lstCodes, &lviCode);
	
	// Third column: CPU
	lviCode.iSubItem = 2;
	lviCode.pszText = const_cast<LPTSTR>(s_cpu);
	ListView_SetItem(lstCodes, &lviCode);
	
	// Fourth column: Name
	lviCode.iSubItem = 3;
	lviCode.pszText = const_cast<LPTSTR>(name);
	ListView_SetItem(lstCodes, &lviCode);
	
	// Set the "Enabled" state.
	ListView_SetCheckState(lstCodes, lviCode.iItem, gg_code->enabled);
	
	// Code added successfully.
	return 0;
}


/**
 * gg_window_callback_delete(): Delete the selected code(s).
 */
static void gg_window_callback_delete(void)
{
	// Delete all selected codes.
	const int lvItems = ListView_GetItemCount(lstCodes);
	
	LVITEM lvItem;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_STATE | LVIF_PARAM;
	lvItem.stateMask = LVIS_SELECTED;
	
	for (int i = lvItems - 1; i >= 0; i--)
	{
		lvItem.iItem = i;
		if (ListView_GetItem(lstCodes, &lvItem))
		{
			// Item retrieved. Check if it should be deleted.
			if (lvItem.state & LVIS_SELECTED)
			{
				// Selected item. Delete it.
				
				// lvItem.lParam contains the gg_code_t* pointer.
				free((gg_code_t*)(lvItem.lParam));
				
				// Delete the item from the ListView.
				ListView_DeleteItem(lstCodes, i);
			}
		}
	}
}


/**
 * gg_window_callback_deactivate_all(): Deactivate all codes.
 */
static void gg_window_callback_deactivate_all(void)
{
	const int lvItems = ListView_GetItemCount(lstCodes);
	
	for (int i = 0; i < lvItems; i++)
	{
		ListView_SetCheckState(lstCodes, lvItems, false);
	}
}
