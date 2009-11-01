;
; Gens: Debugger - SegaCD Cell Dump Functions.
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008 by David Korth
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

section .data align=64
	
	extern SYM(MD_Screen)
	extern SYM(MD_Palette)
	extern SYM(MD_Screen32)
	extern SYM(MD_Palette32)
	
	; MD bpp
	extern SYM(bppMD)
	
section .text align=64

%macro AFF_PIXEL 2
	mov	eax, ebx				; eax = data pixels
	shr	eax, %2					; keep the first
	and	eax, 0xF
	mov	ax, [SYM(MD_Palette) + eax * 2 + ebp]	; conversion 8->16 bits palette
	mov	[edi + (%1 * 2)], ax			; write the pixel to Dest
%endmacro

; TODO: Gens Rerecording had additional jumps here...
%macro AFF_PIXEL32 2
	mov	eax, ebx				; eax = data pixels
	shr	eax, %2					; keep the first
	and	eax, 0xF
	mov	eax, [SYM(MD_Palette32) + eax * 4 + ebp]	; conversion 8->32 bits palette
	mov	[edi + (%1 * 4)], eax			; write the pixel to Dest
%endmacro


;void Cell_8x8_Dump(unsigned char *Adr, int Palette)
global SYM(Cell_8x8_Dump)
SYM(Cell_8x8_Dump):
	
	push ebx
	push ecx
	push edx
	push edi
	push esi
	push ebp
	
	xor	eax, eax				; eax = 0
	mov	ebp, [esp + 32]				; ebp = palette number
	mov	edx, 20					; edx = Number of rows of the pattern to be copied
	mov	esi, [esp + 28]				; esi = Address
	
	; Check if 32-bit color is in use.
	cmp	byte [SYM(bppMD)], 32
	je	._32BIT
	
	shl	ebp, 5					; ebp = palette number * 32
	lea	edi, [SYM(MD_Screen) + 6780]		; edi = MD_Screen + copy offset

.Loop_EDX:
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX:
	mov	ebx, 8					; ebx = Number of rows in each pattern

