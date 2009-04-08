/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2003, 2004 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Gens/GS MDP specific code.
 * This is basically the mmx-only code, split out into a separate file.
 * It allows for the Scale?x plugin to be compiled with support for both
 * CPUs that support mmx and CPUs that don't support mmx.
 */

#include "scalebit_2x_mmx.h"
#include "scale2x.h"

#include <assert.h>
#include <stdlib.h>

#define SSDST(bits, num) (uint##bits##_t *)dst##num
#define SSSRC(bits, num) (const uint##bits##_t *)src##num

// Scale2x uses GNU inline assembler.
#if defined(__GNUC__) && defined(__i386__)

/**
 * Apply the Scale2x effect on a group of rows. Used internally.
 */
static inline void stage_scale2x_mmx(void* dst0, void* dst1,
				     const void* src0, const void* src1, const void* src2,
				     unsigned int pixel, unsigned int pixel_per_row)
{
	switch (pixel)
	{
		case 2:
			scale2x_16_mmx(SSDST(16,0), SSDST(16,1),
				       SSSRC(16,0), SSSRC(16,1), SSSRC(16,2),
					     pixel_per_row);
			break;
		
		case 4:
			scale2x_32_mmx(SSDST(32,0), SSDST(32,1),
				       SSSRC(32,0), SSSRC(32,1), SSSRC(32,2),
					     pixel_per_row);
			break;
	}
}

#define SCDST(i) (dst+(i)*dst_slice)
#define SCSRC(i) (src+(i)*src_slice)
#define SCMID(i) (mid[(i)])

/**
 * Apply the Scale2x effect on a bitmap.
 * The destination bitmap is filled with the scaled version of the source bitmap.
 * The source bitmap isn't modified.
 * The destination bitmap must be manually allocated before calling the function,
 * note that the resulting size is exactly 2x2 times the size of the source bitmap.
 * \param void_dst Pointer at the first pixel of the destination bitmap.
 * \param dst_slice Size in bytes of a destination bitmap row.
 * \param void_src Pointer at the first pixel of the source bitmap.
 * \param src_slice Size in bytes of a source bitmap row.
 * \param pixel Bytes per pixel of the source and destination bitmap.
 * \param width Horizontal size in pixels of the source bitmap.
 * \param height Vertical size in pixels of the source bitmap.
 */
void MDP_FNCALL scale2x_mmx(void* void_dst, unsigned int dst_slice,
			    const void* void_src, unsigned int src_slice,
			    unsigned int pixel, unsigned int width, unsigned int height)
{
	unsigned char* dst = (unsigned char*)void_dst;
	const unsigned char* src = (const unsigned char*)void_src;
	unsigned int count;
	
	assert(height >= 2);
	
	count = height;
	
	stage_scale2x_mmx(SCDST(0), SCDST(1), SCSRC(0), SCSRC(0), SCSRC(1), pixel, width);
	
	dst = SCDST(2);
	
	count -= 2;
	while (count)
	{
		stage_scale2x_mmx(SCDST(0), SCDST(1), SCSRC(0), SCSRC(1), SCSRC(2), pixel, width);
		
		dst = SCDST(2);
		src = SCSRC(1);
		
		--count;
	}
	
	stage_scale2x_mmx(SCDST(0), SCDST(1), SCSRC(0), SCSRC(1), SCSRC(1), pixel, width);
	
	scale2x_mmx_emms();
}

#endif /* defined(__GNUC__) && defined(__i386__) */
