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
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// git version
#include "macros/git.h"

#include "emulator/g_main.hpp"

// Audio Handler.
#include "audio/audio.h"

#include "ui/win32/charset.hpp"
#include "ui/win32/resource.h"

#include <windowsx.h>
#include <commctrl.h>

// C++ includes
#include <string>
using std::string;

#define ID_TIMER_ICE 0x1234

#ifdef GENS_GIT_VERSION
static const unsigned short lblTitle_HeightInc = 16;
#else
static const unsigned short lblTitle_HeightInc = 0;
#endif /* GENS_GIT_VERSION */

static const unsigned short iceOffsetX = 32;
static const unsigned short iceOffsetY = 8;


static WNDCLASS m_WndClass;


AboutWindow* AboutWindow::m_Instance = NULL;
AboutWindow* AboutWindow::Instance(HWND parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the About window.
		m_Instance = new AboutWindow();
	}
	else
	{
		// Instance already exists. Set focus.
		m_Instance->setFocus();
	}
	
	// Set modality of the window.
	m_Instance->setModal(parent);
	
	return m_Instance;
}


/**
 * AboutWindow(): Create the About Window.
 */
AboutWindow::AboutWindow()
{
	tmrIce = NULL;
	m_hbmpGensLogo = NULL;
	m_childWindowsCreated = false;
	m_hdcComp = NULL;
	
	// Create the window class.
	if (m_WndClass.lpfnWndProc != WndProc_STATIC)
	{
		m_WndClass.style = 0;
		m_WndClass.lpfnWndProc = WndProc_STATIC;
		m_WndClass.cbClsExtra = 0;
		m_WndClass.cbWndExtra = 0;
		m_WndClass.hInstance = ghInstance;
		m_WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
		m_WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		m_WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		m_WndClass.lpszMenuName = NULL;
		m_WndClass.lpszClassName = "Gens_About";
		
		RegisterClass(&m_WndClass);
	}
	
	// Messages are processed before the object is finished being created,
	// so this assignment is needed.
	m_Instance = this;
	
	// Create the window.
	// TODO: Don't hardcode the parent window.
	m_Window = CreateWindowEx(NULL, "Gens_About", "About Gens",
				  WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				  CW_USEDEFAULT, CW_USEDEFAULT,
				  328, 352+lblTitle_HeightInc,
				  Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(m_Window, 328, 352+lblTitle_HeightInc);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(m_Window);
	
	UpdateWindow(m_Window);
	ShowWindow(m_Window, 1);
}


AboutWindow::~AboutWindow()
{
	if (m_hbmpGensLogo)
		DeleteBitmap(m_hbmpGensLogo);
	if (m_hdcComp)
		DeleteDC(m_hdcComp);
	
	m_Instance = NULL;
}


LRESULT CALLBACK AboutWindow::WndProc_STATIC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_Instance->WndProc(hWnd, message, wParam, lParam);
}