.Loop_EBX:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL 0, 12
	AFF_PIXEL 1, 8
	AFF_PIXEL 2, 4
	AFF_PIXEL 3, 0
	AFF_PIXEL 4, 28
	AFF_PIXEL 5, 24
	AFF_PIXEL 6, 20
	AFF_PIXEL 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 2				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX				; then keep going

	sub	edi, ((336 * 8) - 8) * 2		; we skip 8 rows from the top and 8 pixels from the left of Dest
	dec	ecx					; sif there is more to copy on this row
	jnz	near .Loop_ECX				; then keep going

	add 	edi, ((336 * 8) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	dec	edx
	jnz	near .Loop_EDX
	jmp	near .END

align 16

._32BIT:
	shl	ebp, 6
	lea	edi, [SYM(MD_Screen32) + 13560]		; edi = MD_Screen + copy offset

.Loop_EDX32:
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX32:
	mov	ebx, 8					; ebx = Number of rows in each pattern

.Loop_EBX32:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL32 0, 12
	AFF_PIXEL32 1, 8
	AFF_PIXEL32 2, 4
	AFF_PIXEL32 3, 0
	AFF_PIXEL32 4, 28
	AFF_PIXEL32 5, 24
	AFF_PIXEL32 6, 20
	AFF_PIXEL32 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 4				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX32			; then keep going
	
	sub	edi, ((336 * 8) - 8) * 4		; we skip 8 rows from the top and 8 pixels from the left of Dest
	dec	ecx					; if there is more to copy on this row
	jnz	near .Loop_ECX32			; then keep going
	
	add	edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	dec	edx
	jnz	near .Loop_EDX32

.END:
	pop	ebp
	pop	esi
	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	ret


align 64

;void Cell_16x16_Dump(unsigned char *Adr, int Palette)
global SYM(Cell_16x16_Dump)
SYM(Cell_16x16_Dump):
	
	push	ebx
	push	ecx
	push	edx
	push	edi
	push	esi
	push	ebp
	
	xor	eax, eax				; eax = 0
	mov	ebp, [esp + 32]				; ebp = palette_number
	mov	edx, 10					; edx = Number of rows of the pattern to be copied
	mov	esi, [esp + 28]				; esi = Address
	shl	ebp, 5					; ebp = palette_number * 32
	
	; Check if 32-bit color is in use.
	cmp	byte [SYM(bppMD)], 32
	je	._32BIT
	
	lea	edi, [SYM(MD_Screen)	+ 6780]		; edi = MD_Screen + copy offset

.Loop_EDX:
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX:
	mov	ebx, 8					; ebx = Number of rows in each pattern

.Loop_EBX:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL 0, 12
	AFF_PIXEL 1, 8
	AFF_PIXEL 2, 4
	AFF_PIXEL 3, 0
	AFF_PIXEL 4, 28
	AFF_PIXEL 5, 24
	AFF_PIXEL 6, 20
	AFF_PIXEL 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 2				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX				; then keep going
	
	sub	edi, ((336 * 8) - 8) * 2		; we skip 8 rows from the top and 8 pixels from the left of Dest
	add	esi, 0x20				; pattern à droite
	dec	ecx					; if there is more to copy on this row
	jnz	near .Loop_ECX				; then keep going
	
	add	edi, ((336 * 8) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	sub	esi, 0x400 - 0x20
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX_2:
	mov	ebx, 8					; ebx = Number of rows in each pattern

.Loop_EBX_2:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL 0, 12
	AFF_PIXEL 1, 8
	AFF_PIXEL 2, 4
	AFF_PIXEL 3, 0
	AFF_PIXEL 4, 28
	AFF_PIXEL 5, 24
	AFF_PIXEL 6, 20
	AFF_PIXEL 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 2				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX_2			; then keep going
	
	sub	edi, ((336 * 8) - 8) * 2		; we skip 8 rows from the top and 8 pixels from the left of Dest
	add	esi, 0x20				; pattern à droite
	dec	ecx					; if there is more to copy on this row
	jnz	near .Loop_ECX_2			; then keep going
	
	add	edi, ((336 * 8) - (8 * 16)) * 2		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	sub	esi, 0x20
	dec	edx
	jnz	near .Loop_EDX
	jmp	near .END

align 16

._32BIT:
	shl	ebp, 1
	lea	edi, [SYM(MD_Screen32) + 13560]		; edi = MD_Screen + copy offset

.Loop_EDX32:
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX32:
	mov	ebx, 8					; ebx = Number of rows in each pattern

.Loop_EBX32:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL32 0, 12
	AFF_PIXEL32 1, 8
	AFF_PIXEL32 2, 4
	AFF_PIXEL32 3, 0
	AFF_PIXEL32 4, 28
	AFF_PIXEL32 5, 24
	AFF_PIXEL32 6, 20
	AFF_PIXEL32 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 4				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX32			; then keep going
	
	sub	edi, ((336 * 8) - 8) * 4		; we skip 8 rows from the top and 8 pixels from the left of Dest
	add	esi, 0x20				; pattern à droite
	dec	ecx					; if there is more to copy on this row
	jnz	near .Loop_ECX32			; then keep going
	
	add	edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	sub	esi, 0x400 - 0x20
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX_232:
	mov	ebx, 8					; ebx = Number of rows in each pattern

.Loop_EBX_232:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL32 0, 12
	AFF_PIXEL32 1, 8
	AFF_PIXEL32 2, 4
	AFF_PIXEL32 3, 0
	AFF_PIXEL32 4, 28
	AFF_PIXEL32 5, 24
	AFF_PIXEL32 6, 20
	AFF_PIXEL32 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 4				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX_232			; then keep going
	
	sub	edi, ((336 * 8) - 8) * 4		; we skip 8 rows from the top and 8 pixels from the left of Dest
	add	esi, 0x20				; pattern à droite
	dec	ecx					; if there is more to copy on this row
	jnz	near .Loop_ECX_232			; then keep going
	
	add	edi, ((336 * 8) - (8 * 16)) * 4		; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	sub	esi, 0x20
	dec	edx
	jnz	near .Loop_EDX32

.END:
	pop	ebp
	pop	esi
	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	ret


align 64

;void Cell_32x32_Dump(unsigned char *Adr, int Palette)
global SYM(Cell_32x32_Dump)
SYM(Cell_32x32_Dump):
	
	push	ebx
	push	ecx
	push	edx
	push	edi
	push	esi
	push	ebp
	
	xor	eax, eax				; eax = 0
	mov	ebp, [esp + 32]				; ebp = palette_number
	mov	edx, 6					; edx = Number of rows of the pattern to be copied
	shl	ebp, 5					; ebp = palette_number * 32
	mov	esi, [esp + 28]				; esi = Address
	
	; Check if 32-bit color is in use.
	cmp	byte [SYM(bppMD)], 32
	je	._32BIT
	
	lea	edi, [SYM(MD_Screen)	+ 6780]		; edi = MD_Screen + copy offset

.Loop_EDX:
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX:
	mov	ebx, 32					; ebx = Number of rows in each pattern

.Loop_EBX:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL 0, 12
	AFF_PIXEL 1, 8
	AFF_PIXEL 2, 4
	AFF_PIXEL 3, 0
	AFF_PIXEL 4, 28
	AFF_PIXEL 5, 24
	AFF_PIXEL 6, 20
	AFF_PIXEL 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 2				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX				; then keep going
	
	sub	edi, ((336 * 32) - 8) * 2		; we skip 8 rows from the top and 8 pixels from the left of Dest
	dec	ecx					; if there is more to copy on this row
	jnz	near .Loop_ECX				; then keep going
	
	add	edi, ((336 * 32) - (8 * 16)) * 2	; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	dec	edx
	jnz	near .Loop_EDX
	jmp	near .END

align 16

._32BIT:
	shl	ebp, 1
	lea	edi, [SYM(MD_Screen32) + 13560 ]		; edi = MD_Screen + copy offset

.Loop_EDX32:
	mov	ecx, 16					; ecx = Number of patterns per row

.Loop_ECX32:
	mov	ebx, 32					; ebx = Number of rows in each pattern

.Loop_EBX32:
	push	ebx
	mov	ebx, [esi]
	AFF_PIXEL32 0, 12
	AFF_PIXEL32 1, 8
	AFF_PIXEL32 2, 4
	AFF_PIXEL32 3, 0
	AFF_PIXEL32 4, 28
	AFF_PIXEL32 5, 24
	AFF_PIXEL32 6, 20
	AFF_PIXEL32 7, 16
	pop	ebx
	add	esi, 4					; advance Src by 4
	add	edi, 336 * 4				; go to the next Dest row
	dec	ebx					; if there are any more rows
	jnz	near .Loop_EBX32			; then keep going
	
	sub	edi, ((336 * 32) - 8) * 4		; we skip 8 rows from the top and 8 pixels from the left of Dest
	dec	ecx					; if there is more to copy on this row
	jnz	near .Loop_ECX32			; then keep going
	
	add	edi, ((336 * 32) - (8 * 16)) * 4	; we skip 8 rows from the top and 16*8 pixels from the left of Dest
	dec	edx
	jnz	near .Loop_EDX32

.END:
	pop	ebp
	pop	esi
	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	ret

