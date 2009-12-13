/***************************************************************************
 * Gens: (Win32) Button Control - Color Handler.                           *
 *                                                                         *
 * Copyright (c) 2002 by John Wellbelove.                                  *
 * Copyright (c) 2004 by Marius Bancila.                                   *
 * Non-MFC version Copyright (c) 2008-2009 by David Korth                  *
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

// Original code from http://www.codeguru.com/cpp/controls/buttonctrl/advancedbuttons/article.php/c8395/

#include "btncolor.hpp"

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;


WINAPI BtnColor::BtnColor(HWND hWnd)
{
	m_hWnd = hWnd;
	
	// Set the button to OwnerDraw.
	LONG_PTR lStyle = pGetWindowLongPtrU(hWnd, GWL_STYLE);
	lStyle &= ~(BS_AUTOCHECKBOX | BS_AUTORADIOBUTTON | BS_CHECKBOX | BS_DEFPUSHBUTTON |
		    BS_GROUPBOX | BS_PUSHBUTTON | BS_RADIOBUTTON);
	lStyle |= BS_OWNERDRAW;
	pSetWindowLongPtrU(hWnd, GWL_STYLE, lStyle);
	
	m_colorText		= GetSysColor(COLOR_BTNTEXT);
	m_colorBgNormal		= GetSysColor(COLOR_BTNFACE);
	m_colorBgDisabled	= GetSysColor(COLOR_BTNFACE);
	m_colorLight		= GetSysColor(COLOR_3DLIGHT);
	m_colorHighlight	= GetSysColor(COLOR_BTNHIGHLIGHT);
	m_colorShadow		= GetSysColor(COLOR_BTNSHADOW);
	m_colorDarkShadow	= GetSysColor(COLOR_3DDKSHADOW);
}

WINAPI BtnColor::~BtnColor()
{
}


void WINAPI BtnColor::setBgColor(COLORREF newBgColor)
{
	m_colorBgNormal = newBgColor;
	
	// Set the text color based on the brightness of the background color.
	int r = (m_colorBgNormal & 0xFF);
	int g = ((m_colorBgNormal >> 8) & 0xFF);
	int b = ((m_colorBgNormal >> 16) & 0xFF);
	
	int Y = (int)(((float)r * 0.30f) + ((float)g * 0.59f) + ((float)b * 0.11f));
	Y = (Y * 3) / 4;
	if (Y < 0)
		Y = 0;
	else if (Y > 0xFF)
		Y = 0xFF;
	
	if (Y >= 0x60)	// Using 0x60 because 0x80 doesn't seem to work at some high brightnesses.
	{
		// Text color should be black.
		m_colorText = ColorBtn_Black;
	}
	else
	{
		// Text color should be white.
		m_colorText = ColorBtn_White;
	}
	
	InvalidateRect(m_hWnd, NULL, true);
}


void WINAPI BtnColor::handleDrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
	// Draw the item.
	HDC hDC;
	RECT rcFocus, rcButton, rcText, rcOffsetText;
	unsigned int state;
	
	hDC = lpDrawItem->hDC;
	state = lpDrawItem->itemState;
	
	CopyRect(&rcFocus, &lpDrawItem->rcItem);
	CopyRect(&rcButton, &lpDrawItem->rcItem);
	CopyRect(&rcText, &lpDrawItem->rcItem);
	
	OffsetRect(&rcText, -1, -1);
	CopyRect(&rcOffsetText, &rcText);
	OffsetRect(&rcOffsetText, 1, 1);
	
	// Set the focus rectangle to just past the border decoration.
	rcFocus.left	+= 4;
	rcFocus.right	-= 4;
	rcFocus.top	+= 4;
	rcFocus.bottom	-= 4;
	
	// Get the button's caption.
	char buf[1024];
	Button_GetTextU(m_hWnd, buf, sizeof(buf));
	buf[sizeof(buf)-1] = 0x00;
	string strCaption = string(buf);
	
	if (state & ODS_DISABLED)
		DrawFilledRect(hDC, rcButton, m_colorBgDisabled);
	else
		DrawFilledRect(hDC, rcButton, m_colorBgNormal);
	
	if (state & ODS_SELECTED)
	{
		DrawFrame(hDC, rcButton, BUTTON_IN);
	}
	else
	{
		if ((state & ODS_DEFAULT) || (state & ODS_FOCUS))
			DrawFrame(hDC, rcButton, (BUTTON_OUT | BUTTON_BLACK_BORDER));
		else
			DrawFrame(hDC, rcButton, BUTTON_OUT);
	}
	
	if (state & ODS_DISABLED)
	{
		DrawButtonText(hDC, rcOffsetText, strCaption, ColorBtn_White);
		DrawButtonText(hDC, rcText, strCaption, ColorBtn_DarkGray);
	}
	else
	{
		if (state & ODS_SELECTED)
			DrawButtonText(hDC, rcOffsetText, strCaption, m_colorText);
		else
			DrawButtonText(hDC, rcText, strCaption, m_colorText);
	}
}


void WINAPI BtnColor::DrawFilledRect(HDC hDC, RECT &rc, COLORREF color)
{
	HBRUSH brSolid = CreateSolidBrush(color);
	FillRect(hDC, &rc, brSolid);
	DeleteBrush(brSolid);
}


void WINAPI BtnColor::DrawLine(HDC hDC, int x1, int y1, int x2, int y2, COLORREF color)
{
	HPEN newPen;
	HPEN oldPen;
	
	newPen = CreatePen(PS_SOLID, 1, color);
	oldPen = SelectPen(hDC, newPen);
	
	MoveToEx(hDC, x1, y1, NULL);
	LineTo(hDC, x2, y2);
	
	SelectPen(hDC, oldPen);
	DeletePen(newPen);
}


void WINAPI BtnColor::DrawFrame(HDC hDC, RECT &rc, int state)
{
	COLORREF color;
	
	if (state & BUTTON_BLACK_BORDER)
	{
		color = ColorBtn_Black;
		
		DrawLine(hDC, rc.left, rc.top, rc.right, rc.top,    color);	// Across top
		DrawLine(hDC, rc.left, rc.top, rc.left,  rc.bottom, color);	// Down left
		
		DrawLine(hDC, rc.left,    rc.bottom-1, rc.right,   rc.bottom-1, color);	// Across bottom
		DrawLine(hDC, rc.right-1, rc.top,      rc.right-1, rc.bottom,   color);	// Down right
		
		InflateRect(&rc, -1, -1);
	}
	
	if (state & BUTTON_OUT)
	{
		color = m_colorHighlight;
		
		DrawLine(hDC, rc.left, rc.top, rc.right, rc.top,    color);	// Across top
		DrawLine(hDC, rc.left, rc.top, rc.left,  rc.bottom, color);	// Down left
		
		color = m_colorDarkShadow;
		
		DrawLine(hDC, rc.left,    rc.bottom-1, rc.right,   rc.bottom-1, color);	// Across bottom
		DrawLine(hDC, rc.right-1, rc.top,      rc.right-1, rc.bottom,   color);	// Down right
		
		InflateRect(&rc, -1, -1);
		
		color = m_colorLight;
		
		DrawLine(hDC, rc.left, rc.top, rc.right, rc.top,    color);	// Across top
		DrawLine(hDC, rc.left, rc.top, rc.left,  rc.bottom, color);	// Down left
		
		color = m_colorShadow;
		
		DrawLine(hDC, rc.left,    rc.bottom-1, rc.right,   rc.bottom-1, color);	// Across bottom
		DrawLine(hDC, rc.right-1, rc.top,      rc.right-1, rc.bottom,   color);	// Down right
	}
	
	if (state & BUTTON_IN)
	{
		color = m_colorDarkShadow;
		
		DrawLine(hDC, rc.left, rc.top, rc.right, rc.top,    color);	// Across top
		DrawLine(hDC, rc.left, rc.top, rc.left,  rc.bottom, color);	// Down left
		DrawLine(hDC, rc.left,    rc.bottom-1, rc.right,   rc.bottom-1, color);	// Across bottom
		DrawLine(hDC, rc.right-1, rc.top,      rc.right-1, rc.bottom,   color);	// Down right
		
		InflateRect(&rc, -1, -1);
		
		color = m_colorShadow;
		
		DrawLine(hDC, rc.left, rc.top, rc.right, rc.top,    color);	// Across top
		DrawLine(hDC, rc.left, rc.top, rc.left,  rc.bottom, color);	// Down left
		DrawLine(hDC, rc.left,    rc.bottom-1, rc.right,   rc.bottom-1, color);	// Across bottom
		DrawLine(hDC, rc.right-1, rc.top,      rc.right-1, rc.bottom,   color);	// Down right
	}
}


void WINAPI BtnColor::DrawButtonText(HDC hDC, RECT &rc, const string &strCaption, COLORREF textColor)
{
	LONG_PTR lStyle = pGetWindowLongPtrU(m_hWnd, GWL_STYLE);
	
	vector<string> vLines;
	
	if ((lStyle & BS_MULTILINE) == BS_MULTILINE)
	{
		// Multi-line button.
		// Split the lines into the array.
		
		int nLastIndex = 0;
		while (nLastIndex != string::npos && nLastIndex < strCaption.size())
		{
			int nIndex = strCaption.find('\n', nLastIndex);
			if (nIndex == string::npos)
			{
				// No more newlines.
				vLines.push_back(strCaption.substr(nLastIndex));
				break;
			}
			
			// Get the string up to the newline.
			string line = strCaption.substr(nLastIndex, (nIndex - nLastIndex));
			vLines.push_back(line);
			nLastIndex = nIndex + 1;
		}
	}
	else
	{
		// Single-line button.
		vLines.push_back(strCaption);
	}
	
	SIZE szText;
	pGetTextExtentPoint32U(hDC, strCaption.c_str(), strCaption.size(), &szText);
	
	COLORREF oldColor;
	
	oldColor = SetTextColor(hDC, textColor);
	SetBkMode(hDC, TRANSPARENT);
	
	int nStartPos = (((rc.bottom - rc.top) - (vLines.size() * szText.cy)) / 2) - 1;
	if ((lStyle & BS_TOP) == BS_TOP)
		nStartPos = rc.top + 2;
	if ((lStyle & BS_BOTTOM) == BS_BOTTOM)
		nStartPos = rc.bottom - (vLines.size() * szText.cy) - 2;
	if ((lStyle & BS_VCENTER) == BS_VCENTER)
		nStartPos = (((rc.bottom - rc.top) - (vLines.size() * szText.cy)) / 2) - 1;
	
	unsigned int uDrawStyles = 0;
	if ((lStyle & BS_CENTER) == BS_CENTER)
		uDrawStyles |= DT_CENTER;
	else
	{
		if ((lStyle & BS_LEFT) == BS_LEFT)
			uDrawStyles |= DT_LEFT;
		else if ((lStyle & BS_RIGHT) == BS_RIGHT)
			uDrawStyles |= DT_RIGHT;
		else if (uDrawStyles == 0)
			uDrawStyles = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	}
	
	for (size_t i = 0; i < vLines.size(); i++)
	{
		RECT textRc = rc;
		textRc.left -= 3;
		textRc.right -= 3;
		textRc.top = nStartPos + (szText.cy * i);
		textRc.bottom = nStartPos + (szText.cy * (i + 1));
		
		const string &line = vLines.at(i);
		pDrawTextU(hDC, line.c_str(), line.size(), &textRc, uDrawStyles);
	}
	
	SetTextColor(hDC, oldColor);
}
