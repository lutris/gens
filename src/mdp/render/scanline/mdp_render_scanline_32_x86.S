/***************************************************************************
 * MDP: Scanline renderer. [32-bit color] (x86 asm version)                *
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
#define arg_destScreen	 8(%ebp)
#define arg_mdScreen	12(%ebp)
#define arg_destPitch	16(%ebp)
#define arg_srcPitch	20(%ebp)
#define arg_width	24(%ebp)
#define arg_height	28(%ebp)

/** .text section **/
.text

/*****************************************************************************
 * void mdp_render_scanline_32_x86(uint32_t *destScreen, uint32_t *mdScreen, *
 *                           int destPitch, int srcPitch,                    *
 *                           int width, int height);                         *
 *****************************************************************************/
.globl SYM(mdp_render_scanline_32_x86)
SYMTYPE(SYM(mdp_render_scanline_32_x86),@function)
SYM(mdp_render_scanline_32_x86):
	
	/* Set up the frame pointer. */
	pushl	%ebp
	movl	%esp, %ebp
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebx	/* %ebx = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	shll	$2, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebx		/* %ebx = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$4, %ecx		/* Transfer 16 bytes per cycle. (4 32-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f /* .Loop_Y */

.p2align 4 /* 16-byte alignment */

0: /* .Loop_Y */
1: /* .Loop_X1 */
			/* Get source pixels. */
			movl	 (%esi), %eax
			movl	4(%esi), %edx
			
			/* Put destination pixels. */
			movl	%eax,   (%edi)
			movl	%eax,  4(%edi)
			movl	%edx,  8(%edi)
			movl	%edx, 12(%edi)
			
			/* Next group of pixels. */
			addl	$8,  %esi
			addl	$16, %edi
			decl	%ecx
			jnz	1b /* .Loop_X1 */
		
		movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		xorl	%edx, %edx		/* Scanlines are all black. */
		jmp	2f /* .Loop_X2 */

.p2align 4 /* 16-byte alignment */

2: /* .Loop_X2 */
			/* Erase the scanline. */
			movl	%edx,   (%edi)
			movl	%edx,  4(%edi)
			movl	%edx,  8(%edi)
			movl	%edx, 12(%edi)
			addl	$16, %edi
			
			/* Next group of pixels. */
			decl	%ecx
			jnz	2b /* .Loop_X2 */
		
		addl	arg_srcPitch, %esi	/* Add the source pitch difference. */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		movl	arg_width, %ecx		/* Reset the X conuter. */
		decl	arg_height		/* Decrement the Y counter. */
		jnz	0b /* .Loop_Y */
	
	/* Restore registers. */
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%edx
	popl	%ecx
	
	/* Reset the frame pointer. */
	movl	%ebp, %esp
	popl	%ebp
	ret

SYMSIZE_FUNC(SYM(mdp_render_scanline_32_x86))

.p2align 6 /* 64-byte alignment */

/*********************************************************************************
 * void mdp_render_scanline_32_x86_mmx(uint32_t *destScreen, uint32_t *mdScreen, *
 *                               int destPitch, int srcPitch,                    *
 *                               int width, int height);                         *
 *********************************************************************************/
.globl SYM(mdp_render_scanline_32_x86_mmx)
SYMTYPE(SYM(mdp_render_scanline_32_x86_mmx),@function)
SYM(mdp_render_scanline_32_x86_mmx):
	
	/* Set up the frame pointer. */
	pushl	%ebp
	movl	%esp, %ebp
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebx	/* %ebx = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	shll	$2, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebx		/* %ebx = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$6, %ecx		/* Transfer 64 bytes per cycle. (16 32-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f /* .Loop_Y */

.p2align 4 /* 16-byte alignment */

0: /* .Loop_Y */
1: /* .Loop_X1 */
			/* Get source pixels. */
			movq	  (%esi), %mm0
			movq	 8(%esi), %mm2
			movq	16(%esi), %mm4
			movq	24(%esi), %mm6
			
			/* Double the pixels. */
			movq	%mm0, %mm1
			movq	%mm2, %mm3
			movq	%mm4, %mm5
			movq	%mm6, %mm7
			addl	$32, %esi
			
			/* Unpack source pixels. */
			punpckldq	%mm1, %mm1
			punpckhdq	%mm0, %mm0
			punpckldq	%mm3, %mm3
			punpckhdq	%mm2, %mm2
			punpckldq	%mm5, %mm5
			punpckhdq	%mm4, %mm4
			punpckldq	%mm7, %mm7
			punpckhdq	%mm6, %mm6
			
			/* Put destination pixels. */
			movq	%mm1,   (%edi)
			movq	%mm0,  8(%edi)
			movq	%mm3, 16(%edi)
			movq	%mm2, 24(%edi)
			movq	%mm5, 32(%edi)
			movq	%mm4, 40(%edi)
			movq	%mm7, 48(%edi)
			movq	%mm6, 56(%edi)
			addl	$64, %edi
			
			/* Next group of pixels. */
			decl	%ecx
			jnz	1b /* .Loop_X1 */
		
		movl	arg_width, %ecx		/* Reset the X counter. */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		pxor	%mm0, %mm0		/* Scanlines are all black. */
		jmp	2f /* .Loop_X2 */

.p2align 4 /* 16-byte alignment */

2: /* .Loop_X2 */
			/* Erase the scanline. */
			movq	%mm0,   (%edi)
			movq	%mm0,  8(%edi)
			movq	%mm0, 16(%edi)
			movq	%mm0, 24(%edi)
			movq	%mm0, 32(%edi)
			movq	%mm0, 40(%edi)
			movq	%mm0, 48(%edi)
			movq	%mm0, 56(%edi)
			addl	$64, %edi
			
			/* Next group of pixels. */
			decl	%ecx
			jnz	2b /* .Loop_X2 */
		
		addl	arg_srcPitch, %esi	/* Add the source pitch difference. */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		movl	arg_width, %ecx		/* Reset the X counter. */
		decl	arg_height		/* Decrement the Y counter. */
		jnz	0b /* .Loop_Y */
	
	/* Restore registers. */
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%edx
	popl	%ecx
	
	/* Reset the frame pointer. */
	movl	%ebp, %esp
	popl	%ebp
	emms
	ret

SYMSIZE_FUNC(SYM(mdp_render_scanline_32_x86_mmx))
