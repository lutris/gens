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
	extern TAB336
	extern Have_MMX
	extern Mode_555


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
		shr ecx, 4					; Transfer 16 bytes per cycle. (8 16-bit pixels)
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

		mov ecx, [esp + 32]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 28]				; ebx = pitch de la surface Dest
		lea ecx, [ecx * 4]				; ecx = Nb bytes par ligne Dest
		add ebx, ebx					; ebx = pitch * 2
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 5						; on transfert 32 bytes Dest � chaque boucle
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X
		jnp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				movq mm0, [esi]
				add edi, byte 32
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm3, mm2
				punpcklwd mm1, mm0
				add esi, byte 16
				punpckhwd mm0, mm0
				movq [edi + 0 - 32], mm1
				punpcklwd mm3, mm2
				movq [edi + 8 - 32], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 32], mm3
				dec ecx
				movq [edi + 24 - 32], mm2
				jnz short .Loop_X
	
			add esi, [esp + 40]			; on augmente la source pour pointer sur la prochaine ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			dec dword [esp + 36]		; on continue tant qu'il reste des lignes
			mov ecx, [esp + 32]
			jnz short .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
