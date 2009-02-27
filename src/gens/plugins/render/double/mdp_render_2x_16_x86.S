/***************************************************************************
 * Gens: [MDP] 2x renderer. [16-bit color] (x86 asm version)               *
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

#if defined(__linux__) && defined(__ELF__)
	/* Mark the stack as non-executable on ELF. */
	.section .note.GNU-stack, "", %progbits
#endif

/* Symbol redefines for ELF. */
#if defined(__ELF__)
	#define _mdp_render_2x_16_x86		mdp_render_2x_16_x86
	#define _mdp_render_2x_16_x86_mmx	mdp_render_2x_16_x86_mmx
#endif

/* Function parameters. */
#define arg_destScreen	 8(%ebp)
#define arg_mdScreen	12(%ebp)
#define arg_destPitch	16(%ebp)
#define arg_srcPitch	20(%ebp)
#define arg_width	24(%ebp)
#define arg_height	28(%ebp)

.section .text

/***********************************************************************
 * void mdp_render_2x_16_x86(uint16_t *destScreen, uint16_t *mdScreen, *
 *                           int destPitch, int srcPitch,              *
 *                           int width, int height);                   *
 ***********************************************************************/
.globl _mdp_render_2x_16_x86
_mdp_render_2x_16_x86:
	
	/* Set up the frame pointer. */
	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebx	/* %ebx = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	addl	%ecx, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebx		/* %ebx = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$4, %ecx		/* Transfer 16 bytes per cycle. (8 16-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f

.balign 16
0: /* .Loop_Y */
1: /* .Loop_X1 */
			/* First two pixels. */
			movl	(%esi), %eax
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax, (%edi)
			movl	%edx, 4(%edi)
			
			/* Second two pixels. */
			movl	4(%esi), %eax
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax, 8(%edi)
			movl	%edx, 12(%edi)
			
			/* Next group of pixels. */
			addl	$8,  %esi
			addl	$16, %edi
			decl	%ecx
			jnz	1b
		
		movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		shll	$3, %ecx
		subl	%ecx, %esi		/* Go back to the beginning of the source line. */
		shrl	$3, %ecx
		jmp	2f

.balign 16

2: /* .Loop_X2 */
			/* First two pixels. */
			movl	(%esi), %eax
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax, (%edi)
			movl	%edx, 4(%edi)
			
			/* Second two pixels. */
			movl	4(%esi), %eax
			movl	%eax, %edx
			roll	$16, %eax
			xchgw	%dx, %ax
			movl	%eax, 8(%edi)
			movl	%edx, 12(%edi)
			
			/* Next group of pixels. */
			addl	$8,  %esi
			addl	$16, %edi
			decl	%ecx
			jnz	2b
		
		addl	arg_srcPitch, %esi	/* Add the source pitch difference. */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		movl	arg_width, %ecx		/* Reset the X conuter. */
		decl	arg_height		/* Decrement the Y counter. */
		jnz	0b
	
	/* Reset the frame pointer. */
	popal
	movl	%ebp, %esp
	popl	%ebp
	ret

.balign 64

/***************************************************************************
 * void mdp_render_2x_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen, *
 *                               int destPitch, int srcPitch,              *
 *                               int width, int height);                   *
 ***************************************************************************/
.globl _mdp_render_2x_16_x86_mmx
_mdp_render_2x_16_x86_mmx:
	
	/* Set up the frame pointer. */
	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	movl	arg_width, %ecx		/* %ecx = Number of pixels per line */
	movl	arg_destPitch, %ebx	/* %ebx = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	addl	%ecx, %ecx
	subl	%ecx, arg_srcPitch	/* arg_srcPitch = offset */
	addl	%ecx, %ecx		/* %ecx = Number of bytes per src line */
	subl	%ecx, %ebx		/* %ebx = Difference between dest pitch and src pitch */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	shrl	$6, %ecx		/* Transfer 64 bytes per cycle. (32 16-bit pixels) */
	movl	%ecx, arg_width		/* Initialize the X counter. */
	jmp	0f

.balign 16

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
			punpcklwd	%mm1, %mm1
			punpckhwd	%mm0, %mm0
			punpcklwd	%mm3, %mm3
			punpckhwd	%mm2, %mm2
			punpcklwd	%mm5, %mm5
			punpckhwd	%mm4, %mm4
			punpcklwd	%mm7, %mm7
			punpckhwd	%mm6, %mm6
			
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
			jnz	1b
		
		movl	arg_width, %ecx		/* Reset the X counter. */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		shll	$5, %ecx
		subl	%ecx, %esi		/* Go back to the beginning of the source line. */
		shrl	$5, %ecx
		jmp	2f

.balign 16

2: /* .Loop_X2 */
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
			punpcklwd	%mm1, %mm1
			punpckhwd	%mm0, %mm0
			punpcklwd	%mm3, %mm3
			punpckhwd	%mm2, %mm2
			punpcklwd	%mm5, %mm5
			punpckhwd	%mm4, %mm4
			punpcklwd	%mm7, %mm7
			punpckhwd	%mm6, %mm6
			
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
			jnz	2b
		
		addl	arg_srcPitch, %esi	/* Add the source pitch difference. */
		addl	%ebx, %edi		/* Add the destination pitch difference. */
		movl	arg_width, %ecx		/* Reset the X counter. */
		decl	arg_height		/* Decrement the Y counter. */
		jnz	0b
	
	/* Reset the frame pointer. */
	popal
	movl	%ebp, %esp
	popl	%ebp
	emms
	ret
