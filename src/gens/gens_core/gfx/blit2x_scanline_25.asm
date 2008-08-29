;
; Gens: 2x 25% scanline renderer. (x86 ASM version)
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
	extern bpp

	MASK_DIV2_15:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16:	dd 0x7BEF7BEF, 0x7BEF7BEF
	MASK_DIV4_15:	dd 0x1CE71CE7, 0x1CEF1CE7
	MASK_DIV4_16:	dd 0x39E739E7, 0x39E739E7


section .text align=64


	ALIGN64

	;*******************************************************************************
	; void Blit2x_Scanline_25_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_25_16_asm_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 28]				; ebx = pitch de la surface Dest
		lea ecx, [ecx * 4]				; ecx = Nb bytes par ligne Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 5						; on transfert 32 bytes screen � chaque boucle
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X

		movq mm6, [MASK_DIV2_15]
		movq mm7, [MASK_DIV4_15]
		; Check if this is 15-bit color mode.
		cmp byte [bpp], 15
		je short .Loop_Y

		; 16-bit color mode.
		movq mm6, [MASK_DIV2_16]
		movq mm7, [MASK_DIV4_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
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
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 32 dans une ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			shl ecx, 4
			sub esi, ecx
			shr ecx, 4
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 32
				movq mm3, [esi + 8]
				movq mm2, mm0
				movq mm5, mm3
				psrlq mm0, 2
				psrlq mm2, 1
				psrlq mm3, 2
				psrlq mm5, 1
				pand mm0, mm7
				pand mm2, mm6
				pand mm3, mm7
				pand mm5, mm6
				paddw mm0, mm2
				paddw mm3, mm5
				movq mm1, mm0
				movq mm4, mm3
				punpcklwd mm1, mm1
				add esi, byte 16
				punpckhwd mm0, mm0
				movq [edi + 0 - 32], mm1
				punpcklwd mm4, mm4
				movq [edi + 8 - 32], mm0
				punpckhwd mm3, mm3
				movq [edi + 16 - 32], mm4
				dec ecx
				movq [edi + 24 - 32], mm3
				jnz short .Loop_X2

			add esi, [esp + 40]			; on augmente la source pour pointer sur la prochaine ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			dec dword [esp + 36]		; on continue tant qu'il reste des lignes
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 4 dans une ligne
			jnz near .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
