/***************************************************************************
 * Gens: (Win32) Zip File Selection Dialog.                                *
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

#ifndef GENS_WIN32_ZIP_SELECT_DIALOG_MISC_HPP
#define GENS_WIN32_ZIP_SELECT_DIALOG_MISC_HPP

#include "util/file/decompressor/decompressor.h"

#ifdef __cplusplus

#include <list>

#include "wndbase.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MDP decompression structs.
#include "mdp/mdp_z.h"

class ZipSelectDialog : public WndBase
{
	public:
		ZipSelectDialog();
		~ZipSelectDialog();
		
		mdp_z_entry_t* getFile(mdp_z_entry_t *file_list);
		
		void init(HWND hWndDlg);
	
	protected:
		mdp_z_entry_t* m_fileList;
};

#endif /* __cplusplus */

#endif /* GENS_WIN32_ZIP_SELECT_DIALOG_MISC_HPP */
