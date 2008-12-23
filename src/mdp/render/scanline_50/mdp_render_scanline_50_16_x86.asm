;
; Gens: [MDP] 50% Scanline renderer. [16-bit color] (x86 asm version)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008 by David Korth
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

%ifidn	__OUTPUT_FORMAT__, elf
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, elf32
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, elf64
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, win32
	%define	__OBJ_WIN32
	%define	.rodata	.rdata
%elifidn __OUTPUT_FORMAT__, win64
	%define	__OBJ_WIN64
	%define	.rodata	.rdata
%elifidn __OUTPUT_FORMAT__, macho
	%define	__OBJ_MACHO
%endif

%ifdef __OBJ_ELF
	; Mark the stack as non-executable on ELF.
	section .note.GNU-stack noalloc noexec nowrite progbits
%endif

arg_destScreen	equ 28+8
arg_mdScreen	equ 28+12
arg_destPitch	equ 28+16
arg_srcPitch	equ 28+20
arg_width	equ 28+24
arg_height	equ 28+28
arg_mask	equ 28+32
arg_mode555	equ 28+32

; Symbol redefines for ELF.
%ifdef __OBJ_ELF
	%define	_mdp_render_scanline_50_16_x86		mdp_render_scanline_50_16_x86
	%define	_mdp_render_scanline_50_16_x86_mmx	mdp_render_scanline_50_16_x86_mmx
%endif

; Position-independent code macros.
%include "pic.inc"

section .rodata align=64
	
	; 64-bit masks used for the mmx version.
	MASK_DIV2_15_MMX:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16_MMX:	dd 0x7BEF7BEF, 0x7BEF7BEF
	
