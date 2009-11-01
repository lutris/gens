/***************************************************************************
 * Gens: [MDP] 1x renderer. [32-bit color] (x86 asm version)               *
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

/***********************************************************************
 * void mdp_render_1x_32_x86(uint32_t *destScreen, uint32_t *mdScreen, *
 *                           int destPitch, int srcPitch,              *
 *                           int width, int height);                   *
 ***********************************************************************/
.globl SYM(mdp_render_1x_32_x86)
SYMTYPE(SYM(mdp_render_1x_32_x86),@function)
SYM(mdp_render_1x_32_x86):
	
	/* Set up the frame pointer. */
	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	/* Save registers for later. */
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebx	/* %ebx = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	shll	$2, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	subl	%ecx, %ebx		/* %ebx = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$3, %ecx		/* Transfer 8 bytes per cycle. (2 32-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f /* .Loop_Y */

.p2align 4 /* 16-byte alignment */

0: /* .Loop_Y */
1: /* .Loop_X */
			/* Get source pixels. */
			movl	(%esi), %eax	/* First two pixels. */
			mov	4(%esi), %edx	/* Second two pixels. */
			addl	$8, %esi
			
			/* Put destination pixels. */
			movl	%eax, (%edi)
			movl	%edx, 4(%edi)
			addl	$8, %edi
			
			/* Next group of pixels. */
			decl	%ecx
			jnz	1b /* .Loop_X */
		
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
	ret

SYMSIZE_FUNC(SYM(mdp_render_1x_32_x86))

.p2align 6 /* 64-byte alignment */

/***************************************************************************
 * void mdp_render_1x_32_x86_mmx(uint32_t *destScreen, uint32_t *mdScreen, *
 *                               int destPitch, int srcPitch,              *
 *                               int width, int height);                   *
 ***************************************************************************/
.globl SYM(mdp_render_1x_32_x86_mmx)
SYMTYPE(SYM(mdp_render_1x_32_x86_mmx),@function)
SYM(mdp_render_1x_32_x86_mmx):
	
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
	shll	$2, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	subl	%ecx, %ebx		/* %ebx = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$6, %ecx		/* Transfer 64 bytes per cycle. (16 32-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f /* .Loop_Y */

.p2align 4 /* 16-byte alignment */

0: /* .Loop_Y */
1: /* .Loop_X */
			/* Get source pixels. */
			movq	  (%esi), %mm0
			movq	 8(%esi), %mm1
			movq	16(%esi), %mm2
			movq	24(%esi), %mm3
			movq	32(%esi), %mm4
			movq	40(%esi), %mm5
			movq	48(%esi), %mm6
			movq	56(%esi), %mm7
			addl	$64, %esi
			
			/* Put destination pixels. */
			movq	%mm0,   (%edi)
			movq	%mm1,  8(%edi)
			movq	%mm2, 16(%edi)
			movq	%mm3, 24(%edi)
			movq	%mm4, 32(%edi)
			movq	%mm5, 40(%edi)
			movq	%mm6, 48(%edi)
			movq	%mm7, 56(%edi)
			addl	$64, %edi
			
			/* Next group of pixels. */
			decl	%ecx
			jnz	1b /* .Loop_X */
		
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

SYMSIZE_FUNC(SYM(mdp_render_1x_32_x86_mmx))
