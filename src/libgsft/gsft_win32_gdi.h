/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_win32_gdi.h: Win32 convenience functions. (GDI Stuff)              *
 *                                                                         *
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

#ifndef __GSFT_WIN32_GDI_H
#define __GSFT_WIN32_GDI_H

#ifndef _WIN32
#error Do not include libgsft/gsft_win32_gdi.h on non-Win32 platforms!
#else

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "gsft_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * gsft_win32_gdi_get_message_font(): Get the message font.
 * @return Main font. (Must be deleted with DeleteFont() when finished.)
 */
DLL_LOCAL HFONT GSFT_FNCALL gsft_win32_gdi_get_message_font(void);

/**
 * gsft_win32_gdi_get_title_font(): Get the title font.
 * @return Main font. (Must be deleted with DeleteFont() when finished.)
 */
DLL_LOCAL HFONT GSFT_FNCALL gsft_win32_gdi_get_title_font(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */

#endif /* __MDP_WIN32_H */