/**
 * About_Window_WndProc(): The About window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK AboutWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			if (!m_childWindowsCreated)
				createChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(m_Window);
			return 0;
		
		case WM_PAINT:
			if (ice == 3)
				updateIce();
			break;
		
		case WM_CTLCOLORSTATIC:
			if (hWnd != m_Window)
				break;
			
			// Set the title and version labels to transparent.
			if ((HWND)lParam == lblGensTitle ||
			    (HWND)lParam == lblGensDesc ||
			    (HWND)lParam == imgGensLogo)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
			return true;
			break;
		
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_BTN_OK || LOWORD(wParam) == IDOK ||
			    LOWORD(wParam) == IDC_BTN_CANCEL || LOWORD(wParam) == IDCANCEL)
			{
				DestroyWindow(m_Window);
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
						sTabContents = charset_utf8_to_cp1252(StrCopyright);
						break;
					case 1:
						// Included Libraries.
						sTabContents = charset_utf8_to_cp1252(StrIncludedLibs);
						break;
					default:
						// Unknown.
						break;
				}
				
				Static_SetText(lblTabContents, sTabContents.c_str());
			}
			break;
			
		case WM_DESTROY:
			if (hWnd != m_Window)
				break;
			
			if (tmrIce)
			{
				KillTimer(m_Window, tmrIce);
				tmrIce = 0;
			}
			
			delete this;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void AboutWindow::createChildWindows(HWND hWnd)
{
	if (m_childWindowsCreated)
		return;
	
	cx = 0; iceLastTicks = 0;
	if (ice != 3)
	{
		// Gens logo
		imgGensLogo = CreateWindow(WC_STATIC, NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP,
					   12, 0, 128, 96, hWnd, NULL, ghInstance, NULL);
		m_hbmpGensLogo = (HBITMAP)LoadImage(ghInstance, MAKEINTRESOURCE(IDB_GENS_LOGO_SMALL),
						 IMAGE_BITMAP, 0, 0,
						 LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		SendMessage(imgGensLogo, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hbmpGensLogo);
	}
	else
	{
		// "ice" timer
		
		m_hdcComp = CreateCompatibleDC(GetDC(Gens_hWnd));
		
		// Create the DIB.
		BITMAPINFOHEADER bih;
		memset(&bih, 0x00, sizeof(bih));
		bih.biSize	= sizeof(bih);
		bih.biPlanes	= 1;
		bih.biBitCount	= 32;
		bih.biWidth	= 80;
		bih.biHeight	= -80;
		m_hbmpGensLogo = CreateDIBSection(m_hdcComp, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (LPVOID*)&m_pbmpData, NULL, 0);
		SelectObject(m_hdcComp, m_hbmpGensLogo);
		
		ax = 0; bx = 0; cx = 1;
		tmrIce = SetTimer(hWnd, ID_TIMER_ICE, 10, (TIMERPROC)iceTime_STATIC);
		
		m_Window = hWnd;
		updateIce();
	}
	
	// Title and version information.
	lblGensTitle = CreateWindow(WC_STATIC, StrTitle, WS_CHILD | WS_VISIBLE | SS_CENTER,
				    128, 8, 192, 32+lblTitle_HeightInc,
				    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblGensTitle, fntTitle, true);
	
	lblGensDesc = CreateWindow(WC_STATIC, StrDescription, WS_CHILD | WS_VISIBLE | SS_CENTER,
				   128, 42+lblTitle_HeightInc, 192, 100,
				   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblGensDesc, fntMain, true);
	
	// Tab content size.
	static const unsigned int tabWidth = 312;
	static const unsigned int tabHeight = 224;
	
	// Tab control.
	tabInfo = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,
			       8, 88+lblTitle_HeightInc, tabWidth, tabHeight,
			       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(tabInfo, fntMain, true);
	
	// Make sure the tab control is in front of all other windows.
	SetWindowPos(tabInfo, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	
	// Add tabs for "Copyright" and "Included Libraries".
	TCITEM tab;
	memset(&tab, 0x00, sizeof(tab));
	tab.mask = TCIF_TEXT;
	tab.pszText = "Copyright";
	TabCtrl_InsertItem(tabInfo, 0, &tab);
	tab.pszText = "Included Libraries";
	TabCtrl_InsertItem(tabInfo, 1, &tab);
	
	// Calculate the tab's display area.
	RECT rectTab;
	rectTab.left = 0;
	rectTab.top = 0;
	rectTab.right = tabWidth;
	rectTab.bottom = tabHeight;
	TabCtrl_AdjustRect(tabInfo, false, &rectTab);
	
	// Box for the tab contents.
	HWND grpTabContents;
	grpTabContents = CreateWindow(WC_BUTTON, NULL, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				      rectTab.left + 4, rectTab.top,
				      rectTab.right - rectTab.left - 8,
				      rectTab.bottom - rectTab.top - 4,
				      tabInfo, NULL, ghInstance, NULL);
	SetWindowFont(grpTabContents, fntMain, true);
	
	// Tab contents.
	string sTabContents = charset_utf8_to_cp1252(StrCopyright);
	lblTabContents = CreateWindow(WC_STATIC, sTabContents.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT,
				      8, 16,
				      rectTab.right - rectTab.left - 24,
				      rectTab.bottom - rectTab.top - 32,
				      grpTabContents, NULL, ghInstance, NULL);
	SetWindowFont(lblTabContents, fntMain, true);
	
	// Add the OK button.
	addDialogButtons(hWnd, WndBase::BAlign_Right,
			 WndBase::BUTTON_OK, WndBase::BUTTON_OK);
	
	// Child windows created.
	m_childWindowsCreated = true;
}


#define ICE_RGB(r, g, b) (((r) << 16) | ((g) << 8) | (b))

void AboutWindow::updateIce(void)
{
	int x, y;
	const unsigned char *src = &Data[ax*01440];
	const unsigned char *src2 = &DX[bx*040];
	
	unsigned int bgc;
	
	bgc = GetSysColor(COLOR_3DFACE);
	bgc = ((bgc >> 16) & 0xFF) | (bgc & 0xFF00) | ((bgc & 0xFF) << 16);
	
	unsigned int *destPixel1 = (unsigned int*)m_pbmpData;
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
	
	hDC = BeginPaint(m_Window, &ps);
	BitBlt(hDC, iceOffsetX, iceOffsetY, 0120, 0120, m_hdcComp, 0, 0, SRCCOPY);
	EndPaint(m_Window, &ps);
}


void AboutWindow::iceTime_STATIC(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	m_Instance->iceTime(hWnd, uMsg, idEvent, dwTime);
}


void AboutWindow::iceTime(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (!(hWnd == m_Window && idEvent == ID_TIMER_ICE && ice == 3))
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
	InvalidateRect(m_Window, &rIce, false);
	SendMessage(m_Window, WM_PAINT, 0, 0);
	
	iceLastTicks = dwTime;
}
