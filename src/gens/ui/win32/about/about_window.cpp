/***************************************************************************
 * Gens: (Win32) About Window.                                             *
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
#include <string>
#include <sstream>
using std::string;
using std::stringstream;

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"
#include "libgsft/w32u/w32u_winnls.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"

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
#define ABOUT_WINDOW_GIT_HEIGHT 10
#else
#define ABOUT_WINDOW_GIT_HEIGHT 0
#endif

// Window size. (NOTE: THESE ARE IN DIALOG UNITS, and must be converted to pixels using DLU_X() / DLU_Y().)
#define ABOUT_WINDOW_WIDTH  210
#define ABOUT_WINDOW_HEIGHT (220+ABOUT_WINDOW_GIT_HEIGHT)

// Logo/Verison Height.

// Tab content size.
#define TAB_TOP    (60+ABOUT_WINDOW_GIT_HEIGHT)
#define TAB_WIDTH  (ABOUT_WINDOW_WIDTH-10)
#define TAB_HEIGHT (ABOUT_WINDOW_HEIGHT-TAB_TOP-10-15)

// Timer ID.
#define IDT_ICETIMER 0x1234

// ice offsets.
static int ice_offset_x;
static int ice_offset_y;

// Window procedure.
static LRESULT CALLBACK about_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Tab callbacks.
static LRESULT CALLBACK about_window_grpTabContents_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void WINAPI about_window_tabInfo_selChanged(void);

// Debug Info.
static string sDebugInfo;
static void WINAPI about_window_buildDebugInfoString(void);

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
static void WINAPI about_window_create_child_windows(HWND hWnd);

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
		about_wndclass.hIcon = LoadIconA(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		about_wndclass.hCursor = NULL;
		about_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		about_wndclass.lpszMenuName = NULL;
		about_wndclass.lpszClassName = "about_window";
		
		pRegisterClassU(&about_wndclass);
	}
	
	// Create the window.
	about_window = pCreateWindowU("about_window", "About " GENS_APPNAME,
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DLU_X(ABOUT_WINDOW_WIDTH), DLU_Y(ABOUT_WINDOW_HEIGHT),
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(about_window, DLU_X(ABOUT_WINDOW_WIDTH), DLU_Y(ABOUT_WINDOW_HEIGHT));
	
	// Center the window on the Gens window.
	gsft_win32_center_on_window(about_window, gens_window);
	
	UpdateWindow(about_window);
	ShowWindow(about_window, SW_SHOW);
}


/**
 * about_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void WINAPI about_window_create_child_windows(HWND hWnd)
{
	cx = 0; iceLastTicks = 0;
	
	if (ice != 3)
	{
		// Gens logo. (Approximate area: 80 DLU_X, 58 DLU Y.)
		const int pos_x = ((DLU_X(80) - 128) / 2) + DLU_X(8);
		const int pos_y = ((DLU_Y(60) - 96) / 2);
		imgGensLogo = pCreateWindowU(WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP,
						pos_x, pos_y,
						128, 96,
						hWnd, NULL, ghInstance, NULL);
		hbmpGensLogo = (HBITMAP)LoadImageA(ghInstance, MAKEINTRESOURCE(IDB_GENS_LOGO_SMALL),
							IMAGE_BITMAP, 0, 0,
							LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		pSendMessageU(imgGensLogo, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbmpGensLogo);
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
		
		ice_offset_x = ((DLU_X(80) - 80) / 2) + 8;
		ice_offset_y = ((DLU_Y(60) - 80) / 2) + 8;
		
		ax = 0; bx = 0; cx = 1;
		tmrIce = SetTimer(hWnd, IDT_ICETIMER, 10, about_window_callback_iceTimer);
		
		about_window_update_ice();
	}
	
	// Title and version information.
	lblGensTitle = pCreateWindowU(WC_STATIC, about_window_title, WS_CHILD | WS_VISIBLE | SS_CENTER,
					DLU_X(80), DLU_Y(5),
					DLU_X(ABOUT_WINDOW_WIDTH-80), DLU_Y(20+ABOUT_WINDOW_GIT_HEIGHT),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblGensTitle, w32_fntTitle, true);
	
	lblGensDesc = pCreateWindowU(WC_STATIC, about_window_description, WS_CHILD | WS_VISIBLE | SS_CENTER,
					DLU_X(80), DLU_Y(26+ABOUT_WINDOW_GIT_HEIGHT),
					DLU_X(ABOUT_WINDOW_WIDTH-80), DLU_Y(65),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblGensDesc, w32_fntMessage, true);
	
	// Build the debug information string.
	about_window_buildDebugInfoString();
	
	// Tab control.
	tabInfo = pCreateWindowU(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,
				DLU_X(5), DLU_Y(TAB_TOP),
				DLU_X(TAB_WIDTH), DLU_Y(TAB_HEIGHT),
				hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(tabInfo, w32_fntMessage, true);
	
	// Make sure the tab control is in front of all other windows.
	SetWindowPos(tabInfo, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	
	// Add the tabs.
	TCITEM tab;
	memset(&tab, 0x00, sizeof(tab));
	tab.mask = TCIF_TEXT;
	tab.pszText = "&Copyright";
	pTabCtrl_InsertItemU(tabInfo, 0, &tab);
	tab.pszText = "Included &Libraries";
	pTabCtrl_InsertItemU(tabInfo, 1, &tab);
	tab.pszText = "&Debug Info";
	pTabCtrl_InsertItemU(tabInfo, 2, &tab);
	
	// Calculate the tab's display area.
	RECT rectTab;
	rectTab.left = 0;
	rectTab.top = 0;
	rectTab.right = DLU_X(TAB_WIDTH);
	rectTab.bottom = DLU_Y(TAB_HEIGHT);
	TabCtrl_AdjustRectU(tabInfo, false, &rectTab);
	
	// Box for the tab contents.
	HWND grpTabContents;
	grpTabContents = pCreateWindowU(WC_BUTTON, NULL, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					rectTab.left + 4, rectTab.top,
					rectTab.right - rectTab.left - 8,
					rectTab.bottom - rectTab.top - 4,
					tabInfo, NULL, ghInstance, NULL);
	SetWindowFontU(grpTabContents, w32_fntMessage, true);
	
	// Subclass the tab box.
	grpTabContents_old_wndproc = (WNDPROC)pSetWindowLongPtrU(grpTabContents, GWL_WNDPROC,
						(LONG_PTR)about_window_grpTabContents_wndproc);
	
	// Tab contents.
	lblTabContents = pCreateWindowU(WC_STATIC, about_window_copyright, WS_CHILD | WS_VISIBLE | SS_LEFT,
					DLU_X(5), DLU_Y(10),
					rectTab.right - rectTab.left - 24,
					rectTab.bottom - rectTab.top - 32,
					grpTabContents, NULL, ghInstance, NULL);
	SetWindowFontU(lblTabContents, w32_fntMessage, true);
	
	// Create the OK button.
	HWND btnOK = pCreateWindowU(WC_BUTTON, "&OK",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					DLU_X(ABOUT_WINDOW_WIDTH-5-50), DLU_Y(ABOUT_WINDOW_HEIGHT-5-14),
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFontU(btnOK, w32_fntMessage, true);
	
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
				return (LRESULT)GetStockBrush(NULL_BRUSH);
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
				about_window_tabInfo_selChanged();
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
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
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
	
	return pCallWindowProcU(grpTabContents_old_wndproc, hWnd, message, wParam, lParam);
}


static void WINAPI about_window_tabInfo_selChanged(void)
{
	// Tab change.
	const char *sTabContents = NULL;
	
	switch (TabCtrl_GetCurSelU(tabInfo))
	{
		case 0:
			// Copyright.
			sTabContents = about_window_copyright;
			break;
		case 1:
			// Included Libraries.
			sTabContents = about_window_included_libs;
			break;
		case 2:
			// Debug Information.
			sTabContents = sDebugInfo.c_str();
			break;
		default:
			// Unknown tab.
			sTabContents = "";
			break;
	}
	
	// Set the text.
	Static_SetTextU(lblTabContents, sTabContents);
	
	// Invalidate the tab contents groupbox.
	InvalidateRect(tabInfo, NULL, true);
	UpdateWindow(about_window);
}


static void WINAPI about_window_buildDebugInfoString(void)
{
	// Build the debug information string.
	stringstream ss;
	
	// Print the ANSI and OEM code pages.
	typedef struct
	{
		unsigned int cp;
		const char *cp_str;
	} cp_info_t;
	
	cp_info_t cp_info[2] =
	{
		{CP_ACP,   "System ANSI code page"},
		{CP_OEMCP, "System OEM code page"}
	};
	
	for (int i = 0; i < 2; i++)
	{
		ss << cp_info[i].cp_str << ": ";
		CPINFOEX cpix;
		BOOL bRet = pGetCPInfoExU(cp_info[i].cp, 0, &cpix);
		if (!bRet)
		{
			ss << "Unknown [GetCPInfoEx() failed]\n";
			continue;
		}
		
		ss << cpix.CodePage << " (";
		
		// Windows XP has the code page number in cpix.CodePageName,
		// followed by two spaces, and then the code page name in parentheses.
		char *parenStart = strchr(cpix.CodePageName, '(');
		if (!parenStart)
		{
			// No parentheses. Use the code page name as-is.
			ss << cpix.CodePageName;
		}
		else
		{
			// Found starting parenthesis. Check for ending parenthesis.
			char *parenEnd = strrchr(parenStart, ')');
			if (parenEnd)
			{
				// Found ending parenthesis. Null it out.
				*parenEnd = 0x00;
			}
			
			ss << (parenStart + 1);
		}
		ss << ")\n";
	}
	
	ss << "\n";
	
	// Is Gens/GS using Unicode?
	if (w32u_is_unicode)
		ss << "Using Unicode strings for Win32 API.\n";
	else
		ss << "Using ANSI strings for Win32 API.\n";
	
	// Save the debug information.
	sDebugInfo = ss.str();
}


/** ICE stuff. **/


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
	BitBlt(hDC, ice_offset_x, ice_offset_y, 0120, 0120, hdcComp, 0, 0, SRCCOPY);
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
	rIce.left = ice_offset_x;
	rIce.top = ice_offset_y;
	rIce.right = ice_offset_x + 80 - 1;
	rIce.bottom = ice_offset_y + 80 - 1;
	InvalidateRect(about_window, &rIce, false);
	pSendMessageU(about_window, WM_PAINT, 0, 0);
	
	iceLastTicks = dwTime;
}