section .text align=64
	
	align 64
	
	;************************************************************************
	; void mdp_render_scanline_50_16_x86(uint16_t *destScreen, uint16_t *mdScreen,
	;				     int destPitch, int srcPitch,
	;				     int width, int height, uint32_t mask);
	global _mdp_render_scanline_50_16_x86
	_mdp_render_scanline_50_16_x86:
		
		; Save registers.
		pushad
		
		mov	ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov	ebp, [esp + arg_destPitch]	; ebp = Pitch of destination surface (bytes per line)
		mov	esi, [esp + arg_mdScreen]	; esi = Source
		add	ecx, ecx
		sub	[esp + arg_srcPitch], ecx	; arg_srcPitch = offset
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	ebp, ecx			; ebp = Difference between dest pitch and src pitch
		mov	edi, [esp + arg_destScreen]	; edi = Destination
		shr	ecx, 4				; Transfer 16 bytes per cycle. (32 16-bit pixels)
		mov	[esp + arg_width], ecx		; Initialize the X counter.
		jmp	short .Loop_Y
	
	align 64
	
	.Loop_Y:
	.Loop_X1:
				; First two pixels.
				mov	eax, [esi]
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi], eax
				mov	[edi + 4], edx
				
				; Second two pixels.
				mov	eax, [esi + 4]
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi + 8], eax
				mov	[edi + 12], edx
				
				; Next group of pixels.
				add	esi, 8
				add	edi, 16
				dec	ecx
				jnz	short .Loop_X1
			
			mov	ecx, [esp + arg_width]	; ecx = Number of pixels per line
			add	edi, ebp		; Add the destination pitch difference.
			shl	ecx, 3
			sub	esi, ecx		; Go back to the beginning of the source line.
			shr	ecx, 3
			jmp	short .Loop_X2
	
	align 64
	
	.Loop_X2:
				; First two pixels.
				mov	eax, [esi]
				shr	eax, 1			; 50% scanline
				and	eax, [esp + arg_mask]	; Mask off the MSB of each color component.
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi], eax
				mov	[edi + 4], edx
				
				; Second two pixels.
				mov	eax, [esi + 4]
				shr	eax, 1			; 50% scanline
				and	eax, [esp + arg_mask]	; Mask off the MSB of each color component.
				mov	edx, eax
				rol	eax, 16
				xchg	ax, dx
				mov	[edi + 8], eax
				mov	[edi + 12], edx
				
				; Next group of pixels.
				add esi, 8
				add edi, 16
				dec ecx
				jnz short .Loop_X2
			
			add	esi, [esp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebp			; Add the destination pitch difference.
			mov	ecx, [esp + arg_width]		; Reset the X conuter.
			dec	dword [esp + arg_height]	; Decrement the Y counter.
			jnz	near .Loop_Y
		
		; Restore registers.
		popad
		ret
	
	align 64
	
	;************************************************************************
	; void mdp_render_scanline_50_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen,
	;					 int destPitch, int srcPitch,
	;					 int width, int height, int mode555);
	global _mdp_render_scanline_50_16_x86_mmx
	_mdp_render_scanline_50_16_x86_mmx:
		
		; Save registers.
		pushad
		
		; (PIC) Get the Global Offset Table.
		get_GOT
		
		mov	ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov	ebp, [esp + arg_destPitch]	; ebp = Pitch of destination surface (bytes per line)
		mov	esi, [esp + arg_mdScreen]	; esi = Source
		add	ecx, ecx
		sub	[esp + arg_srcPitch], ecx	; arg_srcPitch = offset
		add	ecx, ecx			; ecx = Number of bytes per line
		sub	ebp, ecx			; ebp = Difference between dest pitch and src pitch
		mov	edi, [esp + arg_destScreen]	; edi = Destination
		shr	ecx, 5				; Transfer 32 bytes per cycle. (64 16-bit pixels)
		mov	[esp + arg_width], ecx		; Initialize the X counter.
		
		; Initialize the mask.
		
		; Default to 16-bit color. (Mode 565)
		get_movq_localvar mm7, MASK_DIV2_16_MMX
		test	byte [esp + arg_mode555], 1
		jz	.Loop_Y
		
		; 15-bit color is specified. (Mode 555)
		get_movq_localvar mm7, MASK_DIV2_15_MMX
		jmp	short .Loop_Y
	
	align 64
	
	.Loop_Y:
	.Loop_X1:
				; Get source pixels.
				movq	mm0, [esi]
				movq	mm1, mm0
				movq	mm2, [esi + 8]
				movq	mm3, mm2
				add	esi, byte 16
				
				; Unpack source pixels.
				punpcklwd	mm1, mm1
				punpckhwd	mm0, mm0
				punpcklwd	mm3, mm3
				punpckhwd	mm2, mm2
				
				; Put destination pixels.
				movq	[edi], mm1
				movq	[edi + 8], mm0
				movq	[edi + 16], mm3
				movq	[edi + 24], mm2
				add	edi, byte 32
				
				; Next group of pixels.
				dec	ecx
				jnz	short .Loop_X1
			
			mov	ecx, [esp + arg_width]	; Reset the X counter.
			add	edi, ebp		; Add the destination pitch difference.
			shl	ecx, 4
			sub	esi, ecx		; Go back to the beginning of the source line.
			shr	ecx, 4
			jmp	short .Loop_X2
	
	align 64
	
	.Loop_X2:
				; Get source pixels.
				movq	mm0, [esi]
				movq	mm2, [esi + 8]
				add	esi, byte 16
				
				; 50% scanline.
				psrlq	mm0, 1
				psrlq	mm2, 1
				pand	mm0, mm7
				pand	mm2, mm7
				
				; Unpack pixels.
				movq	mm1, mm0
				movq	mm3, mm2
				punpcklwd	mm1, mm1
				punpckhwd	mm0, mm0
				punpcklwd	mm3, mm3
				punpckhwd	mm2, mm2
				
				; Put destination pixels.
				movq	[edi], mm1
				movq	[edi + 8], mm0
				movq	[edi + 16], mm3
				movq	[edi + 24], mm2
				add	edi, byte 32
				
				; Next group of pixels.
				dec	ecx
				jnz	short .Loop_X2
			
			add	esi, [esp + arg_srcPitch]	; Add the source pitch difference.
			add	edi, ebp			; Add the destination pitch difference.
			mov	ecx, [esp + arg_width]		; Reset the X counter.
			dec	dword [esp + arg_height]	; Decrement the Y counter.
			jnz	near .Loop_Y
		
		; Restore registers.
		emms
		popad
		ret
