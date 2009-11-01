/***************************************************************************
 * MDP: 2xSaI renderer. [16-bit color] (x86 asm version)                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian     *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
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

/* 2xSaI defines. */
#define colorI	-2
#define colorE	0
#define colorF	2
#define colorJ	4

#define colorG	-2
#define colorA	0
#define colorB	2
#define colorK	4

#define colorH	-2
#define colorC	0
#define colorD	2
#define colorL	4

#define colorM	-2
#define colorN	0
#define colorO	2
#define colorP	4

/** .data section **/
.data

/* Current color masks. */
/* Default value is 16-bit. */

colorMask:		.long	0xF7DEF7DE, 0xF7DEF7DE
lowPixelMask:		.long	0x08210821, 0x08210821

qcolorMask:		.long	0xE79CE79C, 0xE79CE79C
qlowpixelMask:		.long	0x18631863, 0x18631863

SYMTYPE(colorMask,@object)
SYMTYPE(lowPixelMask,@object)
SYMTYPE(qcolorMask,@object)
SYMTYPE(qlowpixelMask,@object)

SYMSIZE_DATA(colorMask, 8);
SYMSIZE_DATA(lowPixelMask, 8)
SYMSIZE_DATA(qcolorMask, 8)
SYMSIZE_DATA(qlowpixelMask, 8)

/* Previous Mode 565 state. */
PrevMode565:		.long	0x00000000
SYMTYPE(PrevMode565,@object)
SYMSIZE_DATA(PrevMode565, 8)

/** .rodata section **/
RODATA()

/* 15-bit color masks. */

colorMask15:		.long	0x7BDE7BDE, 0x7BDE7BDE
lowPixelMask15:		.long	0x04210421, 0x04210421

qcolorMask15:		.long	0x739C739C, 0x739C739C
qlowpixelMask15:	.long	0x0C630C63, 0x0C630C63

SYMTYPE(colorMask15,@object)
SYMTYPE(lowPixelMask15,@object)
SYMTYPE(qcolorMask15,@object)
SYMTYPE(qlowpixelMask15,@object)

SYMSIZE_DATA(colorMask15, 8);
SYMSIZE_DATA(lowPixelMask15, 8)
SYMSIZE_DATA(qcolorMask15, 8)
SYMSIZE_DATA(qlowpixelMask15, 8)

/* 16-bit color masks. */

colorMask16:		.long	0xF7DEF7DE, 0xF7DEF7DE
lowPixelMask16:		.long	0x08210821, 0x08210821

qcolorMask16:		.long	0xE79CE79C, 0xE79CE79C
qlowpixelMask16:	.long	0x18631863, 0x18631863

SYMTYPE(colorMask16,@object)
SYMTYPE(lowPixelMask16,@object)
SYMTYPE(qcolorMask16,@object)
SYMTYPE(qlowpixelMask16,@object)

SYMSIZE_DATA(colorMask16, 8);
SYMSIZE_DATA(lowPixelMask16, 8)
SYMSIZE_DATA(qcolorMask16, 8)
SYMSIZE_DATA(qlowpixelMask16, 8)

/* Constants. */
ONE:			.long	0x00010001, 0x00010001
SYMTYPE(ONE,@object)
SYMSIZE_DATA(ONE, 8)

/** Uninitialized data (.bss section) **/
.data

.lcomm	Mask1, 8
.lcomm	Mask2, 8
.lcomm	ACPixel, 8

/** .text section **/
.text

/* Function parameters. */
#define arg_destScreen	28+8(%esp)
#define arg_mdScreen	28+12(%esp)
#define arg_destPitch	28+16(%esp)
#define arg_srcPitch	28+20(%esp)
#define arg_width	28+24(%esp)
#define arg_height	28+28(%esp)
#define arg_mode565	28+32(%esp)

/******************************************************************************
 * void mdp_render_2xSaI_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen, *
 *                           int destPitch, int srcPitch,                     *
 *                           int width, int height, int mode565);             *
 ******************************************************************************/
