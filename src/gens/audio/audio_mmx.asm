%include "nasmhead.inc"

section .text align=64

	ALIGN32

	; void writeSoundMono_MMX(int *left, int *right, short *dest, int length)
	DECL writeSoundMono_MMX

		push ebx
		push ecx
		push edi
		push esi

		mov edi, [esp + 20]			; Left
		mov esi, [esp + 24]			; Right
		mov ecx, [esp + 32]			; Length
		mov ebx, [esp + 28]			; Dest

		shr ecx, 1
		jnc short .Double_Trans

	.Simple_Trans:
		mov eax, [edi]
		mov dword [edi], 0
		add eax, [esi]
		mov dword [esi], 0

		cmp eax, 0xFFFF
		jle short .lower_s

		mov word [ebx], 0x7FFF
		jmp short .ok_s

	.lower_s:
		cmp eax, -0xFFFF
		jge short .greater_s

		mov word [ebx], -0x7FFF
		jmp short .ok_s

	.greater_s:
		shr eax, 1
		mov [ebx], ax

	.ok_s:
		add edi, 4
		add esi, 4
		add ebx, 2

	.Double_Trans:
		mov eax, 32
		pxor mm4, mm4
		movd mm5, eax
		mov eax, 1
		test ecx, ecx
		movd mm6, eax
		jnz short .Loop
		jmp short .End

	ALIGN32

	.Loop:
		movq mm0, [edi]			; L2 | L1
		add ebx, 4
		movq [edi], mm4
		movq mm1, [esi]			; R2 | R1
		add edi, 8
		movq [esi], mm4
		packssdw mm0, mm0		; 0 | 0 | L2 | L1
		packssdw mm1, mm1		; 0 | 0 | R2 | R1
		psraw mm0, 1
		psraw mm1, 1
		add esi, 8
		paddw mm0, mm1			; 0 | 0 | R2 + L2 | R1 + L1
		dec ecx
		movd [ebx - 4], mm0
		jnz short .Loop

		emms

	.End:
		pop esi
		pop edi
		pop ecx
		pop ebx
		ret

	ALIGN32

	; void writeSoundStereo_MMX(int *left, int *right, short *dest, int length)
	DECL writeSoundStereo_MMX

		push ebx
		push ecx
		push edx
		push edi
		push esi

		mov edi, [esp + 24]			; Left
		mov esi, [esp + 28]			; Right
		mov ecx, [esp + 36]			; Length
		mov ebx, [esp + 32]			; Dest

		shr ecx, 1
		jnc short .Double_Trans

	.Simple_Trans:
		mov eax, [edi]
		cmp eax, 0x7FFF
		mov dword [edi], 0
		jle short .lower_s1

		mov word [ebx + 0], 0x7FFF
		jmp short .right_s1

	.lower_s1:
		cmp eax, -0x7FFF
		jge short .greater_s1

		mov word [ebx + 0], -0x7FFF
		jmp short .right_s1

	.greater_s1:
		mov [ebx + 0], ax

	.right_s1:
		mov edx, [esi]
		cmp edx, 0x7FFF
		mov dword [esi], 0
		jle short .lower_s2

		mov word [ebx + 2], 0x7FFF
		jmp short .ok_s1

	.lower_s2:
		cmp edx, -0x7FFF
		jge short .greater_s2

		mov word [ebx + 2], -0x7FFF
		jmp short .ok_s1

	.greater_s2:
		mov [ebx + 2], dx
		
	.ok_s1:
		add edi, 4
		add esi, 4
		add ebx, 4

	.Double_Trans:
		mov eax, 32
		pxor mm4, mm4
		test ecx, ecx
		movd mm5, eax
		jnz short .Loop
		jmp short .End

	ALIGN32

	.Loop:
		movd mm0, [esi]			; 0  | R1
		add edi, 8
		movd mm1, [esi + 4]		; 0  | R2
		psllq mm0, mm5			; R1 |  0
		movq [esi], mm4
		psllq mm1, mm5			; R2 |  0
		movd mm2, [edi - 8]		; 0  | L1
		add esi, 8
		movd mm3, [edi - 8 + 4]	; 0  | L2
		add ebx, 8
		paddd mm0, mm2			; R1 | L1
		paddd mm1, mm3			; R2 | L2
		movq [edi - 8], mm4
		packssdw mm0, mm1		; R2 | L2 | R1 | L1

		dec ecx
		movq [ebx - 8], mm0
		jnz short .Loop

		emms

	.End:
		pop esi
		pop edi
		pop edx
		pop ecx
		pop ebx
		ret
