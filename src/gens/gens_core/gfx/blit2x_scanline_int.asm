;
; Gens: 2x interpolated scanline renderer. (x86 ASM version)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
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
	extern Have_MMX
	extern bpp

	MASK_DIV2_15:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16:	dd 0x7BEF7BEF, 0x7BEF7BEF

section .text align=64

	ALIGN64
	
	;*********************************************************************************
	; void Blit2x_Scanline_Int_16_asm(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_Int_16_asm

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per line
		mov ebx, [esp + 28]				; ebx = Pitch of destination surface (bytes per line)
		lea esi, [MD_Screen + 8 * 2]			; esi = Source
		lea ecx, [ecx * 4]				; ecx = Number of bytes per line
		sub ebx, ecx					; ebx = Difference between dest pitch and src pitch
		shr ecx, 2					; Transfer 4 bytes per cycle. (8 16-bit pixels)
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X1:
;				mov eax, [esi]
;				add esi, byte 2
;				shr eax, 1
;				mov edx, eax
;				and eax, 0x03E0780F
;				and edx, 0x780F03E0
;				rol eax, 16
;				add edi, byte 4
;				add eax, edx
;				mov dx, ax
;				shr eax, 16
;				or dx, ax
;				dec ecx
;				mov ax, [esi]
;				mov [edi + 0 - 4], dx
;				mov [edi + 2 - 4], ax
;				jnz short .Loop_X

				mov ax, [esi]
				mov dx, [esi + 2]
				shr ax, 1
				add esi, byte 2
				shr dx, 1
				and ax, 0x7BCF
				and dx, 0x7BCF
				add edi, byte 4
				add ax, dx
				dec ecx
				mov dx, [esi]
				mov [edi + 0 - 4], ax
				mov [edi + 2 - 4], dx
				jnz short .Loop_X1

			mov ecx, [esp + 32]			; ecx = Number of pixels per line
			shr ecx, 2				; Black out 16 bytes (32 pixels) per cycle.
			add edi, ebx				; Add the pitch difference to the destination pointer.
			xor edx, edx				; Scanlines are all black.
			jmp short .Loop_X2
			
	ALIGN64
	
	.Loop_X2:
				mov [edi], edx
				mov [edi + 4], edx
				mov [edi + 8], edx
				mov [edi + 12], edx
				add edi, 16
				
				dec ecx
				jnz short .Loop_X2

			add esi, [esp + 40]			; Add the line offset.
			add edi, ebx				; Add the pitch difference to the destination pointer.
			dec dword [esp + 36]			; Reset the X conuter.
			mov ecx, [esp + 32]			; Decrement the Y counter.
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64
	
	;*********************************************************************************
	; void Blit2x_Scanline_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_Int_16_asm_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per line
		mov ebx, [esp + 28]				; ebx = Pitch of destination surface (bytes per line)
		lea esi, [MD_Screen + 8 * 2]			; esi = Source
		lea ecx, [ecx * 4]				; ecx = Number of bytes per line
		sub ebx, ecx					; ebx = Difference between dest pitch and src pitch
		shr ecx, 4					; Transfer 16 bytes per cycle. (32 16-bit pixels)
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; Initialize the X counter.

		movq mm7, [MASK_DIV2_15]
		; Check if this is 15-bit color mode.
		cmp byte [bpp], 15			; set good mask for current video mode
		je short .Loop_Y

		; 16-bit color mode.
		movq mm7, [MASK_DIV2_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X1:
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm7
				pand mm1, mm7
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X1

			mov ecx, [esp + 32]			; Reset the X counter.
			shr ecx, 2				; Black out 64 bytes (128 pixels) per cycle.
			add edi, ebx				; Add the pitch difference to the destination pointer.
			pxor mm0, mm0				; Scanlines are all black.
			jmp short .Loop_X2

	ALIGN64

	.Loop_X2:
				movq [edi + 0], mm0
				movq [edi + 8], mm0
				movq [edi + 16], mm0
				movq [edi + 24], mm0
				movq [edi + 32], mm0
				movq [edi + 40], mm0
				movq [edi + 48], mm0
				movq [edi + 56], mm0
				
				add edi, byte 64
				dec ecx
				jnz short .Loop_X2

			add esi, [esp + 40]			; Add the line offset.
			add edi, ebx				; Add the pitch difference to the destination pointer.
			mov ecx, [esp + 32]			; Reset the X counter.
			dec dword [esp + 36]			; Decrement the Y counter.
			jnz near .Loop_Y

		emms
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
