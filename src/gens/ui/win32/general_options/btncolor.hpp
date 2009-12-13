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

#ifndef GENS_WIN32_BTNCOLOR_HPP
#define GENS_WIN32_BTNCOLOR_HPP

// Win32 includes.
#include "libgsft/w32u/w32u.h"

#ifdef __cplusplus

#include <string>

class BtnColor
{
	public:
		BtnColor(HWND hWnd);
		~BtnColor();
		
		void setBgColor(COLORREF newBgColor);
		
		void handleDrawItem(LPDRAWITEMSTRUCT lpDrawItem);
	
	protected:
		HWND m_hWnd;
		COLORREF m_colorText;
		COLORREF m_colorBgNormal;
		COLORREF m_colorBgDisabled;
		COLORREF m_colorLight;
		COLORREF m_colorHighlight;
		COLORREF m_colorShadow;
		COLORREF m_colorDarkShadow;
		
		static void DrawFilledRect(HDC hDC, RECT &rc, COLORREF color);
		static void DrawLine(HDC hDC, int x1, int y1, int x2, int y2, COLORREF color);
		
		void DrawFrame(HDC hDC, RECT &rc, int state);
		void DrawButtonText(HDC hDC, RECT &rc, const std::string &strCaption, COLORREF textColor);
		
	private:
		enum
		{
			BUTTON_IN		= 0x01,
			BUTTON_OUT		= 0x02,
			BUTTON_BLACK_BORDER	= 0x04,
		};
		
		static const COLORREF ColorBtn_White		= RGB(255, 255, 255);
		static const COLORREF ColorBtn_Black		= RGB(0, 0, 0);
		static const COLORREF ColorBtn_DarkGray		= RGB(128, 128, 128);
		static const COLORREF ColorBtn_Gray		= RGB(192, 192, 192);
		static const COLORREF ColorBtn_LightGray	= RGB(224, 224, 224);
};

#endif

#endif /* GENS_WIN32_BTNCOLOR_HPP */
