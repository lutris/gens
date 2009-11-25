/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_shellapi.h: shellapi.h translation.                                *
 *                                                                         *
 * Copyright (c) 2009 by David Korth.                                      *
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

#ifndef GSFT_W32U_SHELLAPI_H
#define GSFT_W32U_SHELLAPI_H

#include "w32u.h"
#include <shellapi.h>

#ifdef __cplusplus
extern "C" {
#endif

MAKE_EXTFUNCPTR(DragQueryFileA);
#define pDragQueryFileU pDragQueryFileA

int WINAPI w32u_shellapi_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GSFT_W32U_SHELLAPI_H */
