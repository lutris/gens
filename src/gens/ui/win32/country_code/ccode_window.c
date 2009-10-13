/***************************************************************************
 * Gens: (Win32) Country Code Window.                                      *
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

#include "ccode_window.h"
#include "gens/gens_window.h"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include "ui/win32/fonts.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"

// Window.
HWND ccode_window = NULL;
static BOOL	ccode_window_child_windows_created = FALSE;

// Window class.
static WNDCLASS	ccode_wndclass;

// Window size.
#define CCODE_WINDOW_WIDTH  256
#define CCODE_WINDOW_DEF_HEIGHT 144
static int ccode_window_height;

#define CCODE_WINDOW_BTN_SIZE      24
#define CCODE_WINDOW_BTN_ICON_SIZE 16

#define CCODE_WINDOW_FRACOUNTRY_WIDTH  (CCODE_WINDOW_WIDTH-16)
#define CCODE_WINDOW_FRACOUNTRY_HEIGHT (CCODE_WINDOW_DEF_HEIGHT-16-24-8)

#define CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH  (CCODE_WINDOW_FRACOUNTRY_WIDTH-8-16-CCODE_WINDOW_BTN_SIZE)
#define CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT (CCODE_WINDOW_DEF_HEIGHT-16-24-8-16-8)

// Window procedure.
static LRESULT CALLBACK ccode_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	lstCountryCodes;
static HWND	btnOK, btnCancel, btnApply;

// Widget creation functions.
static void	ccode_window_create_child_windows(HWND hWnd);
static void	ccode_window_create_lstCountryCodes(HWND container);
static void	ccode_window_create_up_down_buttons(HWND container);

// Configuration load/save functions.
static void	ccode_window_init(void);
static void	ccode_window_save(void);

// Callbacks.
static void	ccode_window_callback_btnUp_clicked(void);
static void	ccode_window_callback_btnDown_clicked(void);

/** BEGIN: Common Controls v6.0 **/

#include "ui/win32/cc6.h"

// Image lists.
static HIMAGELIST	imglArrowUp = NULL;
static HIMAGELIST	imglArrowDown = NULL;

static CC6_STATUS_T	ccode_window_cc6 = CC6_STATUS_UNKNOWN;

/** END: Common Controls v6.0 **/


/**
 * ccode_window_show(): Show the Country Code window.
 */
