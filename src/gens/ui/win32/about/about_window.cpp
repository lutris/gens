/***************************************************************************
 * Gens: (Win32) About Window.                                             *
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

#include "about_window.hpp"
#include "gens/gens_window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"

// Character set conversion
#include "ui/charset.hpp"

static WNDCLASS WndClass;
HWND about_window = NULL;

// Labels and boxes
static HWND lblGensTitle = NULL;
static HWND lblGensDesc = NULL;
static HWND grpGensCopyright = NULL;
static HWND lblGensCopyright = NULL;

// OK button
static HWND btnOK = NULL;

// Gens logo
static HBITMAP bmpGensLogo = NULL;

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include "commctrl.h"

#include "ui/about_window_data.h"
//GtkWidget *image_gens_logo = NULL;
#define ID_TIMER_ICE 0x1234
static UINT_PTR tmrIce = NULL;
static void updateIce(void);
static void iceTime(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

const unsigned short iceOffsetX = 20;
const unsigned short iceOffsetY = 8;
unsigned int iceLastTicks = 0;
unsigned short ax = 0, bx = 0, cx = 0;

LRESULT CALLBACK About_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void About_Window_CreateChildWindows(HWND hWnd);


/**
 * create_about_window(): Create the About Window.
 * @return About Window.
 */
HWND create_about_window(void)
{
	if (about_window)
	{
		// About window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(about_window, 1);
		return about_window;
	}
	
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = About_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_About";
	
	RegisterClass(&WndClass);
	
	about_window = CreateWindowEx(NULL, "Gens_About", "About Gens",
				      (WS_POPUP | WS_SYSMENU | WS_CAPTION) & ~(WS_MINIMIZE),
				      CW_USEDEFAULT, CW_USEDEFAULT,
				      320 + Win32_dw, 288 + 4 + Win32_dh, NULL, NULL, ghInstance, NULL);
	
	ShowWindow(about_window, 1);
	UpdateWindow(about_window);
	return about_window;
}


/**
 * About_Window_WndProc(): The About window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK About_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			About_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(about_window);
			return 0;
		
		case WM_PAINT:
			if (ice == 3)
				updateIce();
			break;
		
		case WM_CTLCOLORSTATIC:
			if (hWnd != about_window)
				break;
			
			// Set the title and version labels to transparent.
			if ((HWND)lParam == lblGensTitle ||
			    (HWND)lParam == lblGensDesc)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
			return TRUE;
			break;
		
		case WM_COMMAND:
			if (LOWORD(wParam) == 0x8472)
				DestroyWindow(about_window);
			break;
		
		case WM_DESTROY:
			if (hWnd != about_window)
				break;
			
			if (tmrIce)
			{
				KillTimer(about_window, tmrIce);
				tmrIce = 0;
			}
			if (bmpGensLogo)
			{
				DeleteObject(bmpGensLogo);
				bmpGensLogo = NULL;
			}
			cx = 0;
			lblGensTitle = NULL;
			lblGensDesc = NULL;
			grpGensCopyright = NULL;
			lblGensCopyright = NULL;
			about_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


static void About_Window_CreateChildWindows(HWND hWnd)
{
	Win32_centerOnGensWindow(hWnd);
	
	if (ice != 3)
	{
		// Gens logo
		HWND imgGensLogo;
		imgGensLogo = CreateWindow("Static", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP,
					   0, 0, 128, 96, hWnd, NULL, ghInstance, NULL);
		bmpGensLogo = (HBITMAP)LoadImage(ghInstance, MAKEINTRESOURCE(IDB_GENS_LOGO_SMALL),
						 IMAGE_BITMAP, 0, 0,
						 LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		SendMessage(imgGensLogo, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpGensLogo);
	}
	else
	{
		// "ice" timer
		ax = 0; bx = 0; cx = 1;
		tmrIce = SetTimer(hWnd, ID_TIMER_ICE, 10, (TIMERPROC)iceTime);
		updateIce();
	}
	
	// Title and version information.
	lblGensTitle = CreateWindow(WC_STATIC, aboutTitle, WS_CHILD | WS_VISIBLE | SS_CENTER,
				    128, 8, 184, 24, hWnd, NULL, ghInstance, NULL);
	SendMessage(lblGensTitle, WM_SETFONT, (WPARAM)fntTitle, 1);
	
	lblGensDesc = CreateWindow(WC_STATIC, aboutDesc, WS_CHILD | WS_VISIBLE | SS_CENTER,
				   128, 44, 184, 100, hWnd, NULL, ghInstance, NULL);
	SendMessage(lblGensDesc, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Box for the copyright message.
	grpGensCopyright = CreateWindow(WC_BUTTON, "", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					8, 88, 304, 160, hWnd, NULL, ghInstance, NULL);
	
	// Copyright message.
	string sCopyright = charset_utf8_to_cp1252(aboutCopyright);
	lblGensCopyright = CreateWindow(WC_STATIC, sCopyright.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 16, 288, 136, grpGensCopyright, NULL, ghInstance, NULL);
	SendMessage(lblGensCopyright, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// OK button
	btnOK = CreateWindow(WC_STATIC, "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 312 - 75, 256, 75, 23,
			     hWnd, (HMENU)0x8472, ghInstance, NULL);
	SendMessage(btnOK, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Set focus to the OK button.
	SetFocus(btnOK);
}


static void updateIce(void)
{
	HDC hDC;
	PAINTSTRUCT ps;
	
	hDC = BeginPaint(about_window, &ps);
	
	int x, y;
	const unsigned char *src = &about_data[ax*01440];
	const unsigned char *src2 = &about_dx[bx*040];
	unsigned char px1, px2;
	
	int bgc = GetSysColor(COLOR_3DFACE);
	int pxc;
	
	for (y = 0; y < 0120; y += 2)
	{
		for (x = 0; x < 0120; x += 4)
		{
			px1 = (*src & 0360) >> 3;
			px2 = (*src & 0017) << 1;
			
			if (!px1)
			{
				pxc = bgc;
			}
			else
			{
				pxc = RGB((src2[px1 + 1] & 0017) << 4,
					  (src2[px1 + 1] & 0360),
					  (src2[px1 + 0] & 0017) << 4);
			}
			
			SetPixel(hDC, x + 0 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 1 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 0 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			SetPixel(hDC, x + 1 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			
			if (!px2)
			{
				pxc = bgc;
			}
			else
			{
				pxc = RGB((src2[px2 + 1] & 0017) << 4,
					  (src2[px2 + 1] & 0360),
					  (src2[px2 + 0] & 0017) << 4);
			}
			SetPixel(hDC, x + 2 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 3 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 2 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			SetPixel(hDC, x + 3 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			
			src++;
		}
	}
	
	EndPaint(about_window, &ps);
}


static void iceTime(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (!(hWnd == about_window && idEvent == ID_TIMER_ICE && ice == 3))
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
	rIce.left = iceOffsetX;
	rIce.top = iceOffsetY;
	rIce.right = iceOffsetX + 80 - 1;
	rIce.bottom = iceOffsetY + 80 - 1;
	InvalidateRect(about_window, &rIce, FALSE);
	SendMessage(about_window, WM_PAINT, 0, 0);
	
	iceLastTicks = dwTime;
}
