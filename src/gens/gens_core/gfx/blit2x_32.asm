;
; Gens: 2x renderer. [32-bit color] (x86 ASM version)
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

	extern MD_Screen32
	extern Have_MMX

section .text align=64

	ALIGN64

	;************************************************************************
	; void Blit2x_32_asm(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_32_asm

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per line
		mov ebx, [esp + 28]				; ebx = Pitch of destination surface (bytes per line)
		lea ecx, [ecx * 8]				; ecx = Number of bytes per line
		lea esi, [MD_Screen32 + 8 * 4]			; esi = Source
		sub ebx, ecx					; ebx = Difference between dest pitch and src pitch
		shr ecx, 4					; Transfer 16 bytes per cycle. (4 32-bit pixels)
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X1:
				mov eax, [esi]			; First pixel.
				mov [edi], eax
				mov [edi + 4], eax
				
				mov eax, [esi + 4]		; Second pixel.
				mov [edi + 8], eax
				mov [edi + 12], eax
				
				add esi, 8
				add edi, 16
				
				dec ecx
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = Number of pixels per line
			add edi, ebx				; Add the pitch difference to the destination pointer.
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2:
				mov eax, [esi]			; First pixel.
				mov [edi], eax
				mov [edi + 4], eax
				
				mov eax, [esi + 4]		; Second pixel.
				mov [edi + 8], eax
				mov [edi + 12], eax
				
				add esi, 8
				add edi, 16
				
				dec ecx
				jnz short .Loop_X2

			add esi, [esp + 40]			; Add 2x the line offset. (1x offset is 16-bit only)
			add esi, [esp + 40]
			add edi, ebx				; Add the pitch difference to the destination pointer.
			mov ecx, [esp + 32]			; Reset the X counter.
			dec dword [esp + 36]			; Decrement the Y counter.
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN64

	;************************************************************************
	; void Blit2x_32_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_32_asm_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per line
		mov ebx, [esp + 28]				; ebx = Pitch of destination surface (bytes per line)
		lea esi, [MD_Screen32 + 8 * 4]			; esi = Source
		lea ecx, [ecx * 8]				; ecx = Number of bytes per line
		sub ebx, ecx					; ebx = Difference between dest pitch and src pitch
		shr ecx, 6					; Transfer 64 bytes per cycle. (16 32-bit pixels)
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
				
				punpckldq mm1, mm1
				punpckhdq mm0, mm0
				punpckldq mm3, mm3
				punpckhdq mm2, mm2
				punpckldq mm5, mm5
				punpckhdq mm4, mm4
				punpckldq mm7, mm7
				punpckhdq mm6, mm6
				
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
			shl ecx, 5
			sub esi, ecx
			shr ecx, 5
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2:
				movq mm0, [esi]
				movq mm1, mm0
				movq mm2, [esi + 8]
				movq mm3, mm2
				movq mm4, [esi + 16]
				movq mm5, mm4
				movq mm6, [esi + 24]
				movq mm7, mm6
				
				punpckldq mm1, mm1
				punpckhdq mm0, mm0
				punpckldq mm3, mm3
				punpckhdq mm2, mm2
				punpckldq mm5, mm5
				punpckhdq mm4, mm4
				punpckldq mm7, mm7
				punpckhdq mm6, mm6
				
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
				jnz short .Loop_X2

			add esi, [esp + 40]			; Add 2x the line offset. (1x offset is 16-bit only)
			add esi, [esp + 40]
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
