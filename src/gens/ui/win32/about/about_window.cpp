/***************************************************************************
 * Gens: (Win32) About Window.                                             *
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

#include "about_window.hpp"
#include "ui/common/about_window_data.h"
#include "gens/gens_window.h"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// C++ includes.
#include <string.h>
using std::string;

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ui/win32/fonts.h"
#include "ui/win32/resource.h"
#include "charset/cp1252.hpp"

// libgsft includes.
#include "libgsft/gsft_win32.h"

// git version
#include "macros/git.h"

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

// Audio Handler.
#include "audio/audio.h"


// Window.
HWND about_window = NULL;

// Window class.
static WNDCLASS about_wndclass;

#ifdef GENS_GIT_VERSION
#define ABOUT_WINDOW_GIT_HEIGHT 16
#else
#define ABOUT_WINDOW_GIT_HEIGHT 0
#endif

// Window size.
#define ABOUT_WINDOW_WIDTH  344
#define ABOUT_WINDOW_HEIGHT (396+ABOUT_WINDOW_GIT_HEIGHT)

// Logo/Verison Height.

// Tab content size.
#define TAB_TOP    (96+ABOUT_WINDOW_GIT_HEIGHT)
#define TAB_WIDTH  (ABOUT_WINDOW_WIDTH-16)
#define TAB_HEIGHT (ABOUT_WINDOW_HEIGHT-TAB_TOP-16-24)

// Timer ID.
#define IDT_ICETIMER 0x1234

// ice offsets.
#define ICE_OFFSET_X 32
#define ICE_OFFSET_Y 8

// Window procedure.
static LRESULT CALLBACK about_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Subclassed window procedure for the tab groupbox.
static LRESULT CALLBACK about_window_grpTabContents_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	tabInfo;
static HWND	lblGensTitle;
static HWND	lblGensDesc;
static HWND	imgGensLogo;
static HWND	lblTabContents;

// Old window procedure for grpTabContents.
static WNDPROC	grpTabContents_old_wndproc;

// Gens logo.
static HBITMAP	hbmpGensLogo = NULL;
static LPBYTE	pbmpData = NULL;
static HDC	hdcComp = NULL;

// Widget creation functions.
static void	about_window_create_child_windows(HWND hWnd);

// ice variables and functions.
static uint8_t		ax, bx, cx;
static unsigned int	iceLastTicks;
static UINT_PTR		tmrIce = NULL;
static void		about_window_update_ice(void);
static void CALLBACK	about_window_callback_iceTimer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


/**
 * about_window_show(): Show the About window.
 */