void ccode_window_show(void)
{
	if (ccode_window)
	{
		// Country Code window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(ccode_window, SW_SHOW);
		return;
	}
	
	ccode_window_child_windows_created = FALSE;
	
	if (ccode_wndclass.lpfnWndProc != ccode_window_wndproc)
	{
		// Create the window class.
		ccode_wndclass.style = 0;
		ccode_wndclass.lpfnWndProc = ccode_window_wndproc;
		ccode_wndclass.cbClsExtra = 0;
		ccode_wndclass.cbWndExtra = 0;
		ccode_wndclass.hInstance = ghInstance;
		ccode_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		ccode_wndclass.hCursor = NULL;
		ccode_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		ccode_wndclass.lpszMenuName = NULL;
		ccode_wndclass.lpszClassName = TEXT("ccode_window");
		
		RegisterClass(&ccode_wndclass);
	}
	
	// Create the window.
	ccode_window = CreateWindow(TEXT("ccode_window"), TEXT("Country Code Order"),
				    WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    CCODE_WINDOW_WIDTH, CCODE_WINDOW_DEF_HEIGHT,
				    gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	// NOTE: This is done in Country_Code_Window_CreateChildWindows to compensate for listbox variations.
	//Win32_setActualWindowSize(ccode_window, CCODE_WINDOW_WIDTH, CCODE_WINDOW_DEF_HEIGHT);
	
	// Center the window on the parent window.
	// NOTE: This is done in Country_Code_Window_CreateChildWindows to compensate for listbox variations.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	//Win32_centerOnGensWindow(ccode_window);
	
	UpdateWindow(ccode_window);
	ShowWindow(ccode_window, SW_SHOW);
}


/**
 * ccode_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void ccode_window_create_child_windows(HWND hWnd)
{
	// Add a frame for country code selection.
	HWND fraCountry = CreateWindow(WC_BUTTON, TEXT("Country Code Order"),
				       WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				       8, 8,
				       CCODE_WINDOW_FRACOUNTRY_WIDTH,
				       CCODE_WINDOW_FRACOUNTRY_HEIGHT,
				       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(fraCountry, fntMain, TRUE);
	
	// Create the Country Code treeview.
	ccode_window_create_lstCountryCodes(hWnd);
	
	// Adjust the window height based on the listbox's actual height.
	gsft_win32_set_actual_window_size(hWnd, CCODE_WINDOW_WIDTH, ccode_window_height);
	gsft_win32_center_on_window(hWnd, gens_window);
	
	// Adjust the frame's height.
	SetWindowPos(fraCountry, 0, 0, 0,
		     CCODE_WINDOW_FRACOUNTRY_WIDTH,
		     CCODE_WINDOW_FRACOUNTRY_HEIGHT + (ccode_window_height - CCODE_WINDOW_DEF_HEIGHT),
		     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	
	// Create the Up/Down buttons.
	ccode_window_create_up_down_buttons(hWnd);
	
	// Create the dialog buttons.
	
	// TODO: Center the buttons, or right-align them?
	// They look better center-aligned in this window...
	static const int btnLeft = (CCODE_WINDOW_WIDTH-75-8-75-8-75)/2;
	
	// OK button.
	btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					btnLeft, ccode_window_height-8-24,
					75, 23,
					hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Cancel button.
	btnCancel = CreateWindow(WC_BUTTON, TEXT("&Cancel"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft+8+75, ccode_window_height-8-24,
					75, 23,
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Apply button.
	btnApply = CreateWindow(WC_BUTTON, TEXT("&Apply"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft+8+75+8+75, ccode_window_height-8-24,
					75, 23,
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, FALSE);
	
	// Initialize the internal data variables.
	ccode_window_init();
	
	// Set focus to the listbox.
	SetFocus(lstCountryCodes);
}


/**
 * ccode_window_create_lstCountryCodes(): Create the Country Codes listbox.
 * @param container Container for the listbox.
 */
static void ccode_window_create_lstCountryCodes(HWND container)
{
	// Create the listbox.
	lstCountryCodes = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTBOX, NULL,
					 WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					 8+8, 8+16,
					 CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH,
					 CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT,
					 container, NULL, ghInstance, NULL);
	SetWindowFont(lstCountryCodes, fntMain, TRUE);
	
	// Check what the listbox's actual height is.
	RECT r;
	GetWindowRect(lstCountryCodes, &r);
	
	// Determine the window height based on the listbox's adjusted size.
	ccode_window_height = CCODE_WINDOW_DEF_HEIGHT + ((r.bottom - r.top) - CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT);
}


/**
 * ccode_window_create_up_down_buttons(): Create the Up/Down buttons.
 * @param container Container for the buttons.
 */
