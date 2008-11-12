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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "renderers.h"
#include "blit.h"

// For some reason, NULL isn't defined here...
#ifndef NULL
#define NULL 0
#endif

// Null-terminated array containing all of the available blitters.
// Format: name, blit_16, blit_16_mmx, blit_32, blit_32_mmx

// Constant indicating the total number of renderers available.
const int Renderers_Count = 12;

#ifdef GENS_X86_ASM

// Includes x86 ASM versions, where available.
const struct Renderer_t Renderers[] =
{
	{"Normal", Blit1x_16_asm, Blit1x_16_asm_MMX, Blit1x_32, Blit1x_32},
	{"Double", Blit2x_16_asm, Blit2x_16_asm_MMX, Blit2x_32, Blit2x_32},
	{"Interpolated", Blit2x_Int_16_asm, Blit2x_Int_16_asm_MMX, NULL, NULL},
	{"Scanline", Blit2x_Scanline_16_asm, Blit2x_Scanline_16_asm_MMX, Blit2x_Scanline_32, Blit2x_Scanline_32},
	{"50% Scanline", NULL, Blit2x_Scanline_50_16_asm_MMX, NULL, NULL},
	{"25% Scanline", NULL, Blit2x_Scanline_25_16_asm_MMX, NULL, NULL},
	{"Interpolated Scanline", Blit2x_Scanline_Int_16_asm, Blit2x_Scanline_Int_16_asm_MMX, NULL, NULL},
	{"Interpolated 50% Scanline", NULL, Blit2x_Scanline_50_Int_16_asm_MMX, NULL, NULL},
	{"Interpolated 25% Scanline", NULL, Blit2x_Scanline_25_Int_16_asm_MMX, NULL, NULL},
	{"2xSAI (Kreed)", NULL, Blit_2xSAI_16_asm_MMX, NULL, NULL},
	{"Scale2x", Blit_Scale2x_16, Blit_Scale2x_16, Blit_Scale2x_32, Blit_Scale2x_32},
#ifndef GENS_OS_WIN32
	{"HQ2x", NULL, Blit_HQ2x_16_asm, NULL, NULL},
#endif /* GENS_OS_WIN32 */
	{NULL, NULL, NULL, NULL, NULL},
};

#else

// Don't include x86 ASM versions.
// Blitters that are only available as x86 ASM are not included.
const struct Renderer_t Renderers[] =
{
	{"Normal", Blit1x_16, Blit1x_16, Blit1x_32, Blit1x_32},
	{"Double", Blit2x_16, Blit2x_16, Blit2x_32, Blit2x_32},
	{"Interpolated", NULL, NULL, NULL, NULL},
	{"Scanline", Blit2x_Scanline_16, Blit2x_Scanline_16, Blit2x_Scanline_32, Blit2x_Scanline_32},
	{"50% Scanline", NULL, NULL, NULL, NULL},
	{"25% Scanline", NULL, NULL, NULL, NULL},
	{"Interpolated Scanline", NULL, NULL, NULL, NULL},
	{"Interpolated 50% Scanline", NULL, NULL, NULL, NULL},
	{"2xSAI (Kreed)", NULL, NULL, NULL, NULL},
	{"Scale2x", Blit_Scale2x_16, Blit_Scale2x_16, Blit_Scale2x_32, Blit_Scale2x_32},
	{"HQ2x", NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
};

#endif