void about_window_show(void)
{
	if (about_window)
	{
		// About window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(about_window, SW_SHOW);
		return;
	}
	
	// Create the window class.
	if (about_wndclass.lpfnWndProc != about_window_wndproc)
	{
		about_wndclass.style = 0;
		about_wndclass.lpfnWndProc = about_window_wndproc;
		about_wndclass.cbClsExtra = 0;
		about_wndclass.cbWndExtra = 0;
		about_wndclass.hInstance = ghInstance;
		about_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		about_wndclass.hCursor = NULL;
		about_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		about_wndclass.lpszMenuName = NULL;
		about_wndclass.lpszClassName = TEXT("about_window");
		
		RegisterClass(&about_wndclass);
	}
	
	// Create the window.
	about_window = CreateWindow(TEXT("about_window"), TEXT("About " GENS_APPNAME),
				    WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    ABOUT_WINDOW_WIDTH, ABOUT_WINDOW_HEIGHT,
				    gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(about_window, ABOUT_WINDOW_WIDTH, ABOUT_WINDOW_HEIGHT);
	
	// Center the window on the Gens window.
	gsft_win32_center_on_window(about_window, gens_window);
	
	UpdateWindow(about_window);
	ShowWindow(about_window, SW_SHOW);
}


/**
 * about_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void about_window_create_child_windows(HWND hWnd)
{
	cx = 0; iceLastTicks = 0;
	if (ice != 3)
	{
		// Gens logo
		imgGensLogo = CreateWindow(WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP,
					   12, 0, 128, 96, hWnd, NULL, ghInstance, NULL);
		hbmpGensLogo = (HBITMAP)LoadImage(ghInstance, MAKEINTRESOURCE(IDB_GENS_LOGO_SMALL),
						 IMAGE_BITMAP, 0, 0,
						 LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		SendMessage(imgGensLogo, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbmpGensLogo);
	}
	else
	{
		// "ice" timer
		hdcComp = CreateCompatibleDC(GetDC(hWnd));
		
		// Create the DIB.
		BITMAPINFOHEADER bih;
		memset(&bih, 0x00, sizeof(bih));
		bih.biSize	= sizeof(bih);
		bih.biPlanes	= 1;
		bih.biBitCount	= 32;
		bih.biWidth	= 80;
		bih.biHeight	= -80;
		hbmpGensLogo = CreateDIBSection(hdcComp, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (LPVOID*)&pbmpData, NULL, 0);
		SelectBitmap(hdcComp, hbmpGensLogo);
		
		ax = 0; bx = 0; cx = 1;
		tmrIce = SetTimer(hWnd, IDT_ICETIMER, 10, about_window_callback_iceTimer);
		
		about_window_update_ice();
	}
	
	// Title and version information.
	lblGensTitle = CreateWindow(WC_STATIC, about_window_title, WS_CHILD | WS_VISIBLE | SS_CENTER,
				    128, 8, (ABOUT_WINDOW_WIDTH-128), 32+ABOUT_WINDOW_GIT_HEIGHT,
				    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblGensTitle, fntTitle, true);
	
	lblGensDesc = CreateWindow(WC_STATIC, about_window_description, WS_CHILD | WS_VISIBLE | SS_CENTER,
				   128, 42+ABOUT_WINDOW_GIT_HEIGHT,
				   (ABOUT_WINDOW_WIDTH-128), 100,
				   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblGensDesc, fntMain, true);
	
	// Tab control.
	tabInfo = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,
			       8, TAB_TOP, TAB_WIDTH, TAB_HEIGHT,
			       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(tabInfo, fntMain, true);
	
	// Make sure the tab control is in front of all other windows.
	SetWindowPos(tabInfo, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	
	// Add tabs for "Copyright" and "Included Libraries".
	TCITEM tab;
	memset(&tab, 0x00, sizeof(tab));
	tab.mask = TCIF_TEXT;
	tab.pszText = TEXT("&Copyright");
	TabCtrl_InsertItem(tabInfo, 0, &tab);
	tab.pszText = TEXT("Included &Libraries");
	TabCtrl_InsertItem(tabInfo, 1, &tab);
	
	// Calculate the tab's display area.
	RECT rectTab;
	rectTab.left = 0;
	rectTab.top = 0;
	rectTab.right = TAB_WIDTH;
	rectTab.bottom = TAB_HEIGHT;
	TabCtrl_AdjustRect(tabInfo, false, &rectTab);
	
	// Box for the tab contents.
	HWND grpTabContents;
	grpTabContents = CreateWindow(WC_BUTTON, NULL, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				      rectTab.left + 4, rectTab.top,
				      rectTab.right - rectTab.left - 8,
				      rectTab.bottom - rectTab.top - 4,
				      tabInfo, NULL, ghInstance, NULL);
	SetWindowFont(grpTabContents, fntMain, true);
	
	// Subclass the tab box.
	grpTabContents_old_wndproc = (WNDPROC)SetWindowLongPtr(grpTabContents, GWL_WNDPROC,
						(LONG_PTR)about_window_grpTabContents_wndproc);
	
	// Tab contents.
	string sTabContents = charset_utf8_to_cp1252(about_window_copyright);
	lblTabContents = CreateWindow(WC_STATIC, sTabContents.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT,
				      8, 16,
				      rectTab.right - rectTab.left - 24,
				      rectTab.bottom - rectTab.top - 32,
				      grpTabContents, NULL, ghInstance, NULL);
	SetWindowFont(lblTabContents, fntMain, true);
	
	// Create the OK button.
	HWND btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  ABOUT_WINDOW_WIDTH-8-75, ABOUT_WINDOW_HEIGHT-8-24,
				  75, 23,
				  hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Set focus to the OK button.
	SetFocus(btnOK);
}


/**
 * about_window_close(): Close the About window.
 */
void about_window_close(void)
{
	if (!about_window)
		return;
	
	// Stop the timer.
	if (tmrIce)
	{
		KillTimer(about_window, tmrIce);
		tmrIce = NULL;
	}
	
	// Destroy the window.
	DestroyWindow(about_window);
	about_window = NULL;
	
	// Make sure the bitmap and DC are deleted.
	if (hbmpGensLogo)
	{
		DeleteBitmap(hbmpGensLogo);
		hbmpGensLogo = NULL;
	}
	if (hdcComp)
	{
		DeleteDC(hdcComp);
		hdcComp = NULL;
	}
}


/**
 * about_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK about_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			about_window_create_child_windows(hWnd);
			break;
		
		case WM_PAINT:
			if (ice == 3)
				about_window_update_ice();
			break;
		
		case WM_CTLCOLORSTATIC:
			// Set the title and version labels to transparent.
			if ((HWND)lParam == lblGensTitle ||
			    (HWND)lParam == lblGensDesc ||
			    (HWND)lParam == imgGensLogo)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
			{
				// Tab change.
				string sTabContents;
				
				switch (TabCtrl_GetCurSel(tabInfo))
				{
					case 0:
						// Copyright.
						sTabContents = charset_utf8_to_cp1252(about_window_copyright);
						break;
					case 1:
						// Included Libraries.
						sTabContents = charset_utf8_to_cp1252(about_window_included_libs);
						break;
					default:
						// Unknown.
						break;
				}
				
				Static_SetText(lblTabContents, sTabContents.c_str());
				
				// Invalidate the tab contents groupbox.
				InvalidateRect(tabInfo, NULL, true);
				UpdateWindow(about_window);
			}
			break;
			
		case WM_DESTROY:
			if (hWnd != about_window)
				break;
			
			// Stop the timer.
			if (tmrIce)
			{
				KillTimer(about_window, tmrIce);
				tmrIce = NULL;
			}
			
			// Clear the About window hWnd.
			about_window = NULL;
			
			// Make sure the bitmap and DC are deleted.
			if (hbmpGensLogo)
			{
				DeleteBitmap(hbmpGensLogo);
				hbmpGensLogo = NULL;
			}
			if (hdcComp)
			{
				DeleteDC(hdcComp);
				hdcComp = NULL;
			}
			
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * about_window_grpTabContents_wndproc(): Subclassed window procedure for the tab groupbox.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK about_window_grpTabContents_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CTLCOLORSTATIC &&
	    (HWND)lParam == lblTabContents)
	{
		// Set the tab contents label to transparent.
		SetBkMode((HDC)wParam, TRANSPARENT);
		return (LRESULT)GetStockObject(NULL_BRUSH);
	}
	
	return CallWindowProc(grpTabContents_old_wndproc, hWnd, message, wParam, lParam);
}


#define ICE_RGB(r, g, b) (((r) << 16) | ((g) << 8) | (b))

static void about_window_update_ice(void)
{
	int x, y;
	const unsigned char *src = &about_window_data[ax*01440];
	const unsigned char *src2 = &about_window_dx[bx*040];
	
	unsigned int bgc;
	
	bgc = GetSysColor(COLOR_3DFACE);
	bgc = ((bgc >> 16) & 0xFF) | (bgc & 0xFF00) | ((bgc & 0xFF) << 16);
	
	unsigned int *destPixel1 = (unsigned int*)pbmpData;
	unsigned int *destPixel2 = destPixel1 + 0120;
	
	for (y = 0; y < 0120; y += 2)
	{
		for (x = 0; x < 0120; x += 4)
		{
			unsigned int pxc;
			
			unsigned char px1 = (*src & 0360) >> 3;
			unsigned char px2 = (*src & 0017) << 1;
			
			pxc = (!px1 ? bgc : ICE_RGB(((src2[px1 + 1] & 0017) << 4),
						     (src2[px1 + 1] & 0360),
						    ((src2[px1 + 0] & 0017) << 4)));
			*destPixel1++ = pxc;
			*destPixel1++ = pxc;
			*destPixel2++ = pxc;
			*destPixel2++ = pxc;
			
			pxc = (!px2 ? bgc : ICE_RGB(((src2[px2 + 1] & 0017) << 4),
						     (src2[px2 + 1] & 0360),
						    ((src2[px2 + 0] & 0017) << 4)));
			*destPixel1++ = pxc;
			*destPixel1++ = pxc;
			*destPixel2++ = pxc;
			*destPixel2++ = pxc;
			
			src++;
		}
		
		destPixel1 += 0120;
		destPixel2 += 0120;
	}
	
	HDC hDC;
	PAINTSTRUCT ps;
	
	hDC = BeginPaint(about_window, &ps);
	BitBlt(hDC, ICE_OFFSET_X, ICE_OFFSET_Y, 0120, 0120, hdcComp, 0, 0, SRCCOPY);
	EndPaint(about_window, &ps);
}


static void CALLBACK about_window_callback_iceTimer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	GSFT_UNUSED_PARAMETER(uMsg);
	
	if (!(hWnd == about_window && idEvent == IDT_ICETIMER && ice == 3))
		return;
	
	if (iceLastTicks + 100 > dwTime)
		return;
	
	if (!cx)
		return;
	
	ax ^= 1;
	bx++;
	if (bx >= 10)
		bx = 0;
	
	// Force a repaint.
	RECT rIce;
	rIce.left = ICE_OFFSET_X;
	rIce.top = ICE_OFFSET_Y;
	rIce.right = ICE_OFFSET_X + 80 - 1;
	rIce.bottom = ICE_OFFSET_Y + 80 - 1;
	InvalidateRect(about_window, &rIce, false);
	SendMessage(about_window, WM_PAINT, 0, 0);
	
	iceLastTicks = dwTime;
}