static void ccode_window_create_up_down_buttons(HWND container)
{
	// Create the Up/Down buttons.
	// TODO: BS_ICON apparently doesn't work on NT4 and earlier.
	// See http://support.microsoft.com/kb/142226
	
	// Load the icons.
	HICON icoUp   = (HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_ARROW_UP),
					 IMAGE_ICON,
					 CCODE_WINDOW_BTN_ICON_SIZE,
					 CCODE_WINDOW_BTN_ICON_SIZE, LR_SHARED);
	HICON icoDown = (HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_ARROW_DOWN),
					 IMAGE_ICON,
					 CCODE_WINDOW_BTN_ICON_SIZE,
					 CCODE_WINDOW_BTN_ICON_SIZE, LR_SHARED);
	
	// Create the buttons.
	
	// "Up" button.
	HWND btnUp = CreateWindow(WC_BUTTON, NULL,
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON,
				  8+8+CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH+8,
				  8+16,
				  CCODE_WINDOW_BTN_SIZE, CCODE_WINDOW_BTN_SIZE,
				  container, (HMENU)IDC_COUNTRY_CODE_UP, ghInstance, NULL);
	SetWindowFont(btnUp, fntMain, TRUE);
	
	// "Down" button.
	HWND btnDown = CreateWindow(WC_BUTTON, NULL,
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_ICON,
				    8+8+CCODE_WINDOW_LSTCOUNTRYCODES_WIDTH+8,
				    8+16+CCODE_WINDOW_LSTCOUNTRYCODES_HEIGHT+(ccode_window_height-CCODE_WINDOW_DEF_HEIGHT)-CCODE_WINDOW_BTN_SIZE,
				    CCODE_WINDOW_BTN_SIZE, CCODE_WINDOW_BTN_SIZE,
				    container, (HMENU)IDC_COUNTRY_CODE_DOWN, ghInstance, NULL);
	SetWindowFont(btnDown, fntMain, TRUE);
	
	// Check the Common Controls v6.0 status.
	if (ccode_window_cc6 == CC6_STATUS_UNKNOWN)
		ccode_window_cc6 = cc6_check();
	
	// Set the button icons.
	if (ccode_window_cc6 == CC6_STATUS_V6)
	{
		// Common Controls v6.0. Use BUTTON_IMAGELIST and Button_SetImageList().
		// This ensures that visual styles are applied correctly.
		
		// "Up" button image list.
		imglArrowUp = ImageList_Create(CCODE_WINDOW_BTN_ICON_SIZE,
					       CCODE_WINDOW_BTN_ICON_SIZE,
					       ILC_MASK | ILC_COLOR4, 1, 1);
		ImageList_SetBkColor(imglArrowUp, CLR_NONE);
		ImageList_AddIcon(imglArrowUp, icoUp);
		
		// "Down" button image list.
		imglArrowDown = ImageList_Create(CCODE_WINDOW_BTN_ICON_SIZE,
						 CCODE_WINDOW_BTN_ICON_SIZE,
						 ILC_MASK | ILC_COLOR4, 1, 1);
		ImageList_SetBkColor(imglArrowDown, CLR_NONE);
		ImageList_AddIcon(imglArrowDown, icoDown);
		
		// BUTTON_IMAGELIST struct.
		BUTTON_IMAGELIST bimglBtn;
		bimglBtn.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;
		bimglBtn.margin.top = 0;
		bimglBtn.margin.bottom = 0;
		bimglBtn.margin.left = 1;
		bimglBtn.margin.right = 0;
		
		// "Up" button.
		bimglBtn.himl = imglArrowUp;
		Button_SetImageList(btnUp, &bimglBtn);
		
		// "Down" button.
		bimglBtn.himl = imglArrowDown;
		Button_SetImageList(btnDown, &bimglBtn);
	}
	else
	{
		// Older version of Common Controls. Use BM_SETIMAGE.
		SetWindowText(btnUp, TEXT("Up"));
		SetWindowText(btnDown, TEXT("Down"));
		SendMessage(btnUp, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icoUp);
		SendMessage(btnDown, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icoDown);
	}
}


/**
 * ccode_window_close(): Close the About window.
 */
void ccode_window_close(void)
{
	if (!ccode_window)
		return;
	
	// Destroy the window.
	DestroyWindow(ccode_window);
	ccode_window = NULL;
	
	// Destroy the image lists.
	if (imglArrowUp)
	{
		ImageList_Destroy(imglArrowUp);
		imglArrowUp = NULL;
	}
	if (imglArrowDown)
	{
		ImageList_Destroy(imglArrowDown);
		imglArrowDown = NULL;
	}
}


/**
 * ccode_window_init(): Initialize the file text boxes.
 */
static void ccode_window_init(void)
{
	// Set up the country order listbox.
	// Elements in Country_Order[3] can have one of three values:
	// - 0 [USA]
	// - 1 [JAP]
	// - 2 [EUR]
	
	// Make sure the country code order is valid.
	Check_Country_Order();
	
	// Clear the listbox.
	ListBox_ResetContent(lstCountryCodes);
	
	// Country codes.
	// TODO: Move this to a common file.
	static const TCHAR* const ccodes[3] = {TEXT("USA"), TEXT("Japan"), TEXT("Europe")};
	
	// Add the country codes to the listbox in the appropriate order.
	int i;
	for (i = 0; i < 3; i++)
	{
		ListBox_InsertString(lstCountryCodes, i, ccodes[Country_Order[i]]);
		ListBox_SetItemData(lstCountryCodes, i, Country_Order[i]);
	}
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, FALSE);
}


/**
 * ccode_window_save(): Save the Country Code order.
 */
static void ccode_window_save(void)
{
	// Save settings.
	int i;
	for (i = 0; i < 3; i++)
	{
		Country_Order[i] = ListBox_GetItemData(lstCountryCodes, i);
	}
	
	// Validate the country code order.
	Check_Country_Order();
	
	// Disable the "Apply" button.
	Button_Enable(btnApply, FALSE);
}


/**
 * ccode_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK ccode_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			if (!ccode_window_child_windows_created)
				ccode_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					ccode_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					ccode_window_save();
					break;
				case IDC_COUNTRY_CODE_UP:
					ccode_window_callback_btnUp_clicked();
					break;
				case IDC_COUNTRY_CODE_DOWN:
					ccode_window_callback_btnDown_clicked();
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != ccode_window)
				break;
			
			ccode_window = NULL;
			
			// Destroy the image lists.
			if (imglArrowUp)
			{
				ImageList_Destroy(imglArrowUp);
				imglArrowUp = NULL;
			}
			if (imglArrowDown)
			{
				ImageList_Destroy(imglArrowDown);
				imglArrowDown = NULL;
			}
			
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * ccode_window_callback_btnUp_clicked(): "Up" button was clicked.
 */
static void ccode_window_callback_btnUp_clicked(void)
{
	int curIndex = ListBox_GetCurSel(lstCountryCodes);
	if (curIndex == -1 || curIndex == 0)
		return;
	
	// Swap the current item and the one above it.
	char aboveItem[32]; int aboveItemData;
	char curItem[32];   int curItemData;
	
	// Get the current text and data for each item.
	ListBox_GetText(lstCountryCodes, curIndex - 1, aboveItem);
	ListBox_GetText(lstCountryCodes, curIndex, curItem);
	aboveItemData = ListBox_GetItemData(lstCountryCodes, curIndex - 1);
	curItemData = ListBox_GetItemData(lstCountryCodes, curIndex);
	
	// Swap the strings and data.
	ListBox_DeleteString(lstCountryCodes, curIndex - 1);
	ListBox_InsertString(lstCountryCodes, curIndex - 1, curItem);
	ListBox_SetItemData(lstCountryCodes, curIndex - 1, curItemData);
	
	ListBox_DeleteString(lstCountryCodes, curIndex);
	ListBox_InsertString(lstCountryCodes, curIndex, aboveItem);
	ListBox_SetItemData(lstCountryCodes, curIndex, aboveItemData);
	
	// Set the current selection.
	ListBox_SetCurSel(lstCountryCodes, curIndex - 1);
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, TRUE);
}


/**
 * ccode_window_callback_btnDown_clicked(): "Down" button was clicked.
 */
static void ccode_window_callback_btnDown_clicked(void)
{
	int curIndex = ListBox_GetCurSel(lstCountryCodes);
	if (curIndex == -1 || curIndex >= (ListBox_GetCount(lstCountryCodes) - 1))
		return;
	
	// Swap the current item and the one below it.
	char curItem[32];   int curItemData;
	char belowItem[32]; int belowItemData;
	
	// Get the current text and data for each item.
	ListBox_GetText(lstCountryCodes, curIndex, curItem);
	ListBox_GetText(lstCountryCodes, curIndex + 1, belowItem);
	curItemData = ListBox_GetItemData(lstCountryCodes, curIndex);
	belowItemData = ListBox_GetItemData(lstCountryCodes, curIndex + 1);
	
	// Swap the strings and data.
	ListBox_DeleteString(lstCountryCodes, curIndex);
	ListBox_InsertString(lstCountryCodes, curIndex, belowItem);
	ListBox_SetItemData(lstCountryCodes, curIndex, belowItemData);
	
	ListBox_DeleteString(lstCountryCodes, curIndex + 1);
	ListBox_InsertString(lstCountryCodes, curIndex + 1, curItem);
	ListBox_SetItemData(lstCountryCodes, curIndex + 1, curItemData);
	
	// Set the current selection.
	ListBox_SetCurSel(lstCountryCodes, curIndex + 1);
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, TRUE);
}
