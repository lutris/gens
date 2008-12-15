;
; Gens: [MDP] 2xSaI renderer. [16-bit color] (x86 asm version)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008 by David Korth
; 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian
;
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the
; Free Software Foundation; either version 2 of the License, or (at your
; option) any later version.
;
; This program is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
;

; Symbol redefines for ELF.
%ifdef __OBJ_ELF
	%define	_mdp_render_2xsai_16_x86_mmx	mdp_render_2xsai_16_x86_mmx
%endif

srcPtr		equ 28+8
srcPitch	equ 28+12
width		equ 28+16
dstOffset	equ 28+20
dstPitch	equ 28+24
dstSegment	equ 28+28

colorI		equ -2
colorE		equ 0
colorF		equ 2
colorJ		equ 4

colorG		equ -2
colorA		equ 0
colorB		equ 2
colorK		equ 4

colorH		equ -2
colorC		equ 0
colorD		equ 2
colorL		equ 4

colorM		equ -2
colorN		equ 0
colorO		equ 2
colorP		equ 4

; Position-independent code macros.
%include "pic.inc"

section .data align=64
	
	; Current color masks.
	; Default value is 16-bit.
	
	colorMask:		dd 0xF7DEF7DE, 0xF7DEF7DE
	lowPixelMask:		dd 0x08210821, 0x08210821
	
	qcolorMask:		dd 0xE79CE79C, 0xE79CE79C
	qlowpixelMask:		dd 0x18631863, 0x18631863
	
	; Previous Mode 555 setting.
	PrevMode555:		dd 0x00000000
	
; Read-only data on Win32 uses the section name ".rdata".
%ifdef __OBJ_WIN32
	%define .rodata .rdata
%endif

section .rodata align=64

	; 15-bit color masks.
	
	colorMask15:		dd 0x7BDE7BDE, 0x7BDE7BDE
	lowPixelMask15:		dd 0x04210421, 0x04210421
	
	qcolorMask15:		dd 0x739C739C, 0x739C739C
	qlowpixelMask15:	dd 0x0C630C63, 0x0C630C63
	
	; 16-bit color masks.
	
	colorMask16:		dd 0xF7DEF7DE, 0xF7DEF7DE
	lowPixelMask16:		dd 0x08210821, 0x08210821
	
	qcolorMask16:		dd 0xE79CE79C, 0xE79CE79C
	qlowpixelMask16:	dd 0x18631863, 0x18631863
	
	; Constants.
	ONE:			dd 0x00010001, 0x00010001
	
section .bss align=64
	
	Mask1:		resb 8
	Mask2:		resb 8
	ACPixel:	resb 8
	
section .text align=64

arg_destScreen	equ 28+8
arg_mdScreen	equ 28+12
arg_destPitch	equ 28+16
arg_srcPitch	equ 28+20
arg_width	equ 28+24
arg_height	equ 28+28
arg_mode555	equ 28+32


;************************************************************************
; void mdp_render_2xsai_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen,
;				   int destPitch, int srcPitch,
;				   int width, int height, int mode555);
global _mdp_render_2xsai_16_x86_mmx
_mdp_render_2xsai_16_x86_mmx:
	
	; Save registers.
	pushad
	
	; (PIC) Get the Global Offset Table.
	get_GOT
	
	; Check if the Mode 555 setting has changed.
	mov	al, byte [esp + arg_mode555]	; Mode 555 setting
	get_mov_localvar	ah, PrevMode555
	cmp	al, ah
	je	near .Parameters
	
	; Mode 555 setting has changed.
	put_mov_localvar	PrevMode555, al
	
	; Check if this is 15-bit color mode.
	test	al, 1
	jnz	short .Mode_555

.Mode_565:
	; 16-bit: Apply 16-bit color masks.
	get_movq_localvar	mm0, colorMask16
	get_movq_localvar	mm1, lowPixelMask16
	put_movq_localvar	colorMask, mm0
	put_movq_localvar	lowPixelMask, mm1
	get_movq_localvar	mm0, qcolorMask16
	get_movq_localvar	mm1, qlowpixelMask16
	put_movq_localvar	qcolorMask, mm0
	put_movq_localvar	qlowpixelMask, mm1
	jmp	short .Parameters

align 64

.Mode_555:
	; 15-bit: Apply 15-bit color masks.
	get_movq_localvar	mm0, colorMask15
	get_movq_localvar	mm1, lowPixelMask15
	put_movq_localvar	colorMask, mm0
	put_movq_localvar	lowPixelMask, mm1
	get_movq_localvar	mm0, qcolorMask15
	get_movq_localvar	mm1, qlowpixelMask15
	put_movq_localvar	qcolorMask, mm0
	put_movq_localvar	qlowpixelMask, mm1
	jmp	short .Parameters

align 64

.Parameters:
	; Miscellaneous parameters.
	mov	ecx, [esp + arg_height]		; ecx = Number of lines
	
	; Move parameters for _2xSaILine into registers.
	mov	edx, [esp + arg_width]		; edx = Width
	mov	ebp, [esp + arg_destPitch]	; ebp = Pitch of destination surface (bytes per line)
	mov	esi, [esp + arg_mdScreen]	; esi = Source
	mov	edi, [esp + arg_destScreen]	; edi = Destination
	mov	eax, [esp + arg_srcPitch]	; eax = Pitch of source surface (bytes per line)
	
	; Push parameters for _2xSaILine onto the stack.
	push	ebp	; 5th parameter == destination pitch
	push	edi	; 4th parameter == destination
	push	edx	; 3rd parameter == width
	push	eax	; 2nd parameter == source pitch
	push	esi	; 1st parameter == source
	
	; Go to the main loop.
	jmp	short .Loop

align 64

.Loop:
		mov	word [esi + edx * 2], 0	; clear clipping
		
		call	_2xSaILine		; Do one line
		
		add	esi, eax		; esi = *Src + 1 line
		lea	edi, [edi + ebp * 2]	; edi = *Dest + 2 lines
		mov	[esp], esi		; 1st Param = *Src
		mov	[esp + 12], edi		; 4th Param = *Dest
		
		dec	ecx
		jnz	short .Loop
	
	; Free the 5 parameters used for _2xSaILine.
	add	esp, byte 4 * 5
	
	; Restore registers.
	emms
	popad
	ret

align 64

;***********************************************************************************************
; void _2xSaILine(uint8 *srcPtr, uint32 srcPitch, uint32 width, uint8 *dstPtr, uint32 dstPitch);
_2xSaILine:
	
	; Save registers.
	pushad
	
	mov	edx, [esp + dstOffset]	; edx points to the screen
	
	mov	eax, [esp + srcPtr]	; eax points to colorA
	mov	ebp, [esp + srcPitch]
	mov	ecx, [esp + width]
	
	sub	eax, ebp		; eax now points to colorE
	
	pxor	mm0, mm0
	jmp	short .Loop

align 64

