/***************************************************************************
 * Gens: Old Print_Text() function.                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "print_text.hpp"
#include "misc.h"

// C includes.
#include <stdint.h>

#include "gens_core/vdp/vdp_rend.h"	// For MD_Screen[] / MD_Screen32[].
#include "gens_core/vdp/vdp_io.h"	// For VDP_Reg.H_Pix.
#include "emulator/g_main.hpp"		// For bppMD.


/**
 * Small_Police: Old Gens onscreen text font.
 */
static const uint32_t Small_Police[128-32][7] =
{
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// 32   
	{0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000000, 0x00000700, 0x00000000},	// 33	!
	{0x00000000, 0x00040002, 0x00050003, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// 34	"
	{0x00000000, 0x00040002, 0x00050403, 0x00060004, 0x00070605, 0x00080006, 0x00000000},	// 35	#
	{0x00000000, 0x00040300, 0x00000403, 0x00000500, 0x00070600, 0x00000706, 0x00000000},	// 36	$
	{0x00000000, 0x00000002, 0x00050000, 0x00000500, 0x00000005, 0x00080000, 0x00000000},	// 37	%
	{0x00000000, 0x00000300, 0x00050003, 0x00000500, 0x00070005, 0x00080700, 0x00000000},	// 38	&
	{0x00000000, 0x00000300, 0x00000400, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// 39	'
	{0x00000000, 0x00000300, 0x00000003, 0x00000004, 0x00000005, 0x00000700, 0x00000000},	// 40	(
	{0x00000000, 0x00000300, 0x00050000, 0x00060000, 0x00070000, 0x00000700, 0x00000000},	// 41	)
	{0x00000000, 0x00000000, 0x00050003, 0x00000500, 0x00070005, 0x00000000, 0x00000000},	// 42	*
	{0x00000000, 0x00000000, 0x00000400, 0x00060504, 0x00000600, 0x00000000, 0x00000000},	// 43	+
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000600, 0x00000700, 0x00000007},	// 44	,
	{0x00000000, 0x00000000, 0x00000000, 0x00060504, 0x00000000, 0x00000000, 0x00000000},	// 45	-
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000700, 0x00000000},	// 46	.
	{0x00030000, 0x00040000, 0x00000400, 0x00000500, 0x00000005, 0x00000006, 0x00000000},	// 47	/
	{0x00000000, 0x00000300, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000},	// 48	0
	{0x00000000, 0x00000300, 0x00000403, 0x00000500, 0x00000600, 0x00000700, 0x00000000},	// 49	1
	{0x00000000, 0x00000302, 0x00050000, 0x00000500, 0x00000005, 0x00080706, 0x00000000},	// 50	2
	{0x00000000, 0x00000302, 0x00050000, 0x00000504, 0x00070000, 0x00000706, 0x00000000},	// 51	3
	{0x00000000, 0x00000300, 0x00000003, 0x00060004, 0x00070605, 0x00080000, 0x00000000},	// 52	4
	{0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00070000, 0x00000706, 0x00000000},	// 53	5
	{0x00000000, 0x00000300, 0x00000003, 0x00000504, 0x00070005, 0x00000700, 0x00000000},	// 54	6
	{0x00000000, 0x00040302, 0x00050000, 0x00000500, 0x00000600, 0x00000700, 0x00000000},	// 55	7
	{0x00000000, 0x00000300, 0x00050003, 0x00000500, 0x00070005, 0x00000700, 0x00000000},	// 56	8
	{0x00000000, 0x00000300, 0x00050003, 0x00060500, 0x00070000, 0x00000700, 0x00000000},	// 57	9
	{0x00000000, 0x00000000, 0x00000400, 0x00000000, 0x00000000, 0x00000700, 0x00000000},	// 58	:
	{0x00000000, 0x00000000, 0x00000000, 0x00000500, 0x00000000, 0x00000700, 0x00000007},	// 59	;
	{0x00000000, 0x00040000, 0x00000400, 0x00000004, 0x00000600, 0x00080000, 0x00000000},	// 60	<
	{0x00000000, 0x00000000, 0x00050403, 0x00000000, 0x00070605, 0x00000000, 0x00000000},	// 61	=
	{0x00000000, 0x00000002, 0x00000400, 0x00060000, 0x00000600, 0x00000006, 0x00000000},	// 62	>
	{0x00000000, 0x00000300, 0x00050003, 0x00060000, 0x00000600, 0x00000000, 0x00000800},	// 63	?
	{0x00000000, 0x00000300, 0x00050400, 0x00060004, 0x00070600, 0x00000000, 0x00000000},	// 64	@
	{0x00000000, 0x00000300, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000},	// 65	A
	{0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00070005, 0x00000706, 0x00000000},	// 66	B
	{0x00000000, 0x00040300, 0x00000003, 0x00000004, 0x00000005, 0x00080700, 0x00000000},	// 67 	C
	{0x00000000, 0x00000302, 0x00050003, 0x00060004, 0x00070005, 0x00000706, 0x00000000},	// 68	D
	{0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00000005, 0x00080706, 0x00000000},	// 69	E
	{0x00000000, 0x00040302, 0x00000003, 0x00000504, 0x00000005, 0x00000006, 0x00000000},	// 70	F
	{0x00000000, 0x00040300, 0x00000003, 0x00060004, 0x00070005, 0x00080700, 0x00000000},	// 71	G
	{0x00000000, 0x00040002, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000},	// 72	H
	{0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000000},	// 73	I
	{0x00000000, 0x00040000, 0x00050000, 0x00060000, 0x00070005, 0x00000700, 0x00000000},	// 74	J
	{0x00000000, 0x00040002, 0x00050003, 0x00000504, 0x00070005, 0x00080006, 0x00000000},	// 75	K
	{0x00000000, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00080706, 0x00000000},	// 76	l
	{0x00000000, 0x00040002, 0x00050403, 0x00060004, 0x00070005, 0x00080006, 0x00000000},	// 77	M
	{0x00000000, 0x00000302, 0x00050003, 0x00060004, 0x00070005, 0x00080006, 0x00000000},	// 78	N
	{0x00000000, 0x00040302, 0x00050003, 0x00060004, 0x00070005, 0x00080706, 0x00000000},	// 79	O
	{0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00000005, 0x00000006, 0x00000000},	// 80	P
	{0x00000000, 0x00040302, 0x00050003, 0x00060004, 0x00070005, 0x00080706, 0x00090000},	// 81	Q
	{0x00000000, 0x00000302, 0x00050003, 0x00000504, 0x00070005, 0x00080006, 0x00000000},	// 82	R
	{0x00000000, 0x00040300, 0x00000003, 0x00000500, 0x00070000, 0x00000706, 0x00000000},	// 83 	S
	{0x00000000, 0x00040302, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000000},	// 84	T
	{0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000},	// 85	U
	{0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00000600, 0x00000700, 0x00000000},	// 86	V
	{0x00000000, 0x00040002, 0x00050003, 0x00060004, 0x00070605, 0x00080006, 0x00000000},	// 87	W
	{0x00000000, 0x00040002, 0x00050003, 0x00000500, 0x00070005, 0x00080006, 0x00000000},	// 88	X
	{0x00000000, 0x00040002, 0x00050003, 0x00000500, 0x00000600, 0x00000700, 0x00000000},	// 89	Y
	{0x00000000, 0x00040302, 0x00050000, 0x00000500, 0x00000005, 0x00080706, 0x00000000},	// 90	Z
	{0x00000000, 0x00040300, 0x00000400, 0x00000500, 0x00000600, 0x00080700, 0x00000000},	// 91	[
	{0x00000001, 0x00000002, 0x00000400, 0x00000500, 0x00070000, 0x00080000, 0x00000000},	// 92	'\'
	{0x00000000, 0x00000302, 0x00000400, 0x00000500, 0x00000600, 0x00000706, 0x00000000},	// 93	]
	{0x00000000, 0x00000300, 0x00050003, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// 94	^
	{0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00080706, 0x00000000},	// 95	_
	{0x00000000, 0x00000002, 0x00000400, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// 96	`
	{0x00000000, 0x00000000, 0x00050400, 0x00060004, 0x00070005, 0x00080700, 0x00000000},	// 97	a
	{0x00000000, 0x00000002, 0x00000003, 0x00000504, 0x00070005, 0x00000706, 0x00000000},	// 98	b
	{0x00000000, 0x00000000, 0x00050400, 0x00000004, 0x00000005, 0x00080700, 0x00000000},	// 99	c
	{0x00000000, 0x00040000, 0x00050000, 0x00060500, 0x00070005, 0x00080700, 0x00000000},	// 100	d
	{0x00000000, 0x00000000, 0x00050400, 0x00060504, 0x00000005, 0x00080700, 0x00000000},	// 101	e
	{0x00000000, 0x00040300, 0x00000003, 0x00000504, 0x00000005, 0x00000006, 0x00000000},	// 102	f
	{0x00000000, 0x00000000, 0x00050400, 0x00060004, 0x00070600, 0x00080000, 0x00000807},	// 103	g
	{0x00000000, 0x00000002, 0x00000003, 0x00000504, 0x00070005, 0x00080006, 0x00000000},	// 104	h
	{0x00000000, 0x00000300, 0x00000000, 0x00000500, 0x00000600, 0x00000700, 0x00000000},	// 105	i
	{0x00000000, 0x00000300, 0x00000000, 0x00000500, 0x00000600, 0x00000700, 0x00000007},	// 106	j
	{0x00000000, 0x00000002, 0x00000403, 0x00060004, 0x00000605, 0x00080006, 0x00000000},	// 107	k
	{0x00000000, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00080000, 0x00000000},	// 108	l
	{0x00000000, 0x00000000, 0x00050003, 0x00060504, 0x00070005, 0x00080006, 0x00000000},	// 109	m
	{0x00000000, 0x00000000, 0x00000403, 0x00060004, 0x00070005, 0x00080006, 0x00000000},	// 110	n
	{0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070005, 0x00000700, 0x00000000},	// 111	o
	{0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00000605, 0x00000006, 0x00000007},	// 112	p
	{0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070600, 0x00080000, 0x00090000},	// 113	q
	{0x00000000, 0x00000000, 0x00050003, 0x00000504, 0x00000005, 0x00000006, 0x00000000},	// 114	r
	{0x00000000, 0x00000000, 0x00050400, 0x00000004, 0x00070600, 0x00000706, 0x00000000},	// 115	s
	{0x00000000, 0x00000300, 0x00050400, 0x00000500, 0x00000600, 0x00080000, 0x00000000},	// 116	t
	{0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070005, 0x00080700, 0x00000000},	// 117	u
	{0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070005, 0x00000700, 0x00000000},	// 118	v
	{0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00070605, 0x00080006, 0x00000000},	// 119	w
	{0x00000000, 0x00000000, 0x00050003, 0x00000500, 0x00070005, 0x00080006, 0x00000000},	// 120	x
	{0x00000000, 0x00000000, 0x00050003, 0x00060004, 0x00000600, 0x00000700, 0x00000000},	// 121	y
	{0x00000000, 0x00000000, 0x00050403, 0x00000500, 0x00000005, 0x00080706, 0x00000000},	// 122	z
	{0x00000000, 0x00040300, 0x00000400, 0x00000504, 0x00000600, 0x00080700, 0x00000000},	// 123	{
	{0x00000000, 0x00000300, 0x00000400, 0x00000000, 0x00000600, 0x00000700, 0x00000000},	// 124	|
	{0x00000000, 0x00000302, 0x00000400, 0x00060500, 0x00000600, 0x00000706, 0x00000000},	// 125	}
	{0x00000000, 0x00000302, 0x00050000, 0x00000000, 0x00000000, 0x00000000, 0x00000000},	// 126	~
	{0x00000000, 0x00000000, 0x00000400, 0x00060004, 0x00070605, 0x00000000, 0x00000000},	// 127
};

// Palettes.
#define MAKE_COLOR15(r, g, b) ((uint16_t)(((r) << 10) | ((g) << 5) | (b)))
#define MAKE_COLOR16(r, g, b) ((uint16_t)(((r) << 11) | ((g) << 6) | (b)))
#define MAKE_COLOR32(r, g, b) ((uint32_t)(((r) << 16) | ((g) << 8) | (b)))

#define MAKE_PAL15(r, g, b) \
	{MAKE_COLOR15(r*16, g*16, b*16), MAKE_COLOR15(r*17, g*17, b*17), \
	 MAKE_COLOR15(r*18, g*18, b*18), MAKE_COLOR15(r*19, g*19, b*19), \
	 MAKE_COLOR15(r*20, g*20, b*20), MAKE_COLOR15(r*21, g*21, b*21), \
	 MAKE_COLOR15(r*22, g*22, b*22), MAKE_COLOR15(r*23, g*23, b*23), \
	 MAKE_COLOR15(r*24, g*24, b*24), MAKE_COLOR15(r*25, g*25, b*25), \
	 MAKE_COLOR15(r*26, g*26, b*26), MAKE_COLOR15(r*27, g*27, b*27), \
	 MAKE_COLOR15(r*28, g*28, b*28), MAKE_COLOR15(r*29, g*29, b*29), \
	 MAKE_COLOR15(r*30, g*30, b*30), MAKE_COLOR15(r*31, g*31, b*31)}

#define MAKE_PAL16(r, g, b) \
	{MAKE_COLOR16(r*16, g*16, b*16), MAKE_COLOR16(r*17, g*17, b*17), \
	 MAKE_COLOR16(r*18, g*18, b*18), MAKE_COLOR16(r*19, g*19, b*19), \
	 MAKE_COLOR16(r*20, g*20, b*20), MAKE_COLOR16(r*21, g*21, b*21), \
	 MAKE_COLOR16(r*22, g*22, b*22), MAKE_COLOR16(r*23, g*23, b*23), \
	 MAKE_COLOR16(r*24, g*24, b*24), MAKE_COLOR16(r*25, g*25, b*25), \
	 MAKE_COLOR16(r*26, g*26, b*26), MAKE_COLOR16(r*27, g*27, b*27), \
	 MAKE_COLOR16(r*28, g*28, b*28), MAKE_COLOR16(r*29, g*29, b*29), \
	 MAKE_COLOR16(r*30, g*30, b*30), MAKE_COLOR16(r*31, g*31, b*31)}

#define MAKE_PAL32(r, g, b) \
	{MAKE_COLOR32(r*128, g*128, b*128), MAKE_COLOR32(r*136, g*136, b*136), \
	 MAKE_COLOR32(r*144, g*144, b*144), MAKE_COLOR32(r*152, g*152, b*152), \
	 MAKE_COLOR32(r*160, g*160, b*160), MAKE_COLOR32(r*168, g*168, b*168), \
	 MAKE_COLOR32(r*176, g*176, b*176), MAKE_COLOR32(r*184, g*184, b*184), \
	 MAKE_COLOR32(r*192, g*192, b*192), MAKE_COLOR32(r*200, g*200, b*200), \
	 MAKE_COLOR32(r*208, g*208, b*208), MAKE_COLOR32(r*216, g*216, b*216), \
	 MAKE_COLOR32(r*224, g*224, b*224), MAKE_COLOR32(r*232, g*232, b*232), \
	 MAKE_COLOR32(r*240, g*240, b*240), MAKE_COLOR32(r*248, g*248, b*248)}

static const uint16_t Text_Palette_15[4][16] =
{
	MAKE_PAL15(1, 1, 1),	// White
	MAKE_PAL15(0, 0, 1),	// Blue
	MAKE_PAL15(0, 1, 0),	// Green
	MAKE_PAL15(1, 0, 0),	// Red
};

static const uint16_t Text_Palette_16[4][16] =
{
	MAKE_PAL16(1, 1, 1),	// White
	MAKE_PAL16(0, 0, 1),	// Blue
	MAKE_PAL16(0, 1, 0),	// Green
	MAKE_PAL16(1, 0, 0),	// Red
};

static const uint32_t Text_Palette_32[4][16] =
{
	MAKE_PAL32(1, 1, 1),	// White
	MAKE_PAL32(0, 0, 1),	// Blue
	MAKE_PAL32(0, 1, 0),	// Green
	MAKE_PAL32(1, 0, 0),	// Red
};


/**
 * T_Aff_Line_Letter(): Draw a letter to the screen.
 * @param pixel Pixel type.
 * @param line Line of the letter to draw. (0-7)
 * @param palette Color palette to use.
 * @param screen_ptr Pointer to position in the screen to draw to.
 * @param chr ((Character & 0x7F) - 0x20).
 */
template<typename pixel, unsigned int line>
static inline void T_Aff_Line_Letter(const pixel *palette, pixel *screen_ptr, int chr)
{
	uint32_t font_data = Small_Police[chr][line];
	screen_ptr += (line * 336);
	
	for (unsigned int i = 4; i != 0; i--, font_data >>= 8, screen_ptr++)
	{
		uint8_t font_byte = (font_data & 0xF);
		if (!font_byte)
			continue;
		
		*screen_ptr = palette[font_byte + 6];
	}
}


/**
 * T_Print_Text(): Print text directly to MD_Screen[] / MD_Screen32[].
 */
template<typename pixel, pixel *screen>
static inline void T_Print_Text(const char *str, int row_length, const pixel *palette,
				int Pos_X, int Pos_Y, int Style)
{
	// Get the screen pointer.
	pixel *screen_ptr = &screen[8 + (Pos_Y * 336) + Pos_X];
	
	// TODO: 2x text rendering and transparency rendering.
	if (Style & (SIZE_X2 | TRANS))
		return;
	
	// Process the string.
	int chr = *str++;
	do
	{
		chr = (chr & 0x7F) - 0x20;
		if (chr < 0)
			chr = 0;
		
		// Draw the letter.
		T_Aff_Line_Letter<pixel, 0>(palette, screen_ptr, chr);
		T_Aff_Line_Letter<pixel, 1>(palette, screen_ptr, chr);
		T_Aff_Line_Letter<pixel, 2>(palette, screen_ptr, chr);
		T_Aff_Line_Letter<pixel, 3>(palette, screen_ptr, chr);
		T_Aff_Line_Letter<pixel, 4>(palette, screen_ptr, chr);
		T_Aff_Line_Letter<pixel, 5>(palette, screen_ptr, chr);
		T_Aff_Line_Letter<pixel, 6>(palette, screen_ptr, chr);
		
		// Next character.
		screen_ptr += 4;
		row_length -= 4;
	} while (row_length > 0 && (chr = *str++));
}


/**
 * Print_Text(): Print text directly to MD_Screen[] / MD_Screen32[].
 * @param str Text to print.
 * @param Size Size of text.
 * @param Pos_X X position.
 * @param Pos_Y Y position.
 * @param Style Text style.
 */
void Print_Text(const char *str, int Size, int Pos_X, int Pos_Y, int Style)
{
	// Make sure we have a string.
	// NOTE: Don't check !str - if that happens, we should crash here.
	if (*str == 0x00)
		return;
	
	// Determine the starting position.
	Pos_Y += (240 - VDP_Num_Vis_Lines) / 2;		// Compensate for the screen border.
	if (Pos_Y + 7 >= VDP_Num_Vis_Lines)
	{
		// Offscreen. Don't draw anything.
		return;
	}
	
	// Row length.
	int row_length = 320;	// Assume 320px if emulation isn't running.
	if (Style & 0x01)
	{
		// Emulation is running. (TODO: That's what this is used for?!)
		row_length = VDP_Reg.H_Pix;
	}
	
	// Get the palette number.
	const unsigned int palette_num = (Style >> 1) & 0x03;
	
	// Render the text.
	if (bppMD == 15)
	{
		T_Print_Text<uint16_t, MD_Screen>(str, row_length,
							&Text_Palette_15[palette_num][0],
							Pos_X, Pos_Y, Style);
	}
	else if (bppMD == 16)
	{
		T_Print_Text<uint16_t, MD_Screen>(str, row_length,
							&Text_Palette_16[palette_num][0],
							Pos_X, Pos_Y, Style);
	}
	else //if (bppMD == 32)
	{
		T_Print_Text<uint32_t, MD_Screen32>(str, row_length,
							&Text_Palette_32[palette_num][0],
							Pos_X, Pos_Y, Style);
	}
}
