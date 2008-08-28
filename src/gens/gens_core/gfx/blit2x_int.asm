%include "nasmhead.inc"

	srcPtr        equ 8
	srcPitch      equ 12
	width         equ 16
	dstOffset     equ 20
	dstPitch      equ 24
	dstSegment    equ 28

	colorI   equ -2
	colorE   equ 0
	colorF   equ 2
	colorJ   equ 4

	colorG   equ -2
	colorA   equ 0
	colorB   equ 2
	colorK   equ 4

	colorH   equ -2
	colorC   equ 0
	colorD   equ 2
	colorL   equ 4

	colorM   equ -2
	colorN   equ 0
	colorO   equ 2
	colorP   equ 4

section .data align=64

	extern MD_Screen
	extern TAB336
	extern Have_MMX
	extern Mode_555

	Var:	dd 0
	NB_X:	dd 0
	NB_X2:	dd 0
	NB_X4:	dd 0
	MASK_DIV2:		dd 0x7BCF7BCF, 0x7BCF7BCF
	MASK_DIV2_15:	dd 0x3DEF3DEF, 0x3DEF3DEF
	MASK_DIV2_16:	dd 0x7BEF7BEF, 0x7BEF7BEF
	MASK_DIV4_15:	dd 0x1CE71CE7, 0x1CEF1CE7
	MASK_DIV4_16:	dd 0x39E739E7, 0x39E739E7
	MASK_DIV8_15:	dd 0x0C630C63, 0x0C630C63
	MASK_DIV8_16:	dd 0x18E318E3, 0x18E318E3
	MASK_RBG_15:	dd 0x7C1F03E0, 0x7C1F03E0
	MASK_GRB_15:	dd 0x03E07C1F, 0x03E07C1F
	MASK_RBG_16:	dd 0xF81F07E0, 0xF81F07E0
	MASK_GRB_16:	dd 0x07E0F81F, 0x07E0F81F
	MASK_RBG_15_2:	dd 0x3C0F01E0, 0x3C0F01E0
	MASK_GRB_15_2:	dd 0x01E03C0F, 0x01E03C0F
	MASK_RBG_16_2:	dd 0x780F03E0, 0x780F03E0
	MASK_GRB_16_2:	dd 0x03E0780F, 0x03E0780F
	MASK_GG_15:		dd 0x03E003E0, 0x03E003E0
	MASK_RBRB_15:	dd 0x7C1F7C1F, 0x7C1F7C1F
	MASK_GG_16:		dd 0x07C007C0, 0x07C007C0
	MASK_RBRB_16	dd 0xF81FF81F, 0xF81FF81F

	; 2xSAI

	ALIGNB32

	colorMask:		dd 0xF7DEF7DE,0xF7DEF7DE
	lowPixelMask:	dd 0x08210821,0x08210821

	qcolorMask:		dd 0xE79CE79C,0xE79CE79C
	qlowpixelMask:	dd 0x18631863,0x18631863

	darkenMask:		dd 0xC718C718,0xC718C718
	GreenMask:		dd 0x07E007E0,0x07E007E0
	RedBlueMask:	dd 0xF81FF81F,0xF81FF81F

	FALSE:			dd 0x00000000,0x00000000
	TRUE:			dd 0xffffffff,0xffffffff
	ONE:			dd 0x00010001,0x00010001

	colorMask15		dd 0x7BDE7BDE,0x7BDE7BDE
	lowPixelMask15	dd 0x04210421,0x04210421

	qcolorMask15	dd 0x739C739C,0x739C739C
	qlowpixelMask15	dd 0x0C630C63,0x0C630C63

	darkenMask15	dd 0x63186318,0x63186318
	GreenMask15		dd 0x03E003E0,0x03E003E0
	RedBlueMask15	dd 0x7C1F7C1F,0x7C1F7C1F



section .bss align=64

	LineBuffer:	resb 32
	Mask1:		resb 8
	Mask2:		resb 8
	ACPixel:	resb 8

	Line1Int:	resb 640 * 2
	Line2Int:	resb 640 * 2
	Line1IntP:	resd 1
	Line2IntP:	resb 1

