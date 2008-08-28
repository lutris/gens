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
	
	;*************************************************************************
	;void Blit_2xSAI_MMX(unsigned char *Dest, int pitch, int x, int y, int offset)
	DECL Blit_2xSAI_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov ecx, [esp + 36]				; ecx = Nombre de lignes
		mov edx, [esp + 32]				; width
		mov ebx, [esp + 28]				; ebx = pitch de la surface Dest
		lea esi, [MD_Screen + 8 * 2]	; esi = Source
		mov edi, [esp + 24]				; edi = Destination
		test byte [Have_MMX], 0xFF		; on teste la pr�sence du MMX
		jz near .End

		sub esp, byte 4 * 5				; 5 params for _2xSaILine
		test byte [Mode_555], 1
		mov [esp], esi					; 1st Param = *Src
		mov [esp + 4], dword (336 * 2)	; 2nd Param = SrcPitch
		mov [esp + 8], edx				; 3rd Param = width
		mov [esp + 12], edi				; 4th Param = *Dest
		mov [esp + 16], ebx				; 5th Param = DestPitch
		jz short .Loop

		movq mm0, [colorMask15]
		movq mm1, [lowPixelMask15]
		movq [colorMask], mm0
		movq [lowPixelMask], mm1
		movq mm0, [qcolorMask15]
		movq mm1, [qlowpixelMask15]
		movq [qcolorMask], mm0
		movq [qlowpixelMask], mm1
		movq mm0, [darkenMask15]
		movq mm1, [GreenMask15]
		movq mm2, [RedBlueMask15]
		movq [darkenMask], mm0
		movq [GreenMask], mm1
		movq [RedBlueMask], mm2
		jmp short .Loop

	ALIGN64

	.Loop
			mov word [esi + 320 * 2], 0		; clear clipping

			call _2xSaILine					; Do one line

			add esi, 336 * 2				; esi = *Src + 1 line
			lea edi, [edi + ebx * 2]		; edi = *Dest + 2 lines
			mov [esp], esi					; 1st Param = *Src
			dec ecx
			mov [esp + 12], edi				; 4th Param = *Dest
			jnz short .Loop

		add esp, byte 4 * 5					; Free 5 params

	.End
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		emms
		ret


	ALIGN64
	
	;***********************************************************************************************
	;void _2xSaILine(uint8 *srcPtr, uint32 srcPitch, uint32 width, uint8 *dstPtr, uint32 dstPitch);
	_2xSaILine:

		push ebp
		mov ebp, esp
		pushad

		mov edx, [ebp+dstOffset]		; edx points to the screen

		mov eax, [ebp+srcPtr]			; eax points to colorA
		mov ebx, [ebp+srcPitch]
		mov ecx, [ebp+width]
		
		sub eax, ebx					; eax now points to colorE

		pxor mm0, mm0
		movq [LineBuffer], mm0
		jmp short .Loop

	ALIGN64
	
	.Loop:
			push ecx

		;1	------------------------------------------

		;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorE) && (colorB == colorL)

			movq mm0, [eax+ebx+colorA]        ;mm0 and mm1 contain colorA
			movq mm2, [eax+ebx+colorB]        ;mm2 and mm3 contain colorB

			movq mm1, mm0
			movq mm3, mm2

			pcmpeqw mm0, [eax+ebx+ebx+colorD]
			pcmpeqw mm1, [eax+colorE]
			pcmpeqw mm2, [eax+ebx+ebx+colorL]
			pcmpeqw mm3, [eax+ebx+ebx+colorC]

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorA == colorC) && (colorB != colorE) && (colorA == colorF) && (colorB == colorJ)

			movq mm4, [eax+ebx+colorA]        ;mm4 and mm5 contain colorA
			movq mm6, [eax+ebx+colorB]        ;mm6 and mm7 contain colorB
			movq mm5, mm4
			movq mm7, mm6

			pcmpeqw mm4, [eax+ebx+ebx+colorC]
			pcmpeqw mm5, [eax+colorF]
			pcmpeqw mm6, [eax+colorJ]
			pcmpeqw mm7, [eax+colorE]

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask1], mm0

		;2	-------------------------------------------

         ;if ((colorB == colorC) && (colorA != colorD) && (colorB == colorF) && (colorA == colorH)

			movq mm0, [eax+ebx+colorB]        ;mm0 and mm1 contain colorB
			movq mm2, [eax+ebx+colorA]        ;mm2 and mm3 contain colorA
			movq mm1, mm0
			movq mm3, mm2

			pcmpeqw mm0, [eax+ebx+ebx+colorC]
			pcmpeqw mm1, [eax+colorF]
			pcmpeqw mm2, [eax+ebx+ebx+colorH]
			pcmpeqw mm3, [eax+ebx+ebx+colorD]

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)

			movq mm4, [eax+ebx+colorB]        ;mm4 and mm5 contain colorB
			movq mm6, [eax+ebx+colorA]        ;mm6 and mm7 contain colorA
			movq mm5, mm4
			movq mm7, mm6

			pcmpeqw mm4, [eax+ebx+ebx+colorD]
			pcmpeqw mm5, [eax+colorE]
			pcmpeqw mm6, [eax+colorI]
			pcmpeqw mm7, [eax+colorF]

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask2], mm0


		;interpolate colorA and colorB

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+colorB]

			movq mm2, mm0
			movq mm3, mm1

			pand mm0, [colorMask]
			pand mm1, [colorMask]

			psrlw mm0, 1
			psrlw mm1, 1

			pand mm3, [lowPixelMask]
			paddw mm0, mm1

			pand mm3, mm2
			paddw mm0, mm3                ;mm0 contains the interpolated values

		;assemble the pixels

			movq mm1, [eax+ebx+colorA]
			movq mm2, [eax+ebx+colorB]

			movq mm3, [Mask1]
			movq mm5, mm1
			movq mm4, [Mask2]
			movq mm6, mm1

			pand mm1, mm3
			por mm3, mm4
			pxor mm7, mm7
			pand mm2, mm4

			pcmpeqw mm3, mm7
			por mm1, mm2
			pand mm0, mm3

			por mm0, mm1

			punpcklwd mm5, mm0
			punpckhwd mm6, mm0