.globl SYM(mdp_render_2xsai_16_x86_mmx)
SYMTYPE(SYM(mdp_render_2xsai_16_x86_mmx),@function)
SYM(mdp_render_2xsai_16_x86_mmx):
	
	/* Save registers for later. */
	pushal
	
	/* Initialize PIC. */
	INIT_PIC();
	
	/* Check if the Mode 565 state has changed. */
	movb	arg_mode565, %al
	movb	MUNG_LOCAL(PrevMode565), %ah
	cmpb	%ah, %al
	je	1f /* .Parameters */
	
	/* Mode 565 state has changed. */
	movb	%al, MUNG_LOCAL(PrevMode565);
	
	/* Check if this is 15-bit color mode. */
	testb	$1, %al
	jz	0f /* .Mode_555 */
	
/* .Mode_565: */
	/* 16-bit: Apply 16-bit color masks. */
	movq	MUNG_LOCAL(colorMask16), %mm0
	movq	MUNG_LOCAL(lowPixelMask16), %mm1
	movq	%mm0, MUNG_LOCAL(colorMask)
	movq	%mm1, MUNG_LOCAL(lowPixelMask)
	movq	MUNG_LOCAL(qcolorMask16), %mm0
	movq	MUNG_LOCAL(qlowpixelMask16), %mm1
	movq	%mm0, MUNG_LOCAL(qcolorMask)
	movq	%mm1, MUNG_LOCAL(qlowpixelMask)
	jmp	1f /* .Parameters */
	
.p2align 4 /* 16-byte alignment */

0: /* .Mode_555: */
	/* 15-bit: Apply 15-bit color masks. */
	movq	MUNG_LOCAL(colorMask15), %mm0
	movq	MUNG_LOCAL(lowPixelMask15), %mm1
	movq	%mm0, MUNG_LOCAL(colorMask)
	movq	%mm1, MUNG_LOCAL(lowPixelMask)
	movq	MUNG_LOCAL(qcolorMask15), %mm0
	movq	MUNG_LOCAL(qlowpixelMask15), %mm1
	movq	%mm0, MUNG_LOCAL(qcolorMask)
	movq	%mm1, MUNG_LOCAL(qlowpixelMask)
	jmp	1f /* .Parameters */

.p2align 4 /* 16-byte alignment */

1: /* .Parameters: */
	/* Miscellaneous parameters. */
	movl	arg_height, %ecx	/* %ecx = Number of lines */
	
	/* Move parameters for _2xSaILine into registers. */
	movl	arg_width, %edx		/* %edx = Width */
	movl	arg_destPitch, %ebp	/* %ebp = Pitch of destination surface (bytes per line) */
	movl	arg_mdScreen, %esi	/* %esi = Source */
	movl	arg_destScreen, %edi	/* %edi = Destination */
	movl	arg_srcPitch, %eax	/* %eax = Pitch of source surface (bytes per line) */
	
	/* Push parameters for _2xSaILine onto the stack. */
	pushl	%ebp	/* 5th parameter == destination pitch */
	pushl	%edi	/* 4th parameter == destination */
	pushl	%edx	/* 3rd parameter == width */
	pushl	%eax	/* 2nd parameter == source pitch */
	pushl	%esi	/* 1st parameter == source */
	
	/* Go to the main loop. */
	jmp	2f /* .Loop */

.p2align 4 /* 16-byte alignment */

2: /* .Loop: */
		movw	$0, (%esi,%edx,2)	/* Clear clipping. [esi + edx * 2] */
		
		call	_2xSaILine		/* Do one line. */
		
		addl	%eax, %esi		/* %esi = *Src + 1 line */
		lea	(%edi,%ebp,2), %edi	/* %edi = *Dest + 2 lines [edi + ebp * 2] */
		movl	%esi,   (%esp)		/* 1st parameter = *Src */
		movl	%edi, 12(%esp)		/* 2nd parameter = *Dest */
		
		decl	%ecx
		jnz	2b /* .Loop */
	
	/* Free the 5 parameters used for _2xSaILine. */
	addl	$(4*5), %esp
	
	/* Restore registers. */
	emms
	popal
	ret

