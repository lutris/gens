/***************************************************************************
 * Gens: Fast Blur function. (15/16-bit color, x86 asm version.)           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
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

/* MDP GNU `as` (x86) macros. */
#include "mdp/mdp_gnu_as_x86.inc"

#define MD_SCREEN_SIZE	(336*240)
#define MASK_DIV2_15	$0x3DEF
#define MASK_DIV2_16	$0x7BCF

/* External symbols. */
#ifndef __APPLE__
.extern SYM(MD_Screen)
.extern SYM(bppMD)
#endif

/** .data section **/
.data
	
	/* 64-bit masks used for the MMX fast blur function. */
	MASK_DIV2_15_MMX:	.long	0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16_MMX:	.long	0x7BCF7BCF, 0x7BCF7BCF
	
	SYMTYPE(MASK_DIV2_15_MMX,@object)
	SYMTYPE(MASK_DIV2_16_MMX,@object)
	
	SYMSIZE_DATA(MASK_DIV2_15_MMX, 8)
	SYMSIZE_DATA(MASK_DIV2_16_MMX, 8)

/** .text section **/
.text

/************************************************
 * void Fast_Blur_16_x86()                      *
 * 15/16-bit color Fast Blur function, non-MMX. *
 ************************************************/
.globl SYM(Fast_Blur_16_x86)
SYMTYPE(SYM(Fast_Blur_16_x86),@function)
SYM(Fast_Blur_16_x86):
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	
	/* Start at the beginning of the actual display data. */
	movl	$(SYM(MD_Screen) + (8*2)), %esi
	movl	$(MD_SCREEN_SIZE - 8), %ecx
	xorl	%ebx, %ebx
	
	/* Default to 16-bit color. */
	movw	MASK_DIV2_16, %dx
	cmpb	$15, SYM(bppMD)
	jne	0f
	
	/* 15-bit color is in use. */
	movw	MASK_DIV2_15, %dx
	jmp	0f
	
.p2align 4 /* 16-byte alignment */
0: /* .Loop */
		movw	(%esi), %ax	/* Get the current pixel. */
		shrw	$1, %ax		/* Reduce pixel brightness by 50%. */
		andw	%dx, %ax	/* Apply color mask. */
		addw	%ax, %bx	/* Blur the pixel with the previous pixel. */
		movw	%bx, -2(%esi)	/* Write the new pixel. */
		movw	%ax, %bx	/* Store the current pixel. */
		
		/* Next pixel. */
		addl	$2, %esi
		decl	%ecx
		jnz	0b
	
	/* Restore registers. */
	popl	%esi
	popl	%ebx
	popl	%edx
	popl	%ecx
	ret

SYMSIZE_FUNC(SYM(Fast_Blur_16_x86))

.p2align 6 /* 64-byte alignment */

/***************************************************************
 * void Fast_Blur_16_x86_mmx()                                 *
 * 15/16-bit color Fast Blur function, using MMX instructions. *
 ***************************************************************/
.globl SYM(Fast_Blur_16_x86_mmx)
SYMTYPE(SYM(Fast_Blur_16_x86_mmx),@function)
SYM(Fast_Blur_16_x86_mmx):
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%ebx
	pushl	%esi
	
	/* Start at the beginning of the actual display data. */
	movl	$(SYM(MD_Screen) + (8*2)), %esi
	movl	$((MD_SCREEN_SIZE - 8) / 4), %ecx
	
	/* Default to 16-bit color. */
	movq	(MASK_DIV2_16_MMX), %mm7
	cmpb	$15, SYM(bppMD)
	jne	0f
	
	/* 15-bit color is in use. */
	movq	(MASK_DIV2_15_MMX), %mm7
	jmp	0f

.p2align 4 /* 16-byte alignment */
0: /* .Loop */
		/* Get source pixels. */
		movq	 (%esi), %mm0
		movq	2(%esi), %mm1
		
		/* Blur source pixels. */
		psrlw	$1, %mm0
		psrlw	$1, %mm1
		pand	%mm7, %mm0
		pand	%mm7, %mm1
		paddw	%mm1, %mm0
		
		/* Put destination pixels. */
		movq	%mm0, (%esi)
		
		/* Next group of pixels. */
		addl	$8, %esi
		decl	%ecx
		jnz	0b
	
	/* Restore registers. */
	popl	%esi
	popl	%ebx
	popl	%ecx
	emms
	ret

SYMSIZE_FUNC(SYM(Fast_Blur_16_x86_mmx))
