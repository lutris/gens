;
; Gens: [MDP] 1x renderer. [16-bit color] (x86 ASM version)
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
	; void mdp_render_1x_16_x86(uint16_t *destScreen, uint16_t *mdScreen, int width, int height, int pitch, int offset);
	DECL mdp_render_1x_16_x86

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov ebx, [esp + arg_pitch]		; ebx = Pitch of destination surface (bytes per line)
		mov esi, [esp + arg_mdScreen]		; esi = Source
		add ecx, ecx				; ecx = Number of bytes per line
		sub ebx, ecx				; ebx = Difference between dest pitch and src pitch
		shr ecx, 3				; Transfer 8 bytes per cycle. (4 16-bit pixels)
		mov edi, [esp + arg_destScreen]		; edi = Destination
		mov [esp + arg_width], ecx		; Initialize the X counter.
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y:
	.Loop_X:
				mov eax, [esi]		; First two pixels.
				mov edx, [esi + 4]	; Second two pixels.
				add esi, 8
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 8
				dec ecx
				jnz .Loop_X
	
			add esi, [esp + arg_offset]	; Add the line offset.
			add edi, ebx			; Add the pitch difference to the destination pointer.
			mov ecx, [esp + arg_width]	; Reset the X counter.
			dec dword [esp + arg_height]	; Decrement the Y counter.
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	ALIGN64
	
	;************************************************************************
	; void mdp_render_1x_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen, int width, int height, int pitch, int offset);
	DECL mdp_render_1x_16_x86_mmx

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov ebx, [esp + arg_pitch]		; ebx = Pitch of destination surface (bytes per line)
		mov esi, [esp + arg_mdScreen]		; esi = Source
		add ecx, ecx				; ecx = Number of bytes per line
		sub ebx, ecx				; ebx = Difference between dest pitch and src pitch
		shr ecx, 6				; Transfer 64 bytes per cycle. (32 16-bit pixels)
		mov edi, [esp + arg_destScreen]		; edi = Destination
		mov [esp + arg_width], ecx		; Initialize the X counter.
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
			
			add esi, [esp + arg_offset]	; Add the line offset.
			add edi, ebx			; Add the pitch difference to the destination pointer.
			mov ecx, [esp + arg_width]	; Reset the X counter.
			dec dword [esp + arg_height]	; Decrement the Y counter.
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
