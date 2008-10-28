/***************************************************************************
 * Gens: Window base class.                                                *
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

#ifndef GENS_UI_WNDBASE_HPP
#define GENS_UI_WNDBASE_HPP

#include <unistd.h>
#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus

class WndBase
{
	public:
		void setFocus(void);
		void setModal(void *parent);
	
	protected:
		WndBase() { }
		~WndBase() { }
		
		void *m_Window;
		void *m_AccelTable;
};

#endif /* __cplusplus */

#endif /* GENS_UI_WNDBASE_HPP */