SYMSIZE_FUNC(SYM(mdp_render_2xsai_16_x86_mmx))

/* _2xSaILine parameters. */
#define srcPtr		28+8(%esp)
#define srcPitch	28+12(%esp)
#define width		28+16(%esp)
#define dstOffset	28+20(%esp)
#define dstPitch	28+24(%esp)
#define dstSegment	28+28(%esp)

/**************************************************************************************************
 * void _2xSaILine(uint8 *srcPtr, uint32 srcPitch, uint32 width, uint8 *dstPtr, uint32 dstPitch); *
 **************************************************************************************************/
SYMTYPE(_2xSaILine,@function)
_2xSaILine:
	
	/* Save registers for later. */
	pushal
	
	/* Prepare the destination. */
	movl	dstOffset, %edx		/* %edx points to the screen. */
	
	/* Prepare the source. */
	movl	srcPtr, %eax		/* %eax points to colorA */
	movl	srcPitch, %ebp
	movl	width, %ecx
	
	subl	%ebp, %eax		/* eax now points to colorE */
	
	pxor	%mm0, %mm0
	jmp	0f /* .Loop */

.p2align 4	/* 16-byte alignment */

0: /* .Loop */
		pushl	%ecx
		
		/* 1	------------------------------------------ */
		/* if ((colorA == colorD) && (colorB != colorC) && (colorA == colorE) && (colorB == colorL) */
		
		movq	colorA(%eax,%ebp), %mm0		/* %mm0 and %mm1 contain colorA [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm2		/* %mm2 and %mm3 contain colorB [eax + ebp + colorB] */
		
		movq	%mm0, %mm1
		movq	%mm2, %mm3
		
		pcmpeqw	colorD(%eax,%ebp,2), %mm0	/* [eax + ebp + ebp + colorD] */
		pcmpeqw	colorE(%eax),        %mm1	/* [eax + colorE] */
		pcmpeqw	colorL(%eax,%ebp,2), %mm2	/* [eax + ebp + ebp + colorL] */
		pcmpeqw	colorC(%eax,%ebp,2), %mm3	/* [eax + ebp + ebp + colorC] */
		
		pand	%mm1, %mm0
		pxor	%mm1, %mm1
		pand	%mm2, %mm0
		pcmpeqw	%mm1, %mm3
		pand	%mm3, %mm0	/* result in %mm0 */
		
		/* if ((colorA == colorC) && (colorB != colorE) && (colorA == colorF) && (colorB == colorJ) */
		
		movq	colorA(%eax,%ebp), %mm4		/* %mm4 and %mm5 contain colorA [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm6		/* %mm6 and %mm7 contain colorB [eax + ebp + colorB] */
		movq	%mm4, %mm5
		movq	%mm6, %mm7
		
		pcmpeqw	colorC(%eax,%ebp,2), %mm4	/* [eax + ebp + ebp + colorC] */
		pcmpeqw	colorF(%eax),        %mm5	/* [eax + colorF] */
		pcmpeqw	colorJ(%eax),        %mm6	/* [eax + colorJ] */
		pcmpeqw	colorE(%eax),        %mm7	/* [eax + colorE] */
		
		pand	%mm5, %mm4
		pxor	%mm5, %mm5
		pand	%mm6, %mm4
		pcmpeqw	%mm5, %mm7
		pand	%mm7, %mm4	/* result in %mm4 */
		
		por	%mm4, %mm0	/* combine the masks */
		movq	%mm0, MUNG_LOCAL(Mask1)
		
		/* 2	------------------------------------------- */
		/* if ((colorB == colorC) && (colorA != colorD) && (colorB == colorF) && (colorA == colorH) */
		
		movq	colorB(%eax,%ebp), %mm0		/* %mm0 and %mm1 contain colorB [eax + ebp + colorB] */
		movq	colorA(%eax,%ebp), %mm2		/* %mm2 and %mm3 contain colorA [eax + ebp + colorA] */
		movq	%mm0, %mm1
		movq	%mm2, %mm3
		
		pcmpeqw	colorC(%eax,%ebp,2), %mm0	/* [eax + ebp + ebp + colorC] */
		pcmpeqw	colorF(%eax),        %mm1	/* [eax + colorF] */
		pcmpeqw	colorH(%eax,%ebp,2), %mm2	/* [eax + ebp + ebp + colorH] */
		pcmpeqw	colorD(%eax,%ebp,2), %mm3	/* [eax + ebp + ebp + colorD] */
		
		pand	%mm1, %mm0
		pxor	%mm1, %mm1
		pand	%mm2, %mm0
		pcmpeqw	%mm1, %mm3
		pand	%mm3, %mm0	/* result in %mm0 */
		
		/* if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI) */
		movq	colorB(%eax,%ebp), %mm4		/* %mm4 and %mm5 contain colorB [eax + ebp + colorB] */
		movq	colorA(%eax,%ebp), %mm6		/* %mm6 and %mm7 contain colorA [eax + ebp + colorA] */
		movq	%mm4, %mm5
		movq	%mm6, %mm7
		
		pcmpeqw	colorD(%eax,%ebp,2), %mm4	/* [eax + ebp + ebp + colorD] */
		pcmpeqw	colorE(%eax),        %mm5	/* [eax + colorE] */
		pcmpeqw	colorI(%eax),        %mm6	/* [eax + colorI] */
		pcmpeqw	colorF(%eax),        %mm7	/* [eax + colorF] */
		
		pand	%mm5, %mm4
		pxor	%mm5, %mm5
		pand	%mm6, %mm4
		pcmpeqw	%mm5, %mm7
		pand	%mm7, %mm4	/* result in mm4 */
		
		por	%mm4, %mm0	/* combine the masks */
		movq	%mm0, MUNG_LOCAL(Mask2)
		
		/* Interpolate colorA and colorB */
		
		movq	colorA(%eax,%ebp), %mm0		/* [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm1		/* [eax + ebp + colorB] */
		
		movq	%mm0, %mm2
		movq	%mm1, %mm3
		
		pand	MUNG_LOCAL(colorMask), %mm0
		pand	MUNG_LOCAL(colorMask), %mm1
		
		psrlw	$1, %mm0
		psrlw	$1, %mm1
		
		pand	MUNG_LOCAL(lowPixelMask), %mm3
		paddw	%mm1, %mm0
		
		pand	%mm2, %mm3
		paddw	%mm3, %mm0	/* %mm0 contains the interpolated values */
		
		/* Assemble the pixels. */
		
		movq	colorA(%eax,%ebp), %mm1		/* [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm2		/* [eax + ebp + colorB] */
		
		movq	MUNG_LOCAL(Mask1), %mm3
		movq	%mm1, %mm5
		movq	MUNG_LOCAL(Mask2), %mm4
		movq	%mm1, %mm6
		
		pand	%mm3, %mm1
		por	%mm4, %mm3
		pxor	%mm7, %mm7
		pand	%mm4, %mm2
		
		pcmpeqw	%mm7, %mm3
		por	%mm2, %mm1
		pand	%mm3, %mm0
		
		por	%mm1, %mm0
		
		punpcklwd	%mm0, %mm5
		punpckhwd	%mm0, %mm6
		/*movq	mm0, [eax + ebp + colorA + 8]	;Only the first pixel is needed*/
		
		movq	%mm5,  (%edx)
		movq	%mm6, 8(%edx)
		
		/* 3 Create the Nextline  ------------------- */
		/* if ((colorA == colorD) && (colorB != colorC) && (colorA == colorG) && (colorC == colorO) */
		
		movq	colorA(%eax,%ebp), %mm0		/* %mm0 and %mm1 contain colorA [eax + ebp + colorA] */
		movq	colorC(%eax,%ebp,2), %mm2	/* %mm2 and %mm3 contain colorC [eax + ebp + ebp + colorC] */
		movq	%mm0, %mm1
		movq	%mm2, %mm3
		
		pushl	%eax
		addl	%ebp, %eax
		pcmpeqw	colorD(%eax,%ebp),   %mm0	/* [eax + ebp + colorD] */
		pcmpeqw	colorG(%eax),        %mm1	/* [eax + colorG] */
		pcmpeqw	colorO(%eax,%ebp,2), %mm2	/* [eax + ebp + ebp + colorO] */
		pcmpeqw	colorB(%eax),        %mm3	/* [eax + colorB] */
		popl	%eax
		
		pand	%mm1, %mm0
		pxor	%mm1, %mm1
		pand	%mm2, %mm0
		pcmpeqw	%mm1, %mm3
		pand	%mm3, %mm0	/* result in %mm0 */
		
		/* if ((colorA == colorB) && (colorG != colorC) && (colorA == colorH) && (colorC == colorM) */
		
		movq	colorA(%eax,%ebp), %mm4		/* %mm4 and %mm5 contain colorA [eax + ebp + colorA] */
		movq	colorC(%eax,%ebp,2), %mm6	/* %mm6 and %mm7 contain colorC [eax + ebp + ebp + colorC] */
		movq	%mm4, %mm5
		movq	%mm6, %mm7
		
		pushl	%eax
		addl	%ebp, %eax
		pcmpeqw	colorH(%eax,%ebp),   %mm4	/* [eax + ebp + colorH] */
		pcmpeqw	colorB(%eax),        %mm5	/* [eax + colorB] */
		pcmpeqw	colorM(%eax,%ebp,2), %mm6	/* [eax + ebp + ebp + colorM] */
		pcmpeqw	colorG(%eax),        %mm7	/* [eax + colorG] */
		popl	%eax
		
		pand	%mm5, %mm4
		pxor	%mm5, %mm5
		pand	%mm6, %mm4
		pcmpeqw	%mm5, %mm7
		pand	%mm7, %mm4	/* result in %mm4 */
		
		por	%mm4, %mm0	/* combine the masks */
		movq	%mm0, MUNG_LOCAL(Mask1)
		
		/* 4  ---------------------------------------- */
		/* if ((colorB == colorC) && (colorA != colorD) && (colorC == colorH) && (colorA == colorF) */
		
		movq	colorC(%eax,%ebp,2), %mm0	/* %mm0 and %mm1 contain colorC [eax + ebp + ebp + colorC] */
		movq	colorA(%eax,%ebp), %mm2		/* %mm2 and %mm3 contain colorA [eax + ebp + colorA] */
		movq	%mm0, %mm1
		movq	%mm2, %mm3
		
		pcmpeqw	colorB(%eax,%ebp),   %mm0	/* [eax + ebp + colorB] */
		pcmpeqw	colorH(%eax,%ebp,2), %mm1	/* [eax + ebp + ebp + colorH] */
		pcmpeqw	colorF(%eax),        %mm2	/* [eax + colorF] */
		pcmpeqw	colorD(%eax,%ebp,2), %mm3	/* [eax + ebp + ebp + colorD] */
		
		pand	%mm1, %mm0
		pxor	%mm1, %mm1
		pand	%mm2, %mm0
		pcmpeqw	%mm1, %mm3
		pand	%mm3, %mm0	/* result in %mm0 */
		
		/* if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI) */
		
		movq	colorC(%eax,%ebp,2), %mm4	/* %mm4 and %mm5 contain colorC [eax + ebp + ebp + colorC] */
		movq	colorA(%eax,%ebp), %mm6 	/* %mm6 and %mm7 contain colorA [eax + ebp + colorA] */
		movq	%mm4, %mm5
		movq	%mm6, %mm7
		
		pcmpeqw	colorD(%eax,%ebp,2), %mm4	/* [eax + ebp + ebp + colorD] */
		pcmpeqw	colorG(%eax,%ebp),   %mm5	/* [eax + ebp + colorG] */
		pcmpeqw	colorI(%eax),        %mm6	/* [eax + colorI] */
		pcmpeqw	colorH(%eax,%ebp,2), %mm7	/* [eax + ebp + ebp + colorH] */
		
		pand	%mm5, %mm4
		pxor	%mm5, %mm5
		pand	%mm6, %mm4
		pcmpeqw	%mm5, %mm7
		pand	%mm7, %mm4	/* result in %mm4 */
		
		por	%mm4, %mm0	/* combine the masks */
		movq	%mm0, MUNG_LOCAL(Mask2)
		
		/* ---------------------------------------------- */
		
		/* interpolate colorA and colorC */
		
		movq	colorA(%eax,%ebp), %mm0		/* [eax + ebp + colorA] */
		movq	colorC(%eax,%ebp,2), %mm1	/* [eax + ebp + ebp + colorC] */
		
		movq	%mm0, %mm2
		movq	%mm1, %mm3
		
		pand	MUNG_LOCAL(colorMask), %mm0
		pand	MUNG_LOCAL(colorMask), %mm1
		
		psrlw	$1, %mm0
		psrlw	$1, %mm1
		
		pand	MUNG_LOCAL(lowPixelMask), %mm3
		paddw	%mm1, %mm0
		
		pand	%mm2, %mm3
		paddw	%mm3, %mm0	/* %mm0 contains the interpolated values */
		
		/* ------------- */
		
		/* assemble the pixels */
		
		movq	colorA(%eax,%ebp), %mm1		/* [eax + ebp + colorA] */
		movq	colorC(%eax,%ebp,2), %mm2	/* [eax + ebp + ebp + colorC] */
		
		movq	MUNG_LOCAL(Mask1), %mm3
		movq	MUNG_LOCAL(Mask2), %mm4
		
		pand	%mm3, %mm1
		pand	%mm4, %mm2
		
		por	%mm4, %mm3
		pxor	%mm7, %mm7
		por	%mm2, %mm1
		
		pcmpeqw	%mm7, %mm3
		pand	%mm3, %mm0
		por	%mm1, %mm0
		movq	%mm0, MUNG_LOCAL(ACPixel)
		
		/*********************************
		 * Decide which "branch" to take *
		 *********************************/
		
		movq	colorA(%eax,%ebp), %mm0		/* [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm1		/* [eax + ebp + colorB] */
		movq	%mm0, %mm6
		movq	%mm1, %mm7
		pcmpeqw	colorD(%eax,%ebp,2), %mm0	/* [eax + ebp + ebp + colorD] */
		pcmpeqw	colorC(%eax,%ebp,2), %mm1	/* [eax + ebp + ebp + colorC] */
		pcmpeqw	%mm7, %mm6
		
		movq	%mm0, %mm2
		movq	%mm0, %mm3
		
		pand	%mm1, %mm0	/* colorA == colorD && colorB == colorC */
		pxor	%mm7, %mm7
		
		pcmpeqw	%mm7, %mm2
		pand	%mm0, %mm6
		pand	%mm1, %mm2	/* colorA != colorD && colorB == colorC */
		
		pcmpeqw	%mm7, %mm1
		
		pand		%mm3, %mm1	/* colorA == colorD && colorB != colorC */
		pxor		%mm6, %mm0
		por		%mm6, %mm1
		movq		%mm0, %mm7
		movq		%mm2, MUNG_LOCAL(Mask2)
		packsswb	%mm7, %mm7
		movq		%mm1, MUNG_LOCAL(Mask1)
		
		movd	%mm7, %ecx
		testl	%ecx, %ecx
		jz	1f /* .SKIP_GUESS */
		
		/* -------------------------------------
		 * Map of the pixels:           I|E F|J
		 *                              G|A B|K
		 *                              H|C D|L
		 *                              M|N O|P
		 */
		
		movq	%mm0, %mm6
		movq	colorA(%eax,%ebp), %mm4		/* [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm5		/* [eax + ebp + colorB] */
		pxor	%mm7, %mm7
		pand	MUNG_LOCAL(ONE), %mm6
		
		movq	colorE(%eax), %mm0		/* [eax + colorE] */
		movq	colorG(%eax,%ebp), %mm1		/* [eax + ebp + colorG] */
		movq	%mm0, %mm2
		movq	%mm1, %mm3
		pcmpeqw	%mm4, %mm0
		pcmpeqw	%mm4, %mm1
		pcmpeqw	%mm5, %mm2
		pcmpeqw	%mm5, %mm3
		pand	%mm6, %mm0
		pand	%mm6, %mm1
		pand	%mm6, %mm2
		pand	%mm6, %mm3
		paddw	%mm1, %mm0
		paddw	%mm3, %mm2
		
		pxor	%mm3, %mm3
		pcmpgtw	%mm6, %mm0
		pcmpgtw	%mm6, %mm2
		pcmpeqw	%mm3, %mm0
		pcmpeqw	%mm3, %mm2
		pand	%mm6, %mm0
		pand	%mm6, %mm2
		paddw	%mm0, %mm7
		psubw	%mm2, %mm7
		
		movq	colorF(%eax), %mm0		/* [eax + colorF] */
		movq	colorK(%eax,%ebp), %mm1		/* [eax + ebp + colorK] */
		movq	%mm0, %mm2
		movq	%mm1, %mm3
		pcmpeqw	%mm4, %mm0
		pcmpeqw	%mm4, %mm1
		pcmpeqw	%mm5, %mm2
		pcmpeqw	%mm5, %mm3
		pand	%mm6, %mm0
		pand	%mm6, %mm1
		pand	%mm6, %mm2
		pand	%mm6, %mm3
		paddw	%mm1, %mm0
		paddw	%mm3, %mm2
		
		pxor	%mm3, %mm3
		pcmpgtw	%mm6, %mm0
		pcmpgtw	%mm6, %mm2
		pcmpeqw	%mm3, %mm0
		pcmpeqw	%mm3, %mm2
		pand	%mm6, %mm0
		pand	%mm6, %mm2
		paddw	%mm0, %mm7
		psubw	%mm2, %mm7
		
		pushl	%eax
		addl	%ebp, %eax
		movq	colorH(%eax,%ebp), %mm0		/* [eax + ebp + colorH] */
		movq	colorN(%eax,%ebp,2), %mm1	/* [eax + ebp + ebp + colorN] */
		movq	%mm0, %mm2
		movq	%mm1, %mm3
		pcmpeqw	%mm4, %mm0
		pcmpeqw	%mm4, %mm1
		pcmpeqw	%mm5, %mm2
		pcmpeqw	%mm5, %mm3
		pand	%mm6, %mm0
		pand	%mm6, %mm1
		pand	%mm6, %mm2
		pand	%mm6, %mm3
		paddw	%mm1, %mm0
		paddw	%mm3, %mm2
		
		pxor	%mm3, %mm3
		pcmpgtw	%mm6, %mm0
		pcmpgtw	%mm6, %mm2
		pcmpeqw	%mm3, %mm0
		pcmpeqw	%mm3, %mm2
		pand	%mm6, %mm0
		pand	%mm6, %mm2
		paddw	%mm0, %mm7
		psubw	%mm2, %mm7
		
		movq	colorL(%eax,%ebp), %mm0		/* [eax + ebp + colorL] */
		movq	colorO(%eax,%ebp,2), %mm1	/* [eax + ebp + ebp + colorO] */
		movq	%mm0, %mm2
		movq	%mm1, %mm3
		pcmpeqw	%mm4, %mm0
		pcmpeqw	%mm4, %mm1
		pcmpeqw	%mm5, %mm2
		pcmpeqw	%mm5, %mm3
		pand	%mm6, %mm0
		pand	%mm6, %mm1
		pand	%mm6, %mm2
		pand	%mm6, %mm3
		paddw	%mm1, %mm0
		paddw	%mm3, %mm2
		
		pxor	%mm3, %mm3
		pcmpgtw	%mm6, %mm0
		pcmpgtw	%mm6, %mm2
		pcmpeqw	%mm3, %mm0
		pcmpeqw	%mm3, %mm2
		pand	%mm6, %mm0
		pand	%mm6, %mm2
		paddw	%mm0, %mm7
		psubw	%mm2, %mm7
		
		popl	%eax
		movq	%mm7, %mm1
		pxor	%mm0, %mm0
		pcmpgtw	%mm0, %mm7
		pcmpgtw	%mm1, %mm0
		
		por	MUNG_LOCAL(Mask1), %mm7
		por	MUNG_LOCAL(Mask2), %mm1
		movq	%mm7, MUNG_LOCAL(Mask1)
		movq	%mm1, MUNG_LOCAL(Mask2)

