;
; Gens: [MDP] 1x renderer. [32-bit color] (x86 asm version)
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
	; void mdp_render_1x_32_x86(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL mdp_render_1x_32_x86

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per line
		mov ebx, [esp + 28]				; ebx = Pitch of destination surface (bytes per line)
		lea esi, [MD_Screen32 + 8 * 4]			; esi = Source
		shl ecx, 2					; ecx = Number of bytes per line
		sub ebx, ecx					; ebx = Difference between dest pitch and src pitch
		shr ecx, 3					; Transfer 8 bytes per cycle. (2 32-bit pixels)
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X:
				mov eax, [esi]			; First pixel.
				mov edx, [esi + 4]		; Second pixel.
				add esi, 8
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 8
				dec ecx
				jnz .Loop_X
	
			add esi, [esp + 40]			; Add 2x the line offset. (1x offset is 16-bit only)
			add esi, [esp + 40]
			add edi, ebx				; Add the pitch difference to the destination pointer.
			mov ecx, [esp + 32]			; Reset the X counter.
			dec dword [esp + 36]			; Decrement the Y counter.
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN64
	
	;************************************************************************
	; void mdp_render_1x_32_x86_mmx(unsigned char *screen, int pitch, int x, int y, int offset)
	; TODO
	DECL mdp_render_1x_32_x86_mmx

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Number of pixels per line
		mov ebx, [esp + 28]				; ebx = Pitch of destination surface (bytes per line)
		lea esi, [MD_Screen32 + 8 * 4]			; esi = Source
		shl ecx, 2					; ecx = Number of bytes per line
		sub ebx, ecx					; ebx = Difference between dest pitch and src pitch
		shr ecx, 6					; Transfer 64 bytes per cycle. (16 32-bit pixels)
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X:
				; Get source pixels.
				movq mm0, [esi]
				movq mm1, [esi + 8]
				movq mm2, [esi + 16]
				movq mm3, [esi + 24]
				movq mm4, [esi + 32]
				movq mm5, [esi + 40]
				movq mm6, [esi + 48]
				movq mm7, [esi + 56]
				
				; Put destination pixels.
				movq [edi], mm0
				movq [edi + 8], mm1
				movq [edi + 16], mm2
				movq [edi + 24], mm3
				movq [edi + 32], mm4
				movq [edi + 40], mm5
				movq [edi + 48], mm6
				movq [edi + 56], mm7
				
				; Loop management.
				add esi, 64
				add edi, 64
				dec ecx
				jnz .Loop_X
			
			add esi, [esp + 40]			; Add 2x the line offset. (1x offset is 16-bit only)
			add esi, [esp + 40]
			add edi, ebx				; Add the pitch difference to the destination pointer.
			mov ecx, [esp + 32]			; Reset the X counter.
			dec dword [esp + 36]			; Decrement the Y counter.
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
