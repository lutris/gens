/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_charset.h: Character set conversion functions.                     *
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

#ifndef GSFT_W32U_CHARSET_H
#define GSFT_W32U_CHARSET_H

#include "w32u.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Multibyte to Wide Character string functions. **/
wchar_t* WINAPI w32u_mbstowcs_alloc(const char *lpSrc, int cchMinSize, UINT cpFrom);
char* WINAPI w32u_wcstombs_alloc(const wchar_t *lpSrc, int cbMinSize, UINT cpTo);

/** Multibyte to Multibyte string functions. **/
char* WINAPI w32u_mbstombs_alloc(const char *lpSrc, int cbMinSize, UINT cpFrom, UINT cpTo);
int WINAPI w32u_mbstombs_copy(char *lpDest, const char *lpSrc, int cbDest, UINT cpFrom, UINT cpTo);
int WINAPI w32u_mbstombs_ip(char *lpBuf, int cbBuf, UINT cpFrom, UINT cpTo);

#define w32u_UTF8toUTF16(lpSrc)		w32u_mbstowcs_alloc((lpSrc), 0, CP_UTF8)
#define w32u_UTF16toUTF8(lpSrc)		w32u_wcstombs_alloc((lpSrc), 0, CP_UTF8)

#define w32u_UTF8toANSI(lpSrc)		w32u_mbstombs_alloc((lpSrc), 0, CP_UTF8, CP_ACP)
#define w32u_ANSItoUTF8(lpSrc)		w32u_mbstombs_alloc((lpSrc), 0, CP_ACP, CP_UTF8)

#define w32u_UTF8toANSI_sz(lpSrc, cbMinSize)		w32u_mbstombs_alloc((lpSrc), (cbMinSize), CP_UTF8, CP_ACP)

#define w32u_UTF8toANSI_copy(lpDest, lpSrc, cbDest)	w32u_mbstombs_copy((lpDest), (lpSrc), (cbDest), CP_UTF8, CP_ACP)
#define w32u_ANSItoUTF8_copy(lpDest, lpSrc, cbDest)	w32u_mbstombs_copy((lpDest), (lpSrc), (cbDest), CP_ACP, CP_UTF8)

#define w32u_UTF8toANSI_ip(lpBuf, cbBuf)		w32u_mbstombs_ip((lpBuf), (cbBuf), CP_UTF8, CP_ACP)
#define w32u_ANSItoUTF8_ip(lpBuf, cbBuf)		w32u_mbstombs_ip((lpBuf), (cbBuf), CP_ACP, CP_UTF8)

#if 0
wchar_t* WINAPI w32u_UTF8toUTF16(const char *mbs);
char* WINAPI w32u_UTF8toANSI(const char *mbs);
char* WINAPI w32u_UTF8toANSI_sz(const char *mbs, int cbSize);

int WINAPI w32u_UTF8toANSI_copy(char *lpDest, char *lpSrc, int cbDest);
int WINAPI w32u_ANSItoUTF8_copy(char *lpDest, char *lpSrc, int cbDest);
#endif

#ifdef __cplusplus
}
#endif

#endif /* GSFT_W32U_PRIV_H */
