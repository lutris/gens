;
; Gens: [MDP] 2x renderer. [16-bit color] (x86 asm version)
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

arg_destScreen	equ 24
arg_mdScreen	equ 28
arg_width	equ 32
arg_height	equ 36
arg_pitch	equ 40
arg_offset	equ 44

section .text align=64

	ALIGN64

	;************************************************************************
	; void mdp_render_2x_16_x86(uint16_t *destScreen, uint16_t *mdScreen, int width, int height, int pitch, int offset);
	DECL mdp_render_2x_16_x86

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov ebx, [esp + arg_pitch]		; ebx = Pitch of destination surface (bytes per line)
		mov esi, [esp + arg_mdScreen]		; esi = Source
		lea ecx, [ecx * 4]			; ecx = Number of bytes per line
		sub ebx, ecx				; ebx = Difference between dest pitch and src pitch
		shr ecx, 4				; Transfer 16 bytes per cycle. (32 16-bit pixels)
		shl dword [esp + arg_offset], 1		; Adjust offset for 16-bit color.
		mov edi, [esp + arg_destScreen]		; edi = Destination
		mov [esp + arg_width], ecx		; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X1:
				mov eax, [esi]		; First two pixels.
				add esi, 8
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 16
				mov eax, [esi - 4]	; Second two pixels.
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
				jnz short .Loop_X1
	
			mov ecx, [esp + arg_width]	; ecx = Number of pixels per line
			add edi, ebx			; Add the pitch difference to the destination pointer.
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2:
				mov eax, [esi]		; First two pixels.
				add esi, 8
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 16
				mov eax, [esi - 4]	; Second two pixels.
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
				jnz short .Loop_X2

			add esi, [esp + arg_offset]	; Add the line offset.
			add edi, ebx			; Add the pitch difference to the destination pointer.
			mov ecx, [esp + arg_width]	; Reset the X conuter.
			dec dword [esp + arg_height]	; Decrement the Y counter.
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN64

	;************************************************************************
	; void mdp_render_2x_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen, int width, int height, int pitch, int offset);
	DECL mdp_render_2x_16_x86_mmx

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov ebx, [esp + arg_pitch]		; ebx = Pitch of destination surface (bytes per line)
		mov esi, [esp + arg_mdScreen]		; esi = Source
		lea ecx, [ecx * 4]			; ecx = Number of bytes per line
		sub ebx, ecx				; ebx = Difference between dest pitch and src pitch
		shr ecx, 6				; Transfer 64 bytes per cycle. (128 16-bit pixels)
		shl dword [esp + arg_offset], 1		; Adjust offset for 16-bit color.
		mov edi, [esp + arg_destScreen]		; edi = Destination
		mov [esp + arg_width], ecx		; Initialize the X counter.
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
	
			mov ecx, [esp + arg_width]	; Reset the X counter.
			add edi, ebx			; Add the pitch difference to the destination pointer.
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
				jnz short .Loop_X2

			add esi, [esp + arg_offset]	; Add the line offset.
			add edi, ebx			; Add the pitch difference to the destination pointer.
			mov ecx, [esp + arg_width]	; Reset the X counter.
			dec dword [esp + arg_height]	; Decrement the Y counter.
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
