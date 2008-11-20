;
; Gens: [MDP] Scanline renderer. [32-bit color] (x86 asm version)
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

arg_destScreen	equ 24
arg_mdScreen	equ 28
arg_width	equ 32
arg_height	equ 36
arg_pitch	equ 40
arg_offset	equ 44

%ifdef __OBJ_ELF
%define _mdp_render_scanline_32_x86 mdp_render_scanline_32_x86
%define _mdp_render_scanline_32_x86_mmx mdp_render_scanline_32_x86_mmx
%endif

section .text align=64

	align 64

	;************************************************************************
	; void mdp_render_scanline_32_x86(uint32_t *destScreen, uint32_t *mdScreen, int width, int height, int pitch, int offset);
	global _mdp_render_scanline_32_x86
	mdp_render_scanline_32_x86:

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov ebx, [esp + arg_pitch]		; ebx = Pitch of destination surface (bytes per line)
		mov esi, [esp + arg_mdScreen]		; esi = Source
		lea ecx, [ecx * 8]			; ecx = Number of bytes per line
		sub ebx, ecx				; ebx = Difference between dest pitch and src pitch
		shr ecx, 4				; Transfer 16 bytes per cycle. (4 32-bit pixels)
		shl dword [esp + arg_offset], 2		; Adjust offset for 32-bit color.
		mov edi, [esp + arg_destScreen]		; edi = Destination
		mov [esp + arg_width], ecx		; Initialize the X counter.
		jmp short .Loop_Y

	align 64

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
	
			mov ecx, [esp + arg_width]		; ecx = Number of pixels per line
			add edi, ebx				; Add the pitch difference to the destination pointer.
			xor edx, edx				; Scanlines are all black.
			jmp short .Loop_X2

	align 64
	
	.Loop_X2:
				mov [edi], edx
				mov [edi + 4], edx
				mov [edi + 8], edx
				mov [edi + 12], edx
				add edi, 16
				
				dec ecx
				jnz short .Loop_X2

			add esi, [esp + arg_offset]		; Add the line offset.
			add edi, ebx				; Add the pitch difference to the destination pointer.
			mov ecx, [esp + arg_width]		; Reset the X counter.
			dec dword [esp + arg_height]		; Decrement the Y counter.
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret

	align 64

	;************************************************************************
	; void mdp_render_scanline_32_x86_mmx(uint32_t *destScreen, uint32_t *mdScreen, int width, int height, int pitch, int offset);
	global _mdp_render_scanline_32_x86_mmx
	_mdp_render_scanline_32_x86_mmx:

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + arg_width]		; ecx = Number of pixels per line
		mov ebx, [esp + arg_pitch]		; ebx = Pitch of destination surface (bytes per line)
		mov esi, [esp + arg_mdScreen]		; esi = Source
		lea ecx, [ecx * 8]			; ecx = Number of bytes per line
		sub ebx, ecx				; ebx = Difference between dest pitch and src pitch
		shr ecx, 6				; Transfer 64 bytes per cycle. (16 32-bit pixels)
		shl dword [esp + arg_offset], 2		; Adjust offset for 32-bit color.
		mov edi, [esp + arg_destScreen]		; edi = Destination
		mov [esp + arg_width], ecx		; Initialize the X counter.
		jmp short .Loop_Y

	align 64

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
			
			mov ecx, [esp + arg_width]	; Reset the X counter.
			add edi, ebx			; Add the pitch difference to the destination pointer.
			pxor mm0, mm0			; Scanlines are all black.
			jmp short .Loop_X2

	align 64
	
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