.Loop:
		push ecx
		
		;1	------------------------------------------
		
		;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorE) && (colorB == colorL)
		
		movq	mm0, [eax + ebp + colorA]	;mm0 and mm1 contain colorA
		movq	mm2, [eax + ebp + colorB]	;mm2 and mm3 contain colorB
		
		movq	mm1, mm0
		movq	mm3, mm2
		
		pcmpeqw	mm0, [eax + ebp + ebp + colorD]
		pcmpeqw	mm1, [eax + colorE]
		pcmpeqw	mm2, [eax + ebp + ebp + colorL]
		pcmpeqw	mm3, [eax + ebp + ebp + colorC]
		
		pand	mm0, mm1
		pxor	mm1, mm1
		pand	mm0, mm2
		pcmpeqw	mm3, mm1
		pand	mm0, mm3	;result in mm0
		
		;if ((colorA == colorC) && (colorB != colorE) && (colorA == colorF) && (colorB == colorJ)
		
		movq	mm4, [eax + ebp + colorA]	;mm4 and mm5 contain colorA
		movq	mm6, [eax + ebp + colorB]	;mm6 and mm7 contain colorB
		movq	mm5, mm4
		movq	mm7, mm6
		
		pcmpeqw	mm4, [eax + ebp + ebp + colorC]
		pcmpeqw	mm5, [eax + colorF]
		pcmpeqw	mm6, [eax + colorJ]
		pcmpeqw	mm7, [eax + colorE]
		
		pand	mm4, mm5
		pxor	mm5, mm5
		pand	mm4, mm6
		pcmpeqw	mm7, mm5
		pand	mm4, mm7	;result in mm4
		
		por	mm0, mm4	;combine the masks
		put_movq_localvar	Mask1, mm0
		
		;2	-------------------------------------------
		
		;if ((colorB == colorC) && (colorA != colorD) && (colorB == colorF) && (colorA == colorH)
		
		movq	mm0, [eax + ebp + colorB]	;mm0 and mm1 contain colorB
		movq	mm2, [eax + ebp + colorA]	;mm2 and mm3 contain colorA
		movq	mm1, mm0
		movq	mm3, mm2
		
		pcmpeqw	mm0, [eax + ebp + ebp + colorC]
		pcmpeqw	mm1, [eax + colorF]
		pcmpeqw	mm2, [eax + ebp + ebp + colorH]
		pcmpeqw	mm3, [eax + ebp + ebp + colorD]
		
		pand	mm0, mm1
		pxor	mm1, mm1
		pand	mm0, mm2
		pcmpeqw	mm3, mm1
		pand	mm0, mm3	;result in mm0
		
		;if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)
		
		movq	mm4, [eax + ebp + colorB]	;mm4 and mm5 contain colorB
		movq	mm6, [eax + ebp + colorA]	;mm6 and mm7 contain colorA
		movq	mm5, mm4
		movq	mm7, mm6
		
		pcmpeqw	mm4, [eax + ebp + ebp + colorD]
		pcmpeqw	mm5, [eax + colorE]
		pcmpeqw	mm6, [eax + colorI]
		pcmpeqw	mm7, [eax + colorF]
		
		pand	mm4, mm5
		pxor	mm5, mm5
		pand	mm4, mm6
		pcmpeqw	mm7, mm5
		pand	mm4, mm7	;result in mm4
		
		por	mm0, mm4	;combine the masks
		put_movq_localvar	Mask2, mm0
		
		;interpolate colorA and colorB
		
		movq	mm0, [eax + ebp + colorA]
		movq	mm1, [eax + ebp + colorB]
		
		movq	mm2, mm0
		movq	mm3, mm1
		
		pand_localvar	mm0, colorMask
		pand_localvar	mm1, colorMask
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand_localvar	mm3, lowPixelMask
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3	;mm0 contains the interpolated values
		
		;assemble the pixels
		
		movq	mm1, [eax + ebp + colorA]
		movq	mm2, [eax + ebp + colorB]
		
		get_movq_localvar	mm3, Mask1
		movq	mm5, mm1
		get_movq_localvar	mm4, Mask2
		movq	mm6, mm1
		
		pand	mm1, mm3
		por	mm3, mm4
		pxor	mm7, mm7
		pand	mm2, mm4
		
		pcmpeqw	mm3, mm7
		por	mm1, mm2
		pand	mm0, mm3
		
		por	mm0, mm1
		
		punpcklwd	mm5, mm0
		punpckhwd	mm6, mm0
		;movq	mm0, [eax + ebp + colorA + 8]	;Only the first pixel is needed
		
		movq	[edx], mm5
		movq	[edx + 8], mm6
		
		;3 Create the Nextline  -------------------
		
		;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorG) && (colorC == colorO)
		
		movq	mm0, [eax+ebp+colorA]			;mm0 and mm1 contain colorA
		movq	mm2, [eax+ebp+ebp+colorC]		;mm2 and mm3 contain colorC
		movq	mm1, mm0
		movq	mm3, mm2
		
		push	eax
		add	eax, ebp
		pcmpeqw	mm0, [eax + ebp + colorD]
		pcmpeqw	mm1, [eax + colorG]
		pcmpeqw	mm2, [eax + ebp + ebp + colorO]
		pcmpeqw	mm3, [eax + colorB]
		pop	eax
		
		pand	mm0, mm1
		pxor	mm1, mm1
		pand	mm0, mm2
		pcmpeqw	mm3, mm1
		pand	mm0, mm3	;result in mm0
		
		;if ((colorA == colorB) && (colorG != colorC) && (colorA == colorH) && (colorC == colorM)
		
		movq	mm4, [eax + ebp + colorA]		;mm4 and mm5 contain colorA
		movq	mm6, [eax + ebp + ebp + colorC]		;mm6 and mm7 contain colorC
		movq	mm5, mm4
		movq	mm7, mm6
		
		push	eax
		add	eax, ebp
		pcmpeqw	mm4, [eax + ebp + colorH]
		pcmpeqw	mm5, [eax + colorB]
		pcmpeqw	mm6, [eax + ebp + ebp + colorM]
		pcmpeqw	mm7, [eax + colorG]
		pop	eax
		
		pand	mm4, mm5
		pxor	mm5, mm5
		pand	mm4, mm6
		pcmpeqw	mm7, mm5
		pand	mm4, mm7	;result in mm4
		
		por	mm0, mm4	;combine the masks
		put_movq_localvar	Mask1, mm0
		
		;4  ----------------------------------------
		
		;if ((colorB == colorC) && (colorA != colorD) && (colorC == colorH) && (colorA == colorF)
		
		movq	mm0, [eax + ebp + ebp + colorC]		;mm0 and mm1 contain colorC
		movq	mm2, [eax + ebp + colorA]		;mm2 and mm3 contain colorA
		movq	mm1, mm0
		movq	mm3, mm2
		
		pcmpeqw	mm0, [eax + ebp + colorB]
		pcmpeqw	mm1, [eax + ebp + ebp + colorH]
		pcmpeqw	mm2, [eax + colorF]
		pcmpeqw	mm3, [eax + ebp + ebp + colorD]
		
		pand	mm0, mm1
		pxor	mm1, mm1
		pand	mm0, mm2
		pcmpeqw	mm3, mm1
		pand	mm0, mm3	;result in mm0
		
		;if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)
		
		movq	mm4, [eax + ebp + ebp + colorC]		;mm4 and mm5 contain colorC
		movq	mm6, [eax + ebp + colorA]		;mm6 and mm7 contain colorA
		movq	mm5, mm4
		movq	mm7, mm6
		
		pcmpeqw	mm4, [eax + ebp + ebp + colorD]
		pcmpeqw	mm5, [eax + ebp + colorG]
		pcmpeqw	mm6, [eax + colorI]
		pcmpeqw	mm7, [eax + ebp + ebp + colorH]
		
		pand	mm4, mm5
		pxor	mm5, mm5
		pand	mm4, mm6
		pcmpeqw	mm7, mm5
		pand	mm4, mm7	;result in mm4
		
		por	mm0, mm4	;combine the masks
		put_movq_localvar	Mask2, mm0
		
		;----------------------------------------------
		
		;interpolate colorA and colorC
		
		movq	mm0, [eax + ebp + colorA]
		movq	mm1, [eax + ebp + ebp + colorC]
		
		movq	mm2, mm0
		movq	mm3, mm1
		
		pand_localvar	mm0, colorMask
		pand_localvar	mm1, colorMask
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand_localvar	mm3, lowPixelMask
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3	;mm0 contains the interpolated values
		
		;-------------
		
		;assemble the pixels
		
		movq	mm1, [eax + ebp + colorA]
		movq	mm2, [eax + ebp + ebp + colorC]
		
		get_movq_localvar	mm3, Mask1
		get_movq_localvar	mm4, Mask2
		
		pand	mm1, mm3
		pand	mm2, mm4
		
		por	mm3, mm4
		pxor	mm7, mm7
		por	mm1, mm2
		
		pcmpeqw	mm3, mm7
		pand	mm0, mm3
		por	mm0, mm1
		put_movq_localvar	ACPixel, mm0
		
		;////////////////////////////////
		; Decide which "branch" to take
		;--------------------------------
		
		movq	mm0, [eax + ebp + colorA]
		movq	mm1, [eax + ebp + colorB]
		movq	mm6, mm0
		movq	mm7, mm1
		pcmpeqw	mm0, [eax + ebp + ebp + colorD]
		pcmpeqw	mm1, [eax + ebp + ebp + colorC]
		pcmpeqw	mm6, mm7
		
		movq	mm2, mm0
		movq	mm3, mm0
		
		pand	mm0, mm1	;colorA == colorD && colorB == colorC
		pxor	mm7, mm7
		
		pcmpeqw	mm2, mm7
		pand	mm6, mm0
		pand	mm2, mm1	;colorA != colorD && colorB == colorC
		
		pcmpeqw	mm1, mm7
		
		pand		mm1, mm3	;colorA == colorD && colorB != colorC
		pxor		mm0, mm6
		por		mm1, mm6
		movq		mm7, mm0
		put_movq_localvar	Mask2, mm2
		packsswb	mm7, mm7
		put_movq_localvar	Mask1, mm1
		
		movd	ecx, mm7
		test	ecx, ecx
		jz	near .SKIP_GUESS
		
		;-------------------------------------
		; Map of the pixels:           I|E F|J
		;                              G|A B|K
		;                              H|C D|L
		;                              M|N O|P
		
		movq	mm6, mm0
		movq	mm4, [eax + ebp + colorA]
		movq	mm5, [eax + ebp + colorB]
		pxor	mm7, mm7
		pand_localvar	mm6, ONE
		
		movq	mm0, [eax + colorE]
		movq	mm1, [eax + ebp + colorG]
		movq	mm2, mm0
		movq	mm3, mm1
		pcmpeqw	mm0, mm4
		pcmpeqw	mm1, mm4
		pcmpeqw	mm2, mm5
		pcmpeqw	mm3, mm5
		pand	mm0, mm6
		pand	mm1, mm6
		pand	mm2, mm6
		pand	mm3, mm6
		paddw	mm0, mm1
		paddw	mm2, mm3
		
		pxor	mm3, mm3
		pcmpgtw	mm0, mm6
		pcmpgtw	mm2, mm6
		pcmpeqw	mm0, mm3
		pcmpeqw	mm2, mm3
		pand	mm0, mm6
		pand	mm2, mm6
		paddw	mm7, mm0
		psubw	mm7, mm2
		
		movq	mm0, [eax + colorF]
		movq	mm1, [eax + ebp + colorK]
		movq	mm2, mm0
		movq	mm3, mm1
		pcmpeqw	mm0, mm4
		pcmpeqw	mm1, mm4
		pcmpeqw	mm2, mm5
		pcmpeqw	mm3, mm5
		pand	mm0, mm6
		pand	mm1, mm6
		pand	mm2, mm6
		pand	mm3, mm6
		paddw	mm0, mm1
		paddw	mm2, mm3
		
		pxor	mm3, mm3
		pcmpgtw	mm0, mm6
		pcmpgtw	mm2, mm6
		pcmpeqw	mm0, mm3
		pcmpeqw	mm2, mm3
		pand	mm0, mm6
		pand	mm2, mm6
		paddw	mm7, mm0
		psubw	mm7, mm2
		
		push	eax
		add	eax, ebp
		movq	mm0, [eax + ebp + colorH]
		movq	mm1, [eax + ebp + ebp + colorN]
		movq	mm2, mm0
		movq	mm3, mm1
		pcmpeqw	mm0, mm4
		pcmpeqw	mm1, mm4
		pcmpeqw	mm2, mm5
		pcmpeqw	mm3, mm5
		pand	mm0, mm6
		pand	mm1, mm6
		pand	mm2, mm6
		pand	mm3, mm6
		paddw	mm0, mm1
		paddw	mm2, mm3
		
		pxor	mm3, mm3
		pcmpgtw	mm0, mm6
		pcmpgtw	mm2, mm6
		pcmpeqw	mm0, mm3
		pcmpeqw	mm2, mm3
		pand	mm0, mm6
		pand	mm2, mm6
		paddw	mm7, mm0
		psubw	mm7, mm2
		
		movq	mm0, [eax + ebp + colorL]
		movq	mm1, [eax + ebp + ebp + colorO]
		movq	mm2, mm0
		movq	mm3, mm1
		pcmpeqw	mm0, mm4
		pcmpeqw	mm1, mm4
		pcmpeqw	mm2, mm5
		pcmpeqw	mm3, mm5
		pand	mm0, mm6
		pand	mm1, mm6
		pand	mm2, mm6
		pand	mm3, mm6
		paddw	mm0, mm1
		paddw	mm2, mm3
		
		pxor	mm3, mm3
		pcmpgtw	mm0, mm6
		pcmpgtw	mm2, mm6
		pcmpeqw	mm0, mm3
		pcmpeqw	mm2, mm3
		pand	mm0, mm6
		pand	mm2, mm6
		paddw	mm7, mm0
		psubw	mm7, mm2
		
		pop	eax
		movq	mm1, mm7
		pxor	mm0, mm0
		pcmpgtw	mm7, mm0
		pcmpgtw	mm0, mm1
		
		por_localvar	mm7, Mask1
		por_localvar	mm1, Mask2
		put_movq_localvar	Mask1, mm7
		put_movq_localvar	Mask2, mm1
		
	.SKIP_GUESS:
		
		;----------------------------
		;interpolate A, B, C and D
		
		movq	mm0, [eax + ebp + colorA]
		movq	mm1, [eax + ebp + colorB]
		movq	mm4, mm0
		movq	mm2, [eax + ebp + ebp + colorC]
		movq	mm5, mm1
		get_movq_localvar	mm3, qcolorMask
		movq	mm6, mm2
		get_movq_localvar	mm7, qlowpixelMask
		
		pand	mm0, mm3
		pand	mm1, mm3
		pand	mm2, mm3
		pand	mm3, [eax + ebp + ebp + colorD]
		
		psrlw	mm0, 2
		pand	mm4, mm7
		psrlw	mm1, 2
		pand	mm5, mm7
		psrlw	mm2, 2
		pand	mm6, mm7
		psrlw	mm3, 2
		pand	mm7, [eax + ebp + ebp + colorD]
		
		paddw	mm0, mm1
		paddw	mm2, mm3
		
		paddw	mm4, mm5
		paddw	mm6, mm7
		
		paddw	mm4, mm6
		paddw	mm0, mm2
		psrlw	mm4, 2
		pand_localvar	mm4, qlowpixelMask
		paddw	mm0, mm4	;mm0 contains the interpolated value of A, B, C and D
		
		;assemble the pixels
		
		get_movq_localvar	mm1, Mask1
		get_movq_localvar	mm2, Mask2
		movq	mm4, [eax + ebp + colorA]
		movq	mm5, [eax + ebp + colorB]
		pand	mm4, mm1
		pand	mm5, mm2
		
		pxor	mm7, mm7
		por	mm1, mm2
		por	mm4, mm5
		pcmpeqw	mm1, mm7
		pand	mm0, mm1
		por	mm4, mm0	;mm4 contains the diagonal pixels
		
		get_movq_localvar	mm0, ACPixel
		movq		mm1, mm0
		punpcklwd	mm0, mm4
		punpckhwd	mm1, mm4
		
		; Extra 8 bytes is caused by pushing 2 DWORDs onto the stack.
		; (%ebp can't be used due to PIC.)
		push	edx
		add	edx, [esp + 8 + dstPitch]
		
		movq	[edx], mm0
		movq	[edx + 8], mm1
		
		pop	edx
		
		add	edx, 16
		add	eax, 8
		
		pop	ecx
		sub	ecx, 4
		cmp	ecx, 0
		jg	near .Loop
	
	; Restore registers.
	popad
	ret