1: /* .SKIP_GUESS */

		/*----------------------------*/
		/* interpolate A, B, C and D  */
		
		movq	colorA(%eax,%ebp), %mm0		/* [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm1		/* [eax + ebp + colorB] */
		movq	%mm0, %mm4
		movq	colorC(%eax,%ebp,2), %mm2	/* [eax + ebp + ebp + colorC] */
		movq	%mm1, %mm5
		movq	MUNG_LOCAL(qcolorMask), %mm3
		movq	%mm2, %mm6
		movq	MUNG_LOCAL(qlowpixelMask), %mm7
		
		pand	%mm3, %mm0
		pand	%mm3, %mm1
		pand	%mm3, %mm2
		pand	colorD(%eax,%ebp,2), %mm3	/* [eax + ebp + ebp + colorD] */
		
		psrlw	$2, %mm0
		pand	%mm7, %mm4
		psrlw	$2, %mm1
		pand	%mm7, %mm5
		psrlw	$2, %mm2
		pand	%mm7, %mm6
		psrlw	$2, %mm3
		pand	colorD(%eax,%ebp,2), %mm7	/* [eax + ebp + ebp + colorD] */
		
		paddw	%mm1, %mm0
		paddw	%mm3, %mm2
		
		paddw	%mm5, %mm4
		paddw	%mm7, %mm6
		
		paddw	%mm6, %mm4
		paddw	%mm2, %mm0
		psrlw	$2, %mm4
		pand	MUNG_LOCAL(qlowpixelMask), %mm4
		paddw	%mm4, %mm0	/* %mm0 contains the interpolated value of A, B, C and D */
		
		/* assemble the pixels */
		
		movq	MUNG_LOCAL(Mask1), %mm1
		movq	MUNG_LOCAL(Mask2), %mm2
		movq	colorA(%eax,%ebp), %mm4		/* [eax + ebp + colorA] */
		movq	colorB(%eax,%ebp), %mm5		/* [eax + ebp + colorB] */
		pand	%mm1, %mm4
		pand	%mm2, %mm5
		
		pxor	%mm7, %mm7
		por	%mm2, %mm1
		por	%mm5, %mm4
		pcmpeqw	%mm7, %mm1
		pand	%mm1, %mm0
		por	%mm0, %mm4	/* %mm4 contains the diagonal pixels */
		
		movq	MUNG_LOCAL(ACPixel), %mm0
		movq		%mm0, %mm1
		punpcklwd	%mm4, %mm0
		punpckhwd	%mm4, %mm1
		
		/* Extra 8 bytes is caused by pushing 2 DWORDs onto the stack. */
		/* (%ebp can't be used due to PIC.) */
		pushl	%edx
		addl	8+dstPitch, %edx		/* [esp + 8 + dstPitch] */
		
		movq	%mm0,  (%edx)
		movq	%mm1, 8(%edx)
		
		popl	%edx
		
		addl	$16, %edx
		addl	$8,  %eax
		
		popl	%ecx
		subl	$4, %ecx
		cmpl	$0, %ecx
		jg	0b /* .Loop */
	
	/* Restore registers. */
	popal
	ret

SYMSIZE_FUNC(_2xSaILine);
