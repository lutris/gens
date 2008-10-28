/***************************************************************************
 * Gens: About Window base class.                                          *
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

#ifndef GENS_UI_ABOUT_WINDOW_BASE_HPP
#define GENS_UI_ABOUT_WINDOW_BASE_HPP

#include "ui/wndbase.hpp"

#ifdef __cplusplus

class AboutWindow_Base : public WndBase
{
	protected:
		AboutWindow_Base() { }
		~AboutWindow_Base() { }
		
		void *m_imgGensLogo;
		unsigned short ax, bx, cx;
		virtual void updateIce(void) = 0;
		
		// Strings
		static const char* StrTitle;
		static const char* StrDescription;
		static const char* StrCopyright;
		
		static const unsigned char Data[];
		static const unsigned char DX[];
};

#endif /* __cplusplus */

#endif /* GENS_UI_ABOUT_WINDOW_BASE_HPP */
