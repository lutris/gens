%include "nasmhead.inc"


section .data align=64

	extern MD_Screen
	extern TAB336
	extern Have_MMX
	extern Mode_555


section .text align=64


	ALIGN64
	
	;************************************************************************
	; void Blit_X1(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X1

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 28]				; ebx = pitch de la surface Dest
		add ecx, ecx					; ecx = Nb bytes par ligne
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 3						; on transfert 8 bytes � chaque boucle
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				mov eax, [esi]			; on transferts 2 pixels d'un coup	
				mov edx, [esi + 4]		; on transferts 2 pixels d'un coup	
				add esi, 8
				mov [edi], eax
				mov [edi + 4], edx
				add edi, 8
				dec ecx
				jnz .Loop_X
	
			add esi, [esp + 40]			; on augmente la source pour pointer sur la prochaine ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			dec dword [esp + 36]		; on continue tant qu'il reste des lignes
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 4 dans une ligne
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret


	ALIGN64
	
	;************************************************************************
	; void Blit_X1_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_X1_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 32]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 28]				; ebx = pitch de la surface Dest
		add ecx, ecx					; ecx = Nb bytes par ligne
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 6						; on transfert 64 bytes � chaque boucle
		mov edi, [esp + 24]				; edi = Destination
		mov [esp + 32], ecx				; on stocke cette nouvelle valeur pour X
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X
				movq mm0, [esi]
				add edi, 64
				movq mm1, [esi + 8]
				movq mm2, [esi + 16]
				movq mm3, [esi + 24]
				movq mm4, [esi + 32]
				movq mm5, [esi + 40]
				movq mm6, [esi + 48]
				movq mm7, [esi + 56]
				movq [edi + 0 - 64], mm0
				add esi, 64
				movq [edi + 8 - 64], mm1
				movq [edi + 16 - 64], mm2
				movq [edi + 24 - 64], mm3
				movq [edi + 32 - 64], mm4
				movq [edi + 40 - 64], mm5
				movq [edi + 48 - 64], mm6
				dec ecx
				movq [edi + 56 - 64], mm7
				jnz .Loop_X
	
			add esi, [esp + 40]			; on augmente la source pour pointer sur la prochaine ligne
			add edi, ebx				; on augmente la destination avec le debordement du pitch
			dec dword [esp + 36]		; on continue tant qu'il reste des lignes
			mov ecx, [esp + 32]			; ecx = Nombre de pixels / 4 dans une ligne
			jnz .Loop_Y

		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
