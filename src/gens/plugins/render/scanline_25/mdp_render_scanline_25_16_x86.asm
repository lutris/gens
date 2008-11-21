;
; Gens: [MDP] 25% Scanline renderer. [16-bit color] (x86 asm version)
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
arg_mask2	equ 48
arg_mask4	equ 52
arg_mode555	equ 48

%ifdef __OBJ_ELF
%define _mdp_render_scanline_25_16_x86 mdp_render_scanline_25_16_x86
%define _mdp_render_scanline_25_16_x86_mmx mdp_render_scanline_25_16_x86_mmx
%endif

section .data align=64

	; 64-bit masks used for the mmx version.
	MASK_DIV2_15_MMX:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16_MMX:	dd 0x7BEF7BEF, 0x7BEF7BEF
	MASK_DIV4_15_MMX:	dd 0x1CE71CE7, 0x1CE71CE7
	MASK_DIV4_16_MMX:	dd 0x39E739E7, 0x39E739E7

section .text align=64

	align 64

	;************************************************************************
	; void mdp_render_scanline_50_16_x86(uint16_t *destScreen, uint16_t *mdScreen,
	;				     int width, int height, int pitch, int offset,
	;				     uint32_t mask);
	global _mdp_render_scanline_25_16_x86
	_mdp_render_scanline_25_16_x86:

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

	align 64

	.Loop_Y:
	.Loop_X1:
				mov eax, [esi]		; First two pixels.
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				
				mov eax, [esi + 4]	; Second two pixels.
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi + 8], eax
				mov [edi + 12], edx
				
				add esi, 8
				add edi, 16
				dec ecx
				jnz short .Loop_X1
	
			mov ecx, [esp + arg_width]	; ecx = Number of pixels per line
			add edi, ebx			; Add the pitch difference to the destination pointer.
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	align 64
	
	.Loop_X2:
				mov eax, [esi]			; First two pixels.
				mov edx, eax
				shr eax, 1			; 50% scanline
				and eax, [esp + arg_mask2]	; Mask off the MSB of each color component.
				shr edx, 2			; 25% scanline
				and edx, [esp + arg_mask4]	; Mask off the MSB of each color component.
				add eax, edx			; Combine the two values.
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				
				mov eax, [esi + 4]		; Second two pixels.
				mov edx, eax
				shr eax, 1			; 50% scanline
				and eax, [esp + arg_mask2]	; Mask off the MSB of each color component.
				shr edx, 2			; 25% scanline
				and edx, [esp + arg_mask4]	; Mask off the MSB of each color component.
				add eax, edx			; Combine the two values.
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi + 8], eax
				mov [edi + 12], edx
				
				add esi, 8
				add edi, 16
				dec ecx
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

	align 64

	;************************************************************************
	; void mdp_render_scanline_25_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen,
	;					 int width, int height, int pitch, int offset,
	;					 int mode555);
	global _mdp_render_scanline_25_16_x86_mmx
	_mdp_render_scanline_25_16_x86_mmx:

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
		shr ecx, 5				; Transfer 32 bytes per cycle. (64 16-bit pixels)
		shl dword [esp + arg_offset], 1		; Adjust offset for 16-bit color.
		mov edi, [esp + arg_destScreen]		; edi = Destination
		mov [esp + arg_width], ecx		; Initialize the X counter.
		
		; Check which mask to use.
		movq mm7, [MASK_DIV2_16_MMX]
		movq mm6, [MASK_DIV4_16_MMX]
		test byte [esp + arg_mode555], 1
		jz .Loop_Y
		movq mm7, [MASK_DIV2_15_MMX]		; 15-bit color. (Mode 555)
		movq mm6, [MASK_DIV4_15_MMX]
		jmp short .Loop_Y

	align 64

	.Loop_Y:
	.Loop_X1:
				movq mm0, [esi]
				movq mm1, mm0
				movq mm2, [esi + 8]
				movq mm3, mm2
				
				punpcklwd mm1, mm1
				punpckhwd mm0, mm0
				punpcklwd mm3, mm3
				punpckhwd mm2, mm2
				
				movq [edi], mm1
				movq [edi + 8], mm0
				movq [edi + 16], mm3
				movq [edi + 24], mm2
				
				add edi, byte 32
				add esi, byte 16
				dec ecx
				jnz short .Loop_X1
	
			mov ecx, [esp + arg_width]	; Reset the X counter.
			add edi, ebx			; Add the pitch difference to the destination pointer.
			shl ecx, 4
			sub esi, ecx
			shr ecx, 4
			jmp short .Loop_X2

	align 64
	
	.Loop_X2:
				movq mm0, [esi]
				movq mm2, [esi + 8]
				movq mm4, mm0
				movq mm5, mm2
				
				; 50% Scanline
				psrlq mm0, 1
				psrlq mm2, 1
				pand mm0, mm7
				pand mm2, mm7
				
				; 25% Scanline
				psrlq mm4, 2
				psrlq mm5, 2
				pand mm4, mm6
				pand mm5, mm6
				
				; Combine the two values.
				paddw mm0, mm4
				paddw mm2, mm5
				
				; Expand to 2x size.
				movq mm1, mm0
				movq mm3, mm2
				punpcklwd mm1, mm1
				punpckhwd mm0, mm0
				punpcklwd mm3, mm3
				punpckhwd mm2, mm2
				
				; Copy to the destination buffer.
				movq [edi], mm1
				movq [edi + 8], mm0
				movq [edi + 16], mm3
				movq [edi + 24], mm2
				
				; Next set of pixels.
				add edi, byte 32
				add esi, byte 16
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
