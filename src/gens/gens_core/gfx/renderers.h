/***************************************************************************
 * Gens: Renderer interface.                                               *
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

#ifndef GENS_RENDERERS_H
#define GENS_RENDERERS_H

#if 0

#ifdef __cplusplus
extern "C" {
#endif

// Function pointer calling conventions from SDL's begin_code.h
// TODO: Combine this with the #defines from compress.h
#ifndef GENS_FNCALL
#if defined(__WIN32__) && !defined(__GNUC__)
#define GENS_FNCALL __cdecl
#else
#ifdef __OS2__
#define GENS_FNCALL _System
#else
#define GENS_FNCALL
#endif
#endif
#endif

/**
 * BlitFn(): Blits the MD screen to the actual screen.
 * @param screen Screen buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
typedef void (GENS_FNCALL *BlitFn)(unsigned char *screen, int pitch, int x, int y, int offset);

/**
 * Renderer_t: Struct containing function pointers for renderers.
 */
struct Renderer_t
{
	char* name;
	BlitFn blit_16;		// Normal blit function, 16-bit color.
	BlitFn blit_16_mmx;	// MMX blit function, 16-bit color.
	BlitFn blit_32;		// Normal blit function, 32-bit color.
	BlitFn blit_32_mmx;	// MMX blit function, 32-bit color.
};

// Null-terminated array containing all of the available blitters.
extern const struct Renderer_t Renderers[];
extern const int Renderers_Count;

#ifdef __cplusplus
}
#endif

#endif

#endif
