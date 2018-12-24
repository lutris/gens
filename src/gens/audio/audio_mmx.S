/***************************************************************************
 * Gens: Audio output functions, MMX-optimized.                            *
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
#define arg_left	 8(%ebp)
#define arg_right	12(%ebp)
#define arg_dest	16(%ebp)
#define arg_length	20(%ebp)

/** .text section **/
.text

/****************************************************************************************
 * void audio_write_sound_mono_x86_mmx(int *left, int *right, short *dest, int length); *
 ****************************************************************************************/
.globl SYM(audio_write_sound_mono_x86_mmx)
SYMTYPE(SYM(audio_write_sound_mono_x86_mmx),@function)
SYM(audio_write_sound_mono_x86_mmx):
	
	/* Set up the frame pointer. */
	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	/* Copy the function parameters to registers. */
	movl	arg_left,	%edi	/* Left */
	movl	arg_right,	%esi	/* Right */
	movl	arg_length,	%ecx	/* Length */
	movl	arg_dest,	%ebx	/* Dest */
	
	shrl	$1, %ecx
	jnc	4f /* .Double_Trans */

0: /* .Simple_Trans */
	movl	(%edi), %eax
	movl	$0, (%edi)
	addl	(%esi), %eax
	movl	$0, (%esi)
	
	cmpl	$0xFFFF, %eax
	jle	1f /* .lower_s */
	
	movw	$0x7FFF, (%ebx)
	jmp	3f /* .ok_s */

1: /* .lower_s */
	cmpl	$-0xFFFF, %eax
	jge	2f /* .greater_s */
	
	movw	$-0x7FFF, (%ebx)
	jmp	3f /* .ok_s */

2: /* .greater_s */
	shrl	$1, %eax
	movw	%ax, (%ebx)

3: /* .ok_s */
	addl	$4, %edi
	addl	$4, %esi
	addl	$2, %ebx

4: /* .Double_Trans */
	movl	$32, %eax
	pxor	%mm4, %mm4
	movd	%eax, %mm5
	movl	$1, %eax
	test	%ecx, %ecx
	movd	%eax, %mm6
	jnz	5f /* .Loop */
	jmp	6f /* .End */

.p2align 4 /* 16-byte alignment */

5: /* .Loop */
		movq		(%edi), %mm0		/* L2 | L1 */
		addl		$4, %ebx
		movq		%mm4, (%edi)
		movq		(%esi), %mm1		/* R2 | R1 */
		addl		$8, %edi
		movq		%mm4, (%esi)
		packssdw	%mm0, %mm0		/* 0 | 0 | L2 | L1 */
		packssdw	%mm1, %mm1		/* 0 | 0 | R2 | R1 */
		psraw		$1, %mm0
		psraw		$1, %mm1
		addl		$8, %esi
		paddw		%mm1, %mm0		/* 0 | 0 | R2 + L2 | R1 + L1 */
		
		decl	%ecx
		movd	%mm0, -4(%ebx)
		jnz	5b /* .Loop */

6: /* .End */
	/* Reset the frame pointer. */
	popal
	movl	%ebp, %esp
	popl	%ebp
	emms
	ret

SYMSIZE_FUNC(SYM(audio_write_sound_mono_x86_mmx))

.p2align 6 /* 64-byte alignment */

/******************************************************************************************
 * void audio_write_sound_stereo_x86_mmx(int *left, int *right, short *dest, int length); *
 ******************************************************************************************/
.globl SYM(audio_write_sound_stereo_x86_mmx)
SYMTYPE(SYM(audio_write_sound_stereo_x86_mmx),@function)
SYM(audio_write_sound_stereo_x86_mmx):
	
	/* Set up the frame pointer. */
	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	/* Copy the function parameters to registers. */
	movl	arg_left,	%edi	/* Left */
	movl	arg_right,	%esi	/* Right */
	movl	arg_length,	%ecx	/* Length */
	movl	arg_dest,	%ebx	/* Dest */
	
	shrl	$1, %ecx
	jnc	7f /* .Double_Trans */

0: /* .Simple_Trans */
	movl	(%edi), %eax
	cmpl	$0x7FFF, %eax
	movl	$0, (%edi)
	jle	1f /* .lower_s1 */
	
	movw	$0x7FFF, (%ebx)
	jmp	3f /* .right_s1 */

1: /* .lower_s1 */
	cmpl	$-0x7FFF, %eax
	jge	2f /* .greater_s1 */
	
	movw	$-0x7FFF, (%ebx)
	jmp	3f /* .right_s1 */

2: /* .greater_s1 */
	movw	%ax, (%ebx)

3: /* .right_s1 */
	movl	(%esi), %edx
	cmpl	$0x7FFF, %edx
	movl	$0, (%esi)
	jle	4f /* .lower_s2 */
	
	movw	$0x7FFF, 2(%ebx)
	jmp	6f /* .ok_s1 */

4: /* .lower_s2 */
	cmpl	$-0x7FFF, %edx
	jge	5f /* .greater_s2 */
	
	movw	$-0x7FFF, 2(%ebx)
	jmp	6f /* .ok_s1 */

5: /* .greater_s2 */
	movw	%dx, 2(%ebx)

6: /* .ok_s1 */
	addl	$4, %edi
	addl	$4, %esi
	addl	$4, %ebx

7: /* .Double_Trans */
	movl	$32, %eax
	pxor	%mm4, %mm4
	test	%ecx, %ecx
	movd	%eax, %mm5
	jnz	8f /* .Loop */
	jmp	9f /* .End */

.p2align 4 /* 16-byte alignment */

8: /* .Loop */
		movd		(%esi), %mm0		/* 0 | R1 */
		addl		$8, %edi
		movd		4(%esi), %mm1		/* 0 | R2 */
		psllq		%mm5, %mm0		/* R1 | 0 */
		movq		%mm4, (%esi)
		psllq		%mm5, %mm1		/* R2 | 0 */
		movd		-8(%edi), %mm2		/* 0 | L1 */
		addl		$8, %esi
		movd		-4(%edi), %mm3		/* 0 | L2 */
		addl		$8, %ebx
		paddd		%mm2, %mm0		/* R1 | L1 */
		paddd		%mm3, %mm1		/* R2 | L2 */
		movq		%mm4, -8(%edi)
		packssdw	%mm1, %mm0		/* R2 | L2 | R1 | L1 */
		
		decl	%ecx
		movq	%mm0, -8(%ebx)
		jnz	8b /* .Loop */

9: /* .End */
	/* Reset the frame pointer. */
	popal
	movl	%ebp, %esp
	popl	%ebp
	emms
	ret

SYMSIZE_FUNC(SYM(audio_write_sound_stereo_x86_mmx))
