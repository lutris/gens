;
; Gens: 2x scanline renderer. (x86 ASM version)
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

section .text align=64

	ALIGN64
	
	;************************************************************************
	; void Blit2x_Scanline_16_asm(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_16_asm

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
		shr ecx, 4					; Transfer 16 bytes per cycle. (128 16-bit pixels)
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X1:
				mov eax, [esi]			; First two pixels.
				add esi, 8
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 16
				mov eax, [esi - 4]		; Second two pixels.
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = Number of pixels per line
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

	;************************************************************************
	; void Blit2x_Scanline_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_16_asm_MMX

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
		shr ecx, 6					; Transfer 64 bytes per cycle. (128 16-bit pixels)
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X1:
				movq mm0, [esi]
				movq mm1, mm0
				movq mm2, [esi + 8]
				movq mm3, mm2
				movq mm4, [esi + 16]
				movq mm5, mm4
				movq mm6, [esi + 24]
				movq mm7, mm6
				
				punpcklwd mm1, mm1
				punpckhwd mm0, mm0
				punpcklwd mm3, mm3
				punpckhwd mm2, mm2
				punpcklwd mm5, mm5
				punpckhwd mm4, mm4
				punpcklwd mm7, mm7
				punpckhwd mm6, mm6
				
				movq [edi + 0], mm1
				movq [edi + 8], mm0
				movq [edi + 16], mm3
				movq [edi + 24], mm2
				movq [edi + 32], mm5
				movq [edi + 40], mm4
				movq [edi + 48], mm7
				movq [edi + 56], mm6
				
				add edi, byte 64
				add esi, byte 32
				dec ecx
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; Reset the X counter.
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

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
