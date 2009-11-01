/***************************************************************************
 * MDP: 50% Scanline renderer. [32-bit color] (x86 asm version)            *
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

/* Function parameters. */
#define arg_destScreen	16+8(%esp)
#define arg_mdScreen	16+12(%esp)
#define arg_destPitch	16+16(%esp)
#define arg_srcPitch	16+20(%esp)
#define arg_width	16+24(%esp)
#define arg_height	16+28(%esp)

#define MASK_DIV2_32	$0x7F7F7F7F

/** .rodata section **/
RODATA()
	
	/* 64-bit mask used for the MMX version. */
	MASK_DIV2_32_MMX:	.long 0x7F7F7F7F, 0x7F7F7F7F
	
	SYMTYPE(MASK_DIV2_32_MMX,@object)
	SYMSIZE_DATA(MASK_DIV2_32_MMX, 8)

/** .text section **/
.text

/********************************************************************************
 * void mdp_render_scanline_50_32_x86(uint16_t *destScreen, uint16_t *mdScreen, *
 *                                    int destPitch, int srcPitch,              *
 *                                    int width, int height);                   *
 ********************************************************************************/
.globl SYM(mdp_render_scanline_50_32_x86)
SYMTYPE(SYM(mdp_render_scanline_50_32_x86),@function)
SYM(mdp_render_scanline_50_32_x86):
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%edx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebp	/* %ebp = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	shll	$2, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebp		/* %ebp = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$4, %ecx		/* Transfer 16 bytes per cycle. (4 32-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f /* .Loop_Y */

.p2align 4 /* 16-byte alignment */

0: /* .Loop_Y */
1: /* .Loop_X1 */
			/* Get source pixels. */
			movl	 (%esi), %eax	/* First pixel. */
			movl	4(%esi), %edx	/* Second pixel. */
			
			/* Put destination pixels. */
			movl	%eax,   (%edi)
			movl	%eax,  4(%edi)
			movl	%edx,  8(%edi)
			movl	%edx, 12(%edi)
			
			/* Next group of pixels. */
			addl	$8, %esi
			addl	$16, %edi
			decl	%ecx
			jnz	1b
		
		movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
		addl	%ebp, %edi		/* Add the destination pitch difference. */
		shll	$3, %ecx
		subl	%ecx, %esi		/* Go back to the beginning of the source line. */
		shrl	$3, %ecx
		jmp	2f

.p2align 4 /* 16-byte alignment */

2: /* .Loop_X2 */
			/* First pixel. */
			movl	(%esi), %eax
			shrl	$1, %eax		/* 50% scanline */
			andl	MASK_DIV2_32, %eax	/* Mask off the MSB of each color component. */
			movl	%eax,  (%edi)
			movl	%eax, 4(%edi)
			
			/* Second pixel. */
			movl	4(%esi), %eax
			shrl	$1, %eax		/* 50% scanline */
			andl	MASK_DIV2_32, %eax	/* Mask off the MSB of each color component. */
			movl	%eax,  8(%edi)
			movl	%eax, 12(%edi)
			
			/* Next group of pixels. */
			addl	$8,  %esi
			addl	$16, %edi
			decl	%ecx
			jnz	2b /* .Loop_X2 */
		
		addl	arg_srcPitch, %esi	/* Add the source pitch difference. */
		addl	%ebp, %edi		/* Add the destination pitch difference. */
		movl	arg_width, %ecx		/* Reset the X conuter. */
		decl	arg_height		/* Decrement the Y counter. */
		jnz	0b /* .Loop_Y */
	
	/* Restore registers. */
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%edx
	popl	%ecx
	ret

SYMSIZE_FUNC(SYM(mdp_render_scanline_50_32_x86))

.p2align 6 /* 64-byte alignment */

/************************************************************************************
 * void mdp_render_scanline_50_32_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen, *
 *                                    int destPitch, int srcPitch,                  *
 *                                    int width, int height);                       *
 ************************************************************************************/
.globl SYM(mdp_render_scanline_50_32_x86_mmx)
SYMTYPE(SYM(mdp_render_scanline_50_32_x86_mmx),@function)
SYM(mdp_render_scanline_50_32_x86_mmx):
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%edx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	
	/* Initialize PIC. */
	INIT_PIC()
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebp	/* %ebp = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	shll	$2, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebp		/* %ebp = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$5, %ecx		/* Transfer 32 bytes per cycle. (8 32-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	movq	MUNG_LOCAL(MASK_DIV2_32_MMX), %mm7	/* Load the mask. */
	jmp	0f

.p2align 4 /* 16-byte alignment */

0: /* .Loop_Y */
1: /* .Loop_X1 */
			/* Get source pixels. */
			movq	 (%esi), %mm0
			movq	8(%esi), %mm2
			movq	%mm0, %mm1
			movq	%mm2, %mm3
			addl	$16, %esi
			
			/* Unpack source pixels. */
			punpckldq	%mm1, %mm1
			punpckhdq	%mm0, %mm0
			punpckldq	%mm3, %mm3
			punpckhdq	%mm2, %mm2
			
			/* Put destination pixels. */
			movq	%mm1,   (%edi)
			movq	%mm0,  8(%edi)
			movq	%mm3, 16(%edi)
			movq	%mm2, 24(%edi)
			addl	$32, %edi
			
			/* Next group of pixels. */
			decl	%ecx
			jnz	1b /* .Loop_X1 */
		
		movl	arg_width, %ecx		/* Reset the X counter. */
		addl	%ebp, %edi		/* Add the destination pitch difference. */
		shll	$4, %ecx
		subl	%ecx, %esi		/* Go back to the beginning of the source line. */
		shrl	$4, %ecx
		jmp	2f /* .Loop_X2 */

.p2align 4 /* 16-byte alignment */

2: /* .Loop_X2 */
			/* Get source pixels. */
			movq	 (%esi), %mm0
			movq	8(%esi), %mm2
			addl	$16, %esi
			
			/* 50% scanline. */
			psrlq	$1, %mm0
			psrlq	$1, %mm2
			pand	%mm7, %mm0
			pand	%mm7, %mm2
			
			/* Unpack pixels. */
			movq	%mm0, %mm1
			movq	%mm2, %mm3
			punpckldq	%mm1, %mm1
			punpckhdq	%mm0, %mm0
			punpckldq	%mm3, %mm3
			punpckhdq	%mm2, %mm2
			
			/* Put destination pixels. */
			movq	%mm1,   (%edi)
			movq	%mm0,  8(%edi)
			movq	%mm3, 16(%edi)
			movq	%mm2, 24(%edi)
			addl	$32, %edi
			
			/* Next group of pixels. */
			decl	%ecx
			jnz	2b /* .Loop_X2 */
		
		addl	arg_srcPitch, %esi	/* Add the source pitch difference. */
		addl	%ebp, %edi		/* Add the destination pitch difference. */
		movl	arg_width, %ecx		/* Reset the X counter. */
		decl	arg_height		/* Decrement the Y counter. */
		jnz	0b /* .Loop_Y */
	
	/* Restore registers. */
	emms
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%edx
	popl	%ecx
	ret

SYMSIZE_FUNC(SYM(mdp_render_scanline_50_32_x86_mmx))
