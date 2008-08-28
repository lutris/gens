;
; Gens: 2x renderer. (x86 ASM version)
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
	; void Blit_2x_16_asm(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit_2x_16_asm

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
		shr ecx, 4						; on transfert 16 bytes Dest � chaque boucle
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				mov eax, [esi]					; on transferts 2 pixels d'un coup	
				add esi, 8
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 16
				mov eax, [esi - 4]				; on transferts 2 pixels d'un coup	
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 4 dans une ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			shl ecx, 3
			sub esi, ecx
			shr ecx, 3
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				mov eax, [esi]					; on transferts 2 pixels d'un coup	
				add esi, 8
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 16
				mov eax, [esi - 4]				; on transferts 2 pixels d'un coup	
				mov edx, eax
				rol eax, 16
				xchg ax, dx
				dec ecx
				mov [edi - 8], eax
				mov [edi - 4], edx
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
		ret


	ALIGN64

	;************************************************************************
	; void Blit_2x_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset)
	DECL Blit_2x_16_asm_MMX

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
		shr ecx, 6						; on transfert 64 bytes Dest � chaque boucle
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add edi, byte 64
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm4, [esi + 16]
				movq mm3, mm2
				movq mm6, [esi + 24]
				movq mm5, mm4
				movq mm7, mm6
				punpcklwd mm1, mm1
				punpckhwd mm0, mm0
				movq [edi + 0 - 64], mm1
				punpcklwd mm3, mm3
				movq [edi + 8 - 64], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 64], mm3
				punpcklwd mm5, mm5
				movq [edi + 24 - 64], mm2
				punpckhwd mm4, mm4
				movq [edi + 32 - 64], mm5
				punpcklwd mm7, mm7
				movq [edi + 40 - 64], mm4
				punpckhwd mm6, mm6
				movq [edi + 48 - 64], mm7
				add esi, byte 32
				dec ecx
				movq [edi + 56 - 64], mm6
				jnz short .Loop_X1
	
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 4 dans une ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			shl ecx, 5
			sub esi, ecx
			shr ecx, 5
			jmp short .Loop_X2

	ALIGN64
	
	.Loop_X2
				movq mm0, [esi]
				add edi, byte 64
				movq mm2, [esi + 8]
				movq mm1, mm0
				movq mm4, [esi + 16]
				movq mm3, mm2
				movq mm6, [esi + 24]
				movq mm5, mm4
				movq mm7, mm6
				punpcklwd mm1, mm1
				punpckhwd mm0, mm0
				movq [edi + 0 - 64], mm1
				punpcklwd mm3, mm3
				movq [edi + 8 - 64], mm0
				punpckhwd mm2, mm2
				movq [edi + 16 - 64], mm3
				punpcklwd mm5, mm5
				movq [edi + 24 - 64], mm2
				punpckhwd mm4, mm4
				movq [edi + 32 - 64], mm5
				punpcklwd mm7, mm7
				movq [edi + 40 - 64], mm4
				punpckhwd mm6, mm6
				movq [edi + 48 - 64], mm7
				add esi, byte 32
				dec ecx
				movq [edi + 56 - 64], mm6
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
