;
; Gens: 32X VDP functions.
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008-2009 by David Korth
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

%include "mdp/mdp_nasm_x86.inc"

section .bss align=64
	
	extern SYM(MD_Screen)
	extern SYM(MD_Screen32)
	
	; MD bpp
	extern SYM(bppMD)
	
	; VDP RAM and CRam.
	extern SYM(_32X_VDP_Ram)
	extern SYM(_32X_VDP_CRam)
	
	; Full 32X palettes.
	extern SYM(_32X_Palette_16B)
	extern SYM(_32X_Palette_32B)
	
	; Adjusted CRam.
	extern SYM(_32X_VDP_CRam_Adjusted)
	extern SYM(_32X_VDP_CRam_Adjusted32)
	
	alignb 32
	
	global SYM(_32X_VDP)
	SYM(_32X_VDP):
		.Mode:		resd 1
		.State:		resd 1
		.AF_Data:	resd 1
		.AF_St:		resd 1
		.AF_Len:	resd 1
		.AF_Line:	resd 1
	
section .text align=64
	
	; void _32X_VDP_Draw(int FB_Num)
	global SYM(_32X_VDP_Draw)
	SYM(_32X_VDP_Draw):
		
		; FB_Num is stored in eax.
		; No frame pointer is used in this function.
		mov	eax, [esp + 4]
		pushad
		
		and	eax, byte 1
		shl	eax, 17
		xor	ebp, ebp
		lea	esi, [eax + SYM(_32X_VDP_Ram)]
		xor	ebx, ebx
		lea	esi, [eax + SYM(_32X_VDP_Ram)]
		
		; Check if 32-bit color mode is enabled.
		; If 32-bit is enabled, do 32-bit drawing.
		cmp	byte [SYM(bppMD)], 32
		je	near ._32BIT
	
	; 16-bit color
	._16BIT:
		lea	edi, [SYM(MD_Screen) + 8 * 2]
	
	.Loop_Y:
		mov	eax, [SYM(_32X_VDP.Mode)]
		mov	bx, [esi + ebp * 2]
		and	eax, byte 3
		mov	ecx, 160
		jmp	[.Table_32X_Draw + eax * 4]
	
	align 16
	
	.Table_32X_Draw:
		dd	._32X_Draw_M00, ._32X_Draw_M01, ._32X_Draw_M10, ._32X_Draw_M11
	
	align 16
	
	._32X_Draw_M10:
	._32X_Draw_M10_P:
			movzx	eax, word [esi + ebx * 2 + 0]
			movzx	edx, word [esi + ebx * 2 + 2]
			mov	ax, [SYM(_32X_Palette_16B) + eax * 2]
			mov	dx, [SYM(_32X_Palette_16B) + edx * 2]
			mov	[edi + 0], ax
			mov	[edi + 2], dx
			add	bx, byte 2
			add	edi, byte 4
			dec	ecx
			jnz	short ._32X_Draw_M10
			jmp	.End_Loop_Y
	
	._32X_Draw_M00_P:
	._32X_Draw_M00:
		popad
		ret
	
	align 16
	
	._32X_Draw_M01:
	._32X_Draw_M01_P:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	edx, byte [esi + ebx * 2 + 1]
			mov	ax, [SYM(_32X_VDP_CRam_Adjusted) + eax * 2]
			mov	dx, [SYM(_32X_VDP_CRam_Adjusted) + edx * 2]
			mov	[edi + 2], ax
			mov	[edi + 0], dx
			inc	bx
			add	edi, byte 4
			dec	ecx
			jnz	short ._32X_Draw_M01
			jmp	.End_Loop_Y
	
	align 16
	
	._32X_Draw_M11:
	._32X_Draw_M11_P:
			mov	edx, 320
			jmp	short ._32X_Draw_M11_Loop
	
	align 16
	
	._32X_Draw_M11_Loop:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	ecx, byte [esi + ebx * 2 + 1]
			mov	ax, [SYM(_32X_VDP_CRam_Adjusted) + eax * 2]
			inc	ecx
			inc	bx
			sub	edx, ecx
			jbe	short ._32X_Draw_M11_End
			rep	stosw
			jmp	short ._32X_Draw_M11_Loop
	
	align 16

	._32X_Draw_M11_End:
		add	ecx, edx
		rep	stosw

	.End_Loop_Y:
		inc	ebp
		add	edi, byte 8 * 2 * 2
		cmp	ebp, 220
		jb	near .Loop_Y
		
		lea	edi, [SYM(MD_Screen) + 8 * 2 + 336 * 2 * 220]
		xor	eax, eax
		mov	ecx, 128
	
	.Palette_Loop:
			mov	dx, [SYM(_32X_VDP_CRam_Adjusted) + eax * 2]
			mov	[edi + 0], dx
			mov	[edi + 2], dx
			mov	[edi + 336 * 2 + 0], dx
			mov	[edi + 336 * 2 + 2], dx
			mov	dx, [SYM(_32X_VDP_CRam_Adjusted) + eax * 2 + 128 * 2]
			mov	[edi + 336 * 4 + 0], dx
			mov	[edi + 336 * 4 + 2], dx
			mov	[edi + 336 * 6 + 0], dx
			mov	[edi + 336 * 6 + 2], dx
			add	edi, byte 4
			inc	eax
			dec	ecx
			jnz	short .Palette_Loop 
		
		popad
		ret
	
	; 32-bit color
	._32BIT:
		lea	edi, [SYM(MD_Screen32) + 8 * 4]

	.Loop_Y32:
		mov	eax, [SYM(_32X_VDP.Mode)]
		mov	bx, [esi + ebp * 2]
		and	eax, byte 3
		mov	ecx, 160
		jmp	[.Table_32X_Draw32 + eax * 4]
	
	align 16
	
	.Table_32X_Draw32:
		dd ._32X_Draw_M0032, ._32X_Draw_M0132, ._32X_Draw_M1032, ._32X_Draw_M1132
	
	align 16
	
	._32X_Draw_M1032:
	._32X_Draw_M10_P32:
			movzx	eax, word [esi + ebx * 2 + 0]
			movzx	edx, word [esi + ebx * 2 + 2]
			mov	eax, [SYM(_32X_Palette_32B) + eax * 4]
			mov	edx, [SYM(_32X_Palette_32B) + edx * 4]
			mov	[edi + 0], eax
			mov	[edi + 4], edx
			add	bx, byte 2
			add	edi, byte 8
			dec	ecx
			jnz	short ._32X_Draw_M1032
			jmp	.End_Loop_Y32
	
	._32X_Draw_M00_P32:
	._32X_Draw_M0032:
		popad
		ret
	
	align 16
	
	._32X_Draw_M0132:
	._32X_Draw_M01_P32:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	edx, byte [esi + ebx * 2 + 1]
			mov	eax, [SYM(_32X_VDP_CRam_Adjusted32) + eax * 4]
			mov	edx, [SYM(_32X_VDP_CRam_Adjusted32) + edx * 4]
			mov	[edi + 4], eax
			mov	[edi + 0], edx
			inc	bx
			add	edi, byte 8
			dec	ecx
			jnz	short ._32X_Draw_M0132
			jmp	.End_Loop_Y32
	
	align 16
	
	._32X_Draw_M1132:
	._32X_Draw_M11_P32:
			mov	edx, 320
			jmp	short ._32X_Draw_M11_Loop32
	
	align 16
	
	._32X_Draw_M11_Loop32:
			movzx	eax, byte [esi + ebx * 2 + 0]
			movzx	ecx, byte [esi + ebx * 2 + 1]
			mov	eax, [SYM(_32X_VDP_CRam_Adjusted32) + eax * 4]
			inc	ecx
			inc	bx
			sub	edx, ecx
			jbe	short ._32X_Draw_M11_End32
			rep	stosd
			jmp	short ._32X_Draw_M11_Loop32
	
	align 16
	
	._32X_Draw_M11_End32:
		add	ecx, edx
		rep	stosw
	
	.End_Loop_Y32:
		inc	ebp
		add	edi, byte 8 * 2 * 4
		cmp	ebp, 220
		jb	near .Loop_Y32
		
		lea	edi, [SYM(MD_Screen32) + 8 * 4 + 336 * 4 * 220]
		xor	eax, eax
		mov	ecx, 128
	
	.Palette_Loop32:
			mov	edx, [SYM(_32X_VDP_CRam_Adjusted32) + eax * 4]
			mov	[edi + 0], edx
			mov	[edi + 4], edx
			mov	[edi + 336 * 4 + 0], edx
			mov	[edi + 336 * 4 + 4], edx
			mov	edx, [SYM(_32X_VDP_CRam_Adjusted32) + eax * 4 + 128 * 4]
			mov	[edi + 336 * 8 + 0], edx
			mov	[edi + 336 * 8 + 4], edx
			mov	[edi + 336 * 12 + 0], edx
			mov	[edi + 336 * 12 + 4], edx
			add	edi, byte 8
			inc	eax
			dec	ecx
			jnz	short .Palette_Loop32
	
	.End32:
		popad
		ret