section .text align=64


	ALIGN64
	
	;*********************************************************************************
	; void Blit_2x_Int_16_asm(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_2x_Int_16_asm

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		mov ecx, [esp + 36]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 32]				; ebx = pitch de la surface Dest
		mov eax, Line1Int				; eax = offset Line 1 Int buffer
		lea ecx, [ecx * 4]				; ecx = Nb bytes par ligne Dest
		mov [Line1IntP], eax			; store first buffer addr
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		mov eax, Line2Int				; eax = offset Line 2 Int buffer
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 2						; on transfert 4 bytes Dest � chaque boucle
		mov [Line2IntP], eax			; store second buffer addr
		mov edi, [esp + 28]				; edi = Dest
		mov [esp + 32], ebx				; on stocke compl�ment offset pour ligne suivante
		mov [esp + 36], ecx				; on stocke cette nouvelle valeur pour X
		mov ebp, [Line1IntP]			; ebp = First Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
		jmp short .First_Copy

	ALIGN4

	.First_Copy
	.Loop_X_FL
			mov ax, [esi]
			mov dx, [esi + 2]
			shr ax, 1
			add esi, byte 2
			shr dx, 1
			and ax, 0x7BCF
			and dx, 0x7BCF
			add ebp, byte 4
			add ax, dx
			dec ecx
			mov dx, [esi]
			mov [ebp + 0 - 4], ax
			mov [ebp + 2 - 4], dx
			jnz short .Loop_X_FL

		dec dword [esp + 40]			; une ligne en moins
		jz near .Last_Line

		mov ecx, [esp + 36]				; on transfert 4 octects Dest par it�ration...
		add esi, [esp + 44]				; on augmente la source pour pointer sur la prochaine ligne
		mov ebp, [Line2IntP]			; ebp = Second Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				mov ax, [esi]
				mov dx, [esi + 2]
				shr ax, 1
				add esi, byte 2
				shr dx, 1
				and ax, 0x7BCF
				and dx, 0x7BCF
				add ebp, byte 4
				add ax, dx
				dec ecx
				mov dx, [esi]
				mov [ebp + 0 - 4], ax
				mov [ebp + 2 - 4], dx
				jnz short .Loop_X1

			mov ecx, [esp + 36]				; Dest num bytes / 4
			add esi, [esp + 44]				; on augmente la source pour pointer sur la prochaine ligne
			shr ecx, 1						; Dest num bytes / 8
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			jmp short .Loop_X2

	ALIGN64

	.Loop_X2
				mov eax, [ebx]
				mov edx, [ebx + 4]
				add edi, byte 8
				add ebx, byte 8
				dec ecx
				mov [edi + 0 - 8], eax
				mov [edi + 4 - 8], edx
				jnz short .Loop_X2

			mov ecx, [esp + 36]				; Dest num bytes / 4
			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			jmp short .Loop_X3

	ALIGN64

	.Loop_X3
				mov eax, [ebx]
				mov edx, [ebp]
				shr eax, 1
				add edi, byte 4
				shr edx, 1
				and eax, 0x7BCF7BCF
				and edx, 0x7BCF7BCF
				add ebx, byte 4
				add eax, edx
				add ebp, byte 4
				dec ecx
				mov [edi - 4], eax
				jnz short .Loop_X3

			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; Swap line buffer
			mov ebp, [Line2IntP]
			mov [Line2IntP], ebx
			mov [Line1IntP], ebp
			mov ecx, [esp + 36]				; on transfert 4 octects Dest par it�ration...
			dec dword [esp + 40]			; encore des lignes ?
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
			jnz near .Loop_Y

	.Last_Line
		mov ecx, [esp + 36]				; Dest num bytes / 4
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		shr ecx, 1						; Dest num bytes / 8
		jmp short .Loop_X1_LL

	ALIGN4

	.Loop_X1_LL
			mov eax, [ebx]
			mov edx, [ebx + 4]
			add edi, byte 8
			add ebx, byte 8
			dec ecx
			mov [edi + 0 - 8], eax
			mov [edi + 4 - 8], edx
			jnz short .Loop_X1_LL

		add edi, [esp + 32]				; Next Dest line
		mov ecx, [esp + 36]				; Dest num bytes / 4
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		shr ecx, 1						; Dest num bytes / 8
		jmp short .Loop_X2_LL

	ALIGN4

	.Loop_X2_LL
			mov eax, [ebx]
			add edi, byte 8
			mov edx, [ebx + 4]
			shr eax, 1
			add ebx, byte 8
			shr edx, 1
			and eax, 0x7BCF7BCF
			and edx, 0x7BCF7BCF
			mov [edi + 0 - 8], eax
			dec ecx
			mov [edi + 4 - 8], edx
			jnz short .Loop_X2_LL

		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret




	ALIGN64
	
	;*********************************************************************************
	; void Blit_2x_Int_16_asm_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_2x_Int_16_asm_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi
		push ebp

		mov ecx, [esp + 36]				; ecx = Nombre de pix par ligne
		mov ebx, [esp + 32]				; ebx = pitch de la surface Dest
		mov eax, Line1Int				; eax = offset Line 1 Int buffer
		lea ecx, [ecx * 4]				; ecx = Nb bytes par ligne Dest
		mov [Line1IntP], eax			; store first buffer addr
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		mov eax, Line2Int				; eax = offset Line 2 Int buffer
		sub ebx, ecx					; ebx = Compl�ment offset pour ligne suivante
		shr ecx, 4						; on transfert 16 bytes Dest � chaque boucle
		mov [Line2IntP], eax			; store second buffer addr
		mov edi, [esp + 28]				; edi = Dest
		mov [esp + 32], ebx				; on stocke compl�ment offset pour ligne suivante
		mov [esp + 36], ecx				; on stocke cette nouvelle valeur pour X
		mov ebp, [Line1IntP]			; ebp = First Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation

		test byte [Mode_555], 1			; set good mask for current video mode
		movq mm7, [MASK_DIV2_15]
		jnz short .First_Copy

		movq mm7, [MASK_DIV2_16]
		jmp short .First_Copy

	ALIGN4

	.First_Copy
	.Loop_X_FL
			movq mm0, [esi]
			add ebp, byte 16
			movq mm2, mm0
			movq mm1, [esi + 2]
			psrlw mm0, 1
			psrlw mm1, 1
			pand mm0, mm7
			pand mm1, mm7
			movq mm3, mm2
			paddw mm0, mm1
			add esi, byte 8
			punpcklwd mm2, mm0
			punpckhwd mm3, mm0
			movq [ebp + 0 - 16], mm2
			dec ecx
			movq [ebp + 8 - 16], mm3
			jnz short .Loop_X_FL

		dec dword [esp + 40]			; une ligne en moins
		jz near .Last_Line

		mov ecx, [esp + 36]				; on transfert 16 octets Dest par it�ration...
		add esi, [esp + 44]				; on augmente la source pour pointer sur la prochaine ligne
		mov ebp, [Line2IntP]			; ebp = Second Line buffer
		mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
		jmp short .Loop_Y

	ALIGN64

	.Loop_Y
	.Loop_X1
				movq mm0, [esi]
				add ebp, byte 16
				movq mm2, mm0
				movq mm1, [esi + 2]
				psrlw mm0, 1
				psrlw mm1, 1
				pand mm0, mm7
				pand mm1, mm7
				movq mm3, mm2
				paddw mm0, mm1
				add esi, byte 8
				punpcklwd mm2, mm0
				punpckhwd mm3, mm0
				movq [ebp + 0 - 16], mm2
				dec ecx
				movq [ebp + 8 - 16], mm3
				jnz short .Loop_X1

			mov ecx, [esp + 36]				; Dest num bytes / 16
			add esi, [esp + 44]				; on augmente la source pour pointer sur la prochaine ligne
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			jmp short .Loop_X2

	ALIGN64

	.Loop_X2
				movq mm0, [ebx]
				add edi, byte 16
				movq mm1, [ebx + 8]
				add ebx, byte 16
				movq [edi + 0 - 16], mm0
				dec ecx
				movq [edi + 8 - 16], mm1
				jnz short .Loop_X2

			mov ecx, [esp + 36]				; Dest num bytes / 16
			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; ebx = First Line buffer
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			jmp short .Loop_X3

	ALIGN64

	.Loop_X3
				movq mm0, [ebx]
				add edi, byte 16
				movq mm1, [ebx + 8]
				psrlw mm0, 1
				movq mm2, [ebp]
				psrlw mm1, 1
				movq mm3, [ebp + 8]
				psrlw mm2, 1
				psrlw mm3, 1
				pand mm0, mm7
				pand mm1, mm7
				pand mm2, mm7
				pand mm3, mm7
				paddw mm0, mm2
				paddw mm1, mm3
				add ebx, byte 16
				movq [edi + 0 - 16], mm0
				add ebp, byte 16
				dec ecx
				movq [edi + 8 - 16], mm1
				jnz short .Loop_X3

			add edi, [esp + 32]				; Next Dest line
			mov ebx, [Line1IntP]			; Swap line buffer
			mov ebp, [Line2IntP]
			mov [Line2IntP], ebx
			mov [Line1IntP], ebp
			mov ecx, [esp + 36]				; on transfert 4 octects Dest par it�ration...
			dec dword [esp + 40]			; encore des lignes ?
			mov ebp, [Line2IntP]			; ebp = Second Line buffer
			mov word [esi + 320 * 2], 0		; clear last pixel for correct interpolation
			jnz near .Loop_Y

	.Last_Line
		mov ecx, [esp + 36]				; Dest num bytes / 16
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		jmp short .Loop_X1_LL

	ALIGN4

	.Loop_X1_LL
			movq mm0, [ebx]
			add edi, byte 16
			movq mm1, [ebx + 8]
			add ebx, byte 16
			movq [edi + 0 - 16], mm0
			dec ecx
			movq [edi + 8 - 16], mm1
			jnz short .Loop_X1_LL

		add edi, [esp + 32]				; Next Dest line
		mov ecx, [esp + 36]				; Dest num bytes / 16
		mov ebx, [Line1IntP]			; ebx = First Line buffer
		jmp short .Loop_X2_LL

	ALIGN4

	.Loop_X2_LL
			movq mm0, [ebx]
			add edi, byte 16
			movq mm1, [ebx + 8]
			psrlw mm0, 1
			add ebx, byte 16
			psrlw mm1, 1
			pand mm0, mm7
			pand mm1, mm7
			movq [edi + 0 - 16], mm0
			dec ecx
			movq [edi + 8 - 16], mm1
			jnz short .Loop_X2_LL

		pop ebp
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret
