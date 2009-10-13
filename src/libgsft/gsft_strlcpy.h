/****************************************************************************
 * libgsft: Common functions.                                               *
 * gsft_strlcpy.h: Improved strncpy() / strncat() functions from OpenBSD.   *
 *                                                                          *
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>            *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software for any    *
 * purpose with or without fee is hereby granted, provided that the above   *
 * copyright notice and this permission notice appear in all copies.        *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
 ****************************************************************************/

#ifndef __GSFT_STRLCPY_H
#define __GSFT_STRLCPY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <string.h>

// TODO: Add wrappers for _sntprintf() [Win32 TCHAR].

#ifdef __GNUC__
#define NONNULL __attribute__ ((nonnull))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_STRLCPY
// A BSD-style strlcpy() function was not found.
size_t gsft_strlcpy(char *dst, const char *src, size_t siz) NONNULL;
#define strlcpy(dst, src, siz) gsft_strlcpy(dst, src, siz)
#endif /* !HAVE_STRLCPY */

#ifndef HAVE_STRLCAT
// A BSD-style strlcat() function was not found.
size_t gsft_strlcat(char *dst, const char *src, size_t siz) NONNULL;
#define strlcat(dst, src, siz) gsft_strlcat(dst, src, siz)
#endif /* !HAVE_STRLCAT */

#ifdef __cplusplus
}
#endif

#endif /* __GSFT_STRLCPY_H */
