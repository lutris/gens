;
; Gens: Fast Blur function. (15/16-bit color, x86 asm version.)
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

%include "nasmhead.inc"

section .data align=64

	extern MD_Screen
	
	; MD bpp
	%ifdef __OBJ_ELF
	%define _bppMD bppMD
	%endif
	extern _bppMD
	
	Mask:		dd 0x00000000, 0x00000000
	Mask_GG_15:	dd 0x03E003E0, 0x03E003E0
	Mask_RBRB_15:	dd 0x7C1F7C1F, 0x7C1F7C1F
	Mask_GG_16:	dd 0x07C007C0, 0x07C007C0
	Mask_RBRB_16	dd 0xF81FF81F, 0xF81FF81F

section .text align=64

	; void Fast_Blur_16_asm()
	; 15/16-bit color Fast Blur function, non-MMX.
	DECL Fast_Blur_16_x86
		
		push ebx
		push ecx
		push edx
		push edi
		push esi
		
		mov esi, MD_Screen
		mov ecx, 336 * 240
		xor edi, edi
		xor edx, edx
		
		; Check which color depth is in use.
		cmp byte [_bppMD], 16
		je short .Loop_565
		
	ALIGN32
	
	.Loop_555
			; 15-bit color
			mov ax, [esi]
			add esi, 2
			shr ax, 1
			mov bx, ax
			and ax, 0x3C0F
			and bx, 0x01E0
			add di, ax
			add dx, bx
			add dx, di
			dec ecx
			mov [esi - 2], dx
			mov di, ax
			mov dx, bx
			jnz short .Loop_555
		
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
	
	.Loop_565
			; 16-bit color
			mov ax, [esi]
			add esi, 2
			shr ax, 1
			mov bx, ax
			and ax, 0x780F
			and bx, 0x03E0
			add di, ax
			add dx, bx
			add dx, di
			dec ecx
			mov [esi - 2], dx
			mov di, ax
			mov dx, bx
			jnz .Loop_565
		
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
	
	
	ALIGN32
	
	; void Fast_Blur_16_asm_MMX()
	; 15/16-bit color Fast Blur function, using MMX instructions.
	DECL Fast_Blur_16_x86_mmx
		
		push ebx
		push ecx
		push edx
		push edi
		push esi
		
		mov esi, MD_Screen
		mov ecx, (336 * 240 * 2) / 8
		xor edi, edi
		xor edx, edx
		
		;If 15-bit, apply the 15-bit color masks.
		cmp byte [_bppMD], 15
		je short .Mask_555
		
		; 16-bit masks
		movq mm6, [Mask_RBRB_16]
		movq mm7, [Mask_GG_16]
		jmp short .MMX_Loop
		
	.Mask_555
		; 15-bit masks
		movq mm6, [Mask_RBRB_15]
		movq mm7, [Mask_GG_15]
	
	ALIGN32
	
	.MMX_Loop
			movq mm0, [esi]
			movq mm1, mm0
			movq mm2, [esi + 2]
			movq mm3, mm2
			pand mm0, mm6
			pand mm1, mm7
			pand mm2, mm6
			pand mm3, mm7
			psrlw mm0, 1
			psrlw mm1, 1
			psrlw mm2, 1
			psrlw mm3, 1
			paddusw mm0, mm2
			paddusw mm1, mm3
			pand mm0, mm6
			pand mm1, mm7
			add esi, 8
			paddw mm0, mm1
			dec ecx
			movq [esi - 8], mm0
			jnz short .MMX_Loop
		
		emms
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