;			movq mm0, [eax+ebx+colorA+8] ;Only the first pixel is needed

			movq [edx], mm5
			movq [edx+8], mm6

		;3 Create the Nextline  -------------------

		;if ((colorA == colorD) && (colorB != colorC) && (colorA == colorG) && (colorC == colorO)

			movq mm0, [eax+ebx+colorA]			;mm0 and mm1 contain colorA
			movq mm2, [eax+ebx+ebx+colorC]		;mm2 and mm3 contain colorC
			movq mm1, mm0
			movq mm3, mm2

			push eax
			add eax, ebx
			pcmpeqw mm0, [eax+ebx+colorD]
			pcmpeqw mm1, [eax+colorG]
			pcmpeqw mm2, [eax+ebx+ebx+colorO]
			pcmpeqw mm3, [eax+colorB]
			pop eax

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorA == colorB) && (colorG != colorC) && (colorA == colorH) && (colorC == colorM)

			movq mm4, [eax+ebx+colorA]			;mm4 and mm5 contain colorA
			movq mm6, [eax+ebx+ebx+colorC]		;mm6 and mm7 contain colorC
			movq mm5, mm4
			movq mm7, mm6

			push eax
			add eax, ebx
			pcmpeqw mm4, [eax+ebx+colorH]
			pcmpeqw mm5, [eax+colorB]
			pcmpeqw mm6, [eax+ebx+ebx+colorM]
			pcmpeqw mm7, [eax+colorG]
			pop eax

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask1], mm0

		;4  ----------------------------------------

		;if ((colorB == colorC) && (colorA != colorD) && (colorC == colorH) && (colorA == colorF)

			movq mm0, [eax+ebx+ebx+colorC]		;mm0 and mm1 contain colorC
			movq mm2, [eax+ebx+colorA]			;mm2 and mm3 contain colorA
			movq mm1, mm0
			movq mm3, mm2

			pcmpeqw mm0, [eax+ebx+colorB]
			pcmpeqw mm1, [eax+ebx+ebx+colorH]
			pcmpeqw mm2, [eax+colorF]
			pcmpeqw mm3, [eax+ebx+ebx+colorD]

			pand mm0, mm1
			pxor mm1, mm1
			pand mm0, mm2
			pcmpeqw mm3, mm1
			pand mm0, mm3                 ;result in mm0

		;if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)

			movq mm4, [eax+ebx+ebx+colorC]		;mm4 and mm5 contain colorC
			movq mm6, [eax+ebx+colorA]			;mm6 and mm7 contain colorA
			movq mm5, mm4
			movq mm7, mm6

			pcmpeqw mm4, [eax+ebx+ebx+colorD]
			pcmpeqw mm5, [eax+ebx+colorG]
			pcmpeqw mm6, [eax+colorI]
			pcmpeqw mm7, [eax+ebx+ebx+colorH]

			pand mm4, mm5
			pxor mm5, mm5
			pand mm4, mm6
			pcmpeqw mm7, mm5
			pand mm4, mm7                 ;result in mm4

			por mm0, mm4                  ;combine the masks
			movq [Mask2], mm0

		;----------------------------------------------

		;interpolate colorA and colorC

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+ebx+colorC]

			movq mm2, mm0
			movq mm3, mm1

			pand mm0, [colorMask]
			pand mm1, [colorMask]

			psrlw mm0, 1
			psrlw mm1, 1

			pand mm3, [lowPixelMask]
			paddw mm0, mm1

			pand mm3, mm2
			paddw mm0, mm3                ;mm0 contains the interpolated values

		;-------------

		;assemble the pixels

			movq mm1, [eax+ebx+colorA]
			movq mm2, [eax+ebx+ebx+colorC]

			movq mm3, [Mask1]
			movq mm4, [Mask2]

			pand mm1, mm3
			pand mm2, mm4

			por mm3, mm4
			pxor mm7, mm7
			por mm1, mm2

			pcmpeqw mm3, mm7
			pand mm0, mm3
			por mm0, mm1
			movq [ACPixel], mm0

		;////////////////////////////////
		; Decide which "branch" to take
		;--------------------------------

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+colorB]
			movq mm6, mm0
			movq mm7, mm1
			pcmpeqw mm0, [eax+ebx+ebx+colorD]
			pcmpeqw mm1, [eax+ebx+ebx+colorC]
			pcmpeqw mm6, mm7

			movq mm2, mm0
			movq mm3, mm0

			pand mm0, mm1       ;colorA == colorD && colorB == colorC
			pxor mm7, mm7

			pcmpeqw mm2, mm7
			pand mm6, mm0
			pand mm2, mm1       ;colorA != colorD && colorB == colorC

			pcmpeqw mm1, mm7

			pand mm1, mm3       ;colorA == colorD && colorB != colorC
			pxor mm0, mm6
			por mm1, mm6
			movq mm7, mm0
			movq [Mask2], mm2
			packsswb mm7, mm7
			movq [Mask1], mm1

			movd ecx, mm7
			test ecx, ecx
			jz near .SKIP_GUESS

		;-------------------------------------
		; Map of the pixels:           I|E F|J
		;                              G|A B|K
		;                              H|C D|L
		;                              M|N O|P

			movq mm6, mm0
			movq mm4, [eax+ebx+colorA]
			movq mm5, [eax+ebx+colorB]
			pxor mm7, mm7
			pand mm6, [ONE]

			movq mm0, [eax+colorE]
			movq mm1, [eax+ebx+colorG]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			movq mm0, [eax+colorF]
			movq mm1, [eax+ebx+colorK]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			push eax
			add eax, ebx
			movq mm0, [eax+ebx+colorH]
			movq mm1, [eax+ebx+ebx+colorN]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			movq mm0, [eax+ebx+colorL]
			movq mm1, [eax+ebx+ebx+colorO]
			movq mm2, mm0
			movq mm3, mm1
			pcmpeqw mm0, mm4
			pcmpeqw mm1, mm4
			pcmpeqw mm2, mm5
			pcmpeqw mm3, mm5
			pand mm0, mm6
			pand mm1, mm6
			pand mm2, mm6
			pand mm3, mm6
			paddw mm0, mm1
			paddw mm2, mm3

			pxor mm3, mm3
			pcmpgtw mm0, mm6
			pcmpgtw mm2, mm6
			pcmpeqw mm0, mm3
			pcmpeqw mm2, mm3
			pand mm0, mm6
			pand mm2, mm6
			paddw mm7, mm0
			psubw mm7, mm2

			pop eax
			movq mm1, mm7
			pxor mm0, mm0
			pcmpgtw mm7, mm0
			pcmpgtw mm0, mm1

			por mm7, [Mask1]
			por mm1, [Mask2]
			movq [Mask1], mm7
			movq [Mask2], mm1

		.SKIP_GUESS:

		;----------------------------
		;interpolate A, B, C and D

			movq mm0, [eax+ebx+colorA]
			movq mm1, [eax+ebx+colorB]
			movq mm4, mm0
			movq mm2, [eax+ebx+ebx+colorC]
			movq mm5, mm1
			movq mm3, [qcolorMask]
			movq mm6, mm2
			movq mm7, [qlowpixelMask]

			pand mm0, mm3
			pand mm1, mm3
			pand mm2, mm3
			pand mm3, [eax+ebx+ebx+colorD]

			psrlw mm0, 2
			pand mm4, mm7
			psrlw mm1, 2
			pand mm5, mm7
			psrlw mm2, 2
			pand mm6, mm7
			psrlw mm3, 2
			pand mm7, [eax+ebx+ebx+colorD]

			paddw mm0, mm1
			paddw mm2, mm3

			paddw mm4, mm5
			paddw mm6, mm7

			paddw mm4, mm6
			paddw mm0, mm2
			psrlw mm4, 2
			pand mm4, [qlowpixelMask]
			paddw mm0, mm4      ;mm0 contains the interpolated value of A, B, C and D

		;assemble the pixels

			movq mm1, [Mask1]
			movq mm2, [Mask2]
			movq mm4, [eax+ebx+colorA]
			movq mm5, [eax+ebx+colorB]
			pand mm4, mm1
			pand mm5, mm2

			pxor mm7, mm7
			por mm1, mm2
			por mm4, mm5
			pcmpeqw mm1, mm7
			pand mm0, mm1
			por mm4, mm0        ;mm4 contains the diagonal pixels

			movq mm0, [ACPixel]
			movq mm1, mm0
			punpcklwd mm0, mm4
			punpckhwd mm1, mm4

			push edx
			add edx, [ebp+dstPitch]

			movq [edx], mm0
			movq [edx+8], mm1

			pop edx

			add edx, 16
			add eax, 8

			pop ecx
			sub ecx, 4
			cmp ecx, 0
			jg  near .Loop

	; Restore some stuff

		popad
		mov esp, ebp
		pop ebp
		emms
		ret
