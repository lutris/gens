;
; Gens: [MDP] Super Eagle renderer. [16-bit color] (x86 asm version)
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
	%define	_mdp_render_super_eagle_16_x86_mmx	mdp_render_super_eagle_16_x86_mmx
%endif

srcPtr		equ 28+8
srcPitch	equ 28+12
width		equ 28+16
dstOffset	equ 28+20
dstPitch	equ 28+24
dstSegment	equ 28+28

colorB0		equ -2
colorB1		equ 0
colorB2		equ 2
colorB3		equ 4

color7		equ -2
color8		equ 0
color9		equ 2

color4		equ -2
color5		equ 0
color6		equ 2
colorS2		equ 4

color1		equ -2
color2		equ 0
color3		equ 2
colorS1		equ 4

colorA0		equ -2
colorA1		equ 0
colorA2		equ 2
colorA3		equ 4

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
	
	; for super2xSAI and supereagle
	I56Pixel:	resb 8
	I23Pixel:	resb 8
	I5556Pixel:	resb 8
	I2223Pixel:	resb 8
	I5666Pixel:	resb 8
	I2333Pixel:	resb 8
	Mask26:		resb 8
	Mask35:		resb 8
	Mask26b:	resb 8
	Mask35b:	resb 8
	product1a:	resb 8
	product1b:	resb 8
	product2a:	resb 8
	product2b:	resb 8
	final1a:	resb 8
	final1b:	resb 8
	final2a:	resb 8
	final2b:	resb 8
	
section .text align=64

arg_destScreen	equ 28+8
arg_mdScreen	equ 28+12
arg_destPitch	equ 28+16
arg_srcPitch	equ 28+20
arg_width	equ 28+24
arg_height	equ 28+28
arg_mode555	equ 28+32


;************************************************************************
; void mdp_render_super_eagle_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen,
;					 int destPitch, int srcPitch,
;					 int width, int height, int mode555);
global _mdp_render_super_eagle_16_x86_mmx
_mdp_render_super_eagle_16_x86_mmx:
	
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
	
	; Move parameters for SuperEagleLine into registers.
	mov	edx, [esp + arg_width]		; edx = Width
	mov	ebp, [esp + arg_destPitch]	; ebp = Pitch of destination surface (bytes per line)
	mov	esi, [esp + arg_mdScreen]	; esi = Source
	mov	edi, [esp + arg_destScreen]	; edi = Destination
	mov	eax, [esp + arg_srcPitch]	; eax = Pitch of source surface (bytes per line)
	
	; Push parameters for SuperEagleLine onto the stack.
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
		
		call	SuperEagleLine		; Do one line
		
		add	esi, eax		; esi = *Src + 1 line
		lea	edi, [edi + ebp * 2]    ; edi = *Dest + 2 lines
		mov	[esp], esi		; 1st Param = *Src
		mov	[esp + 12], edi		; 4th Param = *Dest
		
		dec	ecx
		jnz	short .Loop
	
	; Free the 5 parameters used for SuperEagleLine.
	add	esp, byte 4 * 5

.End:
	; Restore registers.
	emms
	popad
	ret


align 64
SuperEagleLine:
	
	; Save registers.
	pushad
	
	; Prepare the destination.
	mov	edx, [esp + dstOffset]	; edx points to the screen
	
	; Prepare the source
	; eax points to colorA
	mov	eax, [esp + srcPtr]
	mov	ebp, [esp + srcPitch]
	mov	ecx, [esp + width]
	
	sub	eax, ebp	; eax now points to colorB1
	
	pxor	mm0, mm0
	jmp	short .Loop

align 64

