;
; Gens: 2x interpolated 50% scanline renderer. (x86 ASM version)
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

	MASK_DIV2_15:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16:	dd 0x7BEF7BEF, 0x7BEF7BEF
	MASK_DIV4_15:	dd 0x1CE71CE7, 0x1CEF1CE7
	MASK_DIV4_16:	dd 0x39E739E7, 0x39E739E7


section .text align=64


	ALIGN64
	

	;*******************************************************************************
	; void Blit2x_Scanline_50_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit2x_Scanline_50_Int_16_asm_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 28]				; ebx = pitch de la surface screen
		lea ecx, [ecx * 4]				; ecx = Nb bytes par ligne screen
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 4						; on transfert 32 bytes screen � chaque boucle
		mov edi, [esp + 24]				; edi = destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X
		test byte [Mode_555], 1
		movq mm6, [MASK_DIV2_15]
		movq mm7, [MASK_DIV4_15]
		jnz short .Loop_Y

		movq mm6, [MASK_DIV2_16]
		movq mm7, [MASK_DIV4_16]
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm6
				pand mm1, mm6
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 2
				psrlw mm2, 1
				psrlw mm1, 2
				pand mm0, mm7
				pand mm2, mm6
				pand mm1, mm7
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [edi + 0 - 16], mm2
				dec ecx
				movq [edi + 8 - 16], mm3
				jnz short .Loop_X2

			add esi, [esp + 40]			; on augmente la source pour pointer sur la prochaine ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			dec dword [esp + 36]		; on continue tant qu'il reste des lignes
			mov ecx, [esp + 32]
			jnz near .Loop_Y

	.End
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
