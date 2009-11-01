/***************************************************************************
 * MDP: 50% Scanline renderer. [16-bit color] (x86 asm version)            *
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
#define arg_mask	16+32(%esp)
#define arg_mode565	16+32(%esp)

/** .rodata section **/
RODATA()
	
	/* 64-bit masks used for the mmx version. */
	MASK_DIV2_15_MMX:	.long 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16_MMX:	.long 0x7BEF7BEF, 0x7BEF7BEF
	
	SYMTYPE(MASK_DIV2_15_MMX,@object)
	SYMTYPE(MASK_DIV2_16_MMX,@object)
	
	SYMSIZE_DATA(MASK_DIV2_15_MMX, 8)
	SYMSIZE_DATA(MASK_DIV2_16_MMX, 8)

/** .text section **/
.text

/********************************************************************************
 * void mdp_render_scanline_50_16_x86(uint16_t *destScreen, uint16_t *mdScreen, *
 *                                    int destPitch, int srcPitch,              *
 *                                    int width, int height, uint32_t mask);    *
 ********************************************************************************/
.globl SYM(mdp_render_scanline_50_16_x86)
SYMTYPE(SYM(mdp_render_scanline_50_16_x86),@function)
SYM(mdp_render_scanline_50_16_x86):
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%edx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebp	/* %ebp = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	addl	%ecx, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebp		/* %ebp = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$4, %ecx		/* Transfer 16 bytes per cycle. (8 16-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f /* .Loop_Y */

.p2align 4 /* 16-byte alignment */

0: /* .Loop_Y */
1: /* .Loop_X1 */
			/* First two pixels. */
			movl	(%esi), %eax
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax,  (%edi)
			movl	%edx, 4(%edi)
			
			/* Second two pixels. */
			movl	4(%esi), %eax
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax,  8(%edi)
			movl	%edx, 12(%edi)
			
			/* Next group of pixels. */
			addl	$8,  %esi
			addl	$16, %edi
			decl	%ecx
			jnz	1b /* .Loop_X1 */
		
		movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
		addl	%ebp, %edi		/* Add the destination pitch difference. */
		shll	$3, %ecx
		subl	%ecx, %esi		/* Go back to the beginning of the source line. */
		shrl	$3, %ecx
		jmp	2f /* .Loop_X2 */

.p2align 4 /* 16-byte alignment */

2: /* .Loop_X2 */
			/* First two pixels. */
			movl	(%esi), %eax
			shrl	$1, %eax	/* 50% scanline */
			andl	arg_mask, %eax	/* Mask off the MSB of each color component. */
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax,  (%edi)
			movl	%edx, 4(%edi)
			
			/* Second two pixels. */
			movl	4(%esi), %eax
			shrl	$1, %eax	/* 50% scanline */
			andl	arg_mask, %eax	/* Mask off the MSB of each color component. */
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax,  8(%edi)
			movl	%edx, 12(%edi)
			
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

SYMSIZE_FUNC(SYM(mdp_render_scanline_50_16_x86))

.p2align 6 /* 64-byte alignment */

/************************************************************************************
 * void mdp_render_scanline_50_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen, *
 *                                    int destPitch, int srcPitch,                  *
 *                                    int width, int height, int mode565);          *
 ************************************************************************************/
.globl SYM(mdp_render_scanline_50_16_x86_mmx)
SYMTYPE(SYM(mdp_render_scanline_50_16_x86_mmx),@function)
SYM(mdp_render_scanline_50_16_x86_mmx):
	
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
	addl	%ecx, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebp		/* %ebp = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$5, %ecx		/* Transfer 32 bytes per cycle. (16 16-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	
	/* Initialize the mask. */
	
	/* Default to 16-bit color. (Mode 565) */
	movq	MUNG_LOCAL(MASK_DIV2_16_MMX), %mm7
	testb	$1, arg_mode565
	jnz	0f /* .Loop_Y */
	
	/* 15-bit color is specified. (Mode 555) */
	movq	MUNG_LOCAL(MASK_DIV2_15_MMX), %mm7
	jmp	0f /* .Loop_Y */

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
			punpcklwd	%mm1, %mm1
			punpckhwd	%mm0, %mm0
			punpcklwd	%mm3, %mm3
			punpckhwd	%mm2, %mm2
			
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
			punpcklwd	%mm1, %mm1
			punpckhwd	%mm0, %mm0
			punpcklwd	%mm3, %mm3
			punpckhwd	%mm2, %mm2
			
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

SYMSIZE_FUNC(SYM(mdp_render_scanline_50_16_x86_mmx))