; Main Loop
.Loop:
		push	ecx
		
		movq	mm0, [eax+ebp+color5]
		movq	mm1, [eax+ebp+color6]
		movq	mm2, mm0
		movq	mm3, mm1
		movq	mm4, mm0
		movq	mm5, mm1
		
		pand	mm0, [colorMask]
		pand	mm1, [colorMask]
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand	mm3, [lowPixelMask]
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3		;mm0 contains the interpolated values
		movq	[I56Pixel], mm0
		movq	mm7, mm0
		
		;-------------------
		movq	mm0, mm7
		movq	mm1, mm4  ;5,5,5,6
		movq	mm2, mm0
		movq	mm3, mm1
		
		pand	mm0, [colorMask]
		pand	mm1, [colorMask]
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand	mm3, [lowPixelMask]
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3		;mm0 contains the interpolated values
		movq	[product1a], mm0
		;--------------------
		
		movq	mm0, mm7
		movq	mm1, mm5  ;6,6,6,5
		movq	mm2, mm0
		movq	mm3, mm1
		
		pand	mm0, [colorMask]
		pand	mm1, [colorMask]
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand	mm3, [lowPixelMask]
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3
		movq	[product1b], mm0
		
		;-------------------------
		;-------------------------
		movq	mm0, [eax + ebp + ebp + color2]
		movq	mm1, [eax + ebp + ebp + color3]
		movq	mm2, mm0
		movq	mm3, mm1
		movq	mm4, mm0
		movq	mm5, mm1
		
		pand	mm0, [colorMask]
		pand	mm1, [colorMask]
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand	mm3, [lowPixelMask]
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3
		movq	[I23Pixel], mm0
		movq	mm7, mm0
		
		;---------------------
		movq	mm0, mm7
		movq	mm1, mm4  ;2,2,2,3
		movq	mm2, mm0
		movq	mm3, mm1
		
		pand	mm0, [colorMask]
		pand	mm1, [colorMask]
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand	mm3, [lowPixelMask]
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3
		movq	[product2a], mm0
		
		;----------------------
		movq	mm0, mm7
		movq	mm1, mm5  ;3,3,3,2
		movq	mm2, mm0
		movq	mm3, mm1
		
		pand	mm0, [colorMask]
		pand	mm1, [colorMask]
		
		psrlw	mm0, 1
		psrlw	mm1, 1
		
		pand	mm3, [lowPixelMask]
		paddw	mm0, mm1
		
		pand	mm3, mm2
		paddw	mm0, mm3
		movq	[product2b], mm0
		
		
		;////////////////////////////////
		; Decide which "branch" to take
		;--------------------------------
		movq	mm4, [eax + ebp + color5]
		movq	mm5, [eax + ebp + color6]
		movq	mm6, [eax + ebp + ebp + color3]
		movq	mm7, [eax + ebp + ebp + color2]
		
		pxor	mm3, mm3
		movq	mm0, mm4
		movq	mm1, mm5
		
		pcmpeqw	mm0, mm6
		pcmpeqw	mm1, mm7
		pcmpeqw	mm1, mm3
		pand	mm0, mm1
		movq	[Mask35], mm0
		
		movq	mm0, [eax + ebp + ebp + colorS1]
		movq	mm1, [eax + ebp + color4]
		push	eax
		add	eax, ebp
		movq	mm2, [eax + ebp + ebp + colorA2]
		pop	eax
		movq	mm3, [eax + colorB1]
		pcmpeqw	mm0, mm4
		pcmpeqw	mm1, mm4
		pcmpeqw	mm2, mm4
		pcmpeqw	mm3, mm4
		pand	mm0, mm1
		pand	mm2, mm3
		por	mm0, mm2
		pand	mm0, [Mask35]
		movq	[Mask35b], mm0
		
		;-----------
		pxor	mm3, mm3
		movq	mm0, mm4
		movq	mm1, mm5
		
		pcmpeqw	mm0, mm6
		pcmpeqw	mm1, mm7
		pcmpeqw	mm0, mm3
		pand	mm0, mm1
		movq	[Mask26], mm0
		
		movq	mm0, [eax + ebp + ebp + color1]
		movq	mm1, [eax + ebp + colorS2]
		push	eax
		add	eax, ebp
		movq	mm2, [eax + ebp + ebp + colorA1]
		pop	eax
		movq	mm3, [eax + colorB2]
		pcmpeqw	mm0, mm5
		pcmpeqw	mm1, mm5
		pcmpeqw	mm2, mm5
		pcmpeqw	mm3, mm5
		pand	mm0, mm1
		pand	mm2, mm3
		por	mm0, mm2
		pand	mm0, [Mask26]
		movq	[Mask26b], mm0
		
		;--------------------
		movq	mm0, mm4
		movq	mm1, mm5
		movq	mm2, mm0
		
		pcmpeqw	mm2, mm1
		pcmpeqw	mm0, mm6
		pcmpeqw	mm1, mm7
		pand	mm0, mm1
		pand	mm2, mm0
		pxor	mm0, mm2
		movq	mm7, mm0
		
		;------------------
		packsswb	mm7, mm7
		movd		ecx, mm7
		test		ecx, ecx
		jz		near .SKIP_GUESS
		
		;---------------------------------------------
		; Map of the pixels:                    I|E F|J
		;                                       G|A B|K
		;                                       H|C D|L
		;                                       M|N O|P
		movq	mm6, mm0
		movq	mm4, [eax + ebp + color5]
		movq	mm5, [eax + ebp + color6]
		pxor	mm7, mm7
		pand	mm6, [ONE]
		
		movq	mm0, [eax + colorB1]
		movq	mm1, [eax + ebp + color4]
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
		
		movq	mm0, [eax + colorB2]
		movq	mm1, [eax + ebp + colorS2]
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
		movq	mm0, [eax + ebp + color1]
		movq	mm1, [eax + ebp + ebp + colorA1]
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
		
		movq	mm0, [eax + ebp + colorS1]
		movq	mm1, [eax + ebp + ebp + colorA2]
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
		
		por	mm7, [Mask35]
		por	mm1, [Mask26]
		movq	[Mask35], mm7
		movq	[Mask26], mm1
	
	.SKIP_GUESS:
		;Start the ASSEMBLY !!!
		
		movq	mm4, [Mask35]
		movq	mm5, [Mask26]
		movq	mm6, [Mask35b]
		movq	mm7, [Mask26b]
		
		movq	mm0, [eax + ebp + color5]
		movq	mm1, [eax + ebp + color6]
		movq	mm2, [eax + ebp + ebp + color2]
		movq	mm3, [eax + ebp + ebp + color3]
		pcmpeqw	mm0, mm2
		pcmpeqw	mm1, mm3
		movq	mm2, mm4
		movq	mm3, mm5
		por	mm0, mm1
		por	mm2, mm3
		pand	mm2, mm0
		pxor	mm0, mm2
		movq	mm3, mm0
		
		movq	mm2, mm0
		pxor	mm0, mm0
		por	mm2, mm4
		pxor	mm4, mm6
		por	mm2, mm5
		pxor	mm5, mm7
		pcmpeqw	mm2, mm0
		;----------------
		
		movq	mm0, [eax + ebp + color5]
		movq	mm1, mm3
		por	mm1, mm4
		por	mm1, mm6
		pand	mm0, mm1
		movq	mm1, mm5
		pand	mm1, [I56Pixel]
		por	mm0, mm1
		movq	mm1, mm7
		pand	mm1, [product1b]
		por	mm0, mm1
		movq	mm1, mm2
		pand	mm1, [product1a]
		por	mm0, mm1
		movq	[final1a], mm0
		
		movq	mm0, [eax + ebp + color6]
		movq	mm1, mm3
		por	mm1, mm5
		por	mm1, mm7
		pand	mm0, mm1
		movq	mm1, mm4
		pand	mm1, [I56Pixel]
		por	mm0, mm1
		movq	mm1, mm6
		pand	mm1, [product1a]
		por	mm0, mm1
		movq	mm1, mm2
		pand	mm1, [product1b]
		por	mm0, mm1
		movq	[final1b], mm0
		
		movq	mm0, [eax + ebp + ebp + color2]
		movq	mm1, mm3
		por	mm1, mm5
		por	mm1, mm7
		pand	mm0, mm1
		movq	mm1, mm4
		pand	mm1, [I23Pixel]
		por	mm0, mm1
		movq	mm1, mm6
		pand	mm1, [product2b]
		por	mm0, mm1
		movq	mm1, mm2
		pand	mm1, [product2a]
		por	mm0, mm1
		movq	[final2a], mm0
		
		movq	mm0, [eax + ebp + ebp + color3]
		movq	mm1, mm3
		por	mm1, mm4
		por	mm1, mm6
		pand	mm0, mm1
		movq	mm1, mm5
		pand	mm1, [I23Pixel]
		por	mm0, mm1
		movq	mm1, mm7
		pand	mm1, [product2a]
		por	mm0, mm1
		movq	mm1, mm2
		pand	mm1, [product2b]
		por	mm0, mm1
		movq	[final2b], mm0
		
		
		movq		mm0, [final1a]
		movq		mm2, [final1b]
		movq		mm1, mm0
		movq		mm4, [final2a]
		movq		mm6, [final2b]
		movq		mm5, mm4
		punpcklwd	mm0, mm2
		punpckhwd	mm1, mm2
		punpcklwd	mm4, mm6
		punpckhwd	mm5, mm6
		
		
		
		
		; Extra 8 bytes is caused by pushing 2 DWORDs onto the stack.
		; (%ebp can't be used due to PIC.)
		movq	[edx], mm0
		movq	[edx + 8], mm1
		push	edx
		add	edx, [esp + 8 + dstPitch]
		movq	[edx], mm4
		movq	[edx + 8], mm5
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
