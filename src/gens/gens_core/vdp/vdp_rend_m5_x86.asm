;
; Gens: VDP Rendering functions.
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

HIGH_B		equ 0x80
SHAD_B		equ 0x40
PRIO_B		equ 0x01
SPR_B		equ 0x20

HIGH_W		equ 0x8080
SHAD_W		equ 0x4040
NOSHAD_W	equ 0xBFBF
PRIO_W		equ 0x0100
SPR_W		equ 0x2000

SHAD_D		equ 0x40404040
NOSHAD_D	equ 0xBFBFBFBF

section .data align=64
	
	; VDP layer flags (ported from Gens Rerecording; original by Nitsuja)
	VDP_LAYER_SCROLLA_LOW		equ	(1 << 0)
	VDP_LAYER_SCROLLA_HIGH		equ	(1 << 1)
	VDP_LAYER_SCROLLA_SWAP		equ	(1 << 2)
	VDP_LAYER_SCROLLB_LOW		equ	(1 << 3)
	VDP_LAYER_SCROLLB_HIGH		equ	(1 << 4)
	VDP_LAYER_SCROLLB_SWAP		equ	(1 << 5)
	VDP_LAYER_SPRITE_LOW		equ	(1 << 6)
	VDP_LAYER_SPRITE_HIGH		equ	(1 << 7)
	VDP_LAYER_SPRITE_SWAP		equ	(1 << 8)
	VDP_LAYER_SPRITE_ALWAYSONTOP	equ	(1 << 9)
	VDP_LAYER_PALETTE_LOCK		equ	(1 << 10)
	
	; Default layer flags
	VDP_LAYER_DEFAULT		equ	VDP_LAYER_SCROLLA_LOW	| \
						VDP_LAYER_SCROLLA_HIGH	| \
						VDP_LAYER_SCROLLB_LOW	| \
						VDP_LAYER_SCROLLB_HIGH	| \
						VDP_LAYER_SPRITE_LOW	| \
						VDP_LAYER_SPRITE_HIGH
	
	; VDP_Layers: Active layers and layer settings.
	extern SYM(VDP_Layers)
	
section .rodata align=64
	
	extern SYM(TAB336)
	extern SYM(TAB320)
	
	Mask_N:
		dd 0xFFFFFFFF, 0xFFF0FFFF, 0xFF00FFFF, 0xF000FFFF
		dd 0x0000FFFF, 0x0000FFF0, 0x0000FF00, 0x0000F000
	
	Mask_F:
		dd 0xFFFFFFFF, 0xFFFF0FFF, 0xFFFF00FF, 0xFFFF000F
		dd 0xFFFF0000, 0x0FFF0000, 0x00FF0000, 0x000F0000
	
	
section .bss align=64
	
	extern SYM(VRam)
	extern SYM(CRam)
	extern SYM(VSRam)
	
%include "vdp_reg_x86.inc"
	extern SYM(VDP_Reg)
	
	extern SYM(ScrA_Addr)
	extern SYM(ScrB_Addr)
	extern SYM(Win_Addr)
	extern SYM(Spr_Addr)
	extern SYM(H_Scroll_Addr)
	extern SYM(H_Cell)
	extern SYM(H_Win_Mul)
	extern SYM(H_Pix)
	extern SYM(H_Pix_Begin)
	
	extern SYM(H_Scroll_Mask)
	extern SYM(H_Scroll_CMul)
	extern SYM(H_Scroll_CMask)
	extern SYM(V_Scroll_CMask)
	extern SYM(V_Scroll_MMask)
	
	extern SYM(Win_X_Pos)
	extern SYM(Win_Y_Pos)
	
	extern SYM(VDP_Status)
	extern SYM(VDP_Current_Line)
	
	; Flags.
	extern SYM(VDP_Flags)
	VDP_FLAG_VRAM			equ	(1 << 0)
	VDP_FLAG_VRAM_SPR		equ	(1 << 1)
	VDP_FLAG_CRAM			equ	(1 << 2)
	
	extern SYM(_32X_VDP_Ram)
	extern SYM(_32X_VDP)
	
	; MD bpp
	extern SYM(bppMD)
	
	struc vx
		.Mode:		resd 1
		.State:		resd 1
		.AF_Data:	resd 1
		.AF_St:		resd 1
		.AF_Len:	resd 1
	endstruc
	
	extern SYM(Sprite_Struct)
	extern SYM(Sprite_Visible)
	
	Data_Spr:
		.H_Min:		resd 1
		.H_Max:		resd 1
	
	alignb 32
	
%include "vdp/vdp_data_misc_x86.inc"
	extern SYM(VDP_Data_Misc)
	
	; SYM(Sprite_Over): If set, enforces the sprite limit.
	extern SYM(Sprite_Over)
	
	; Line Buffer from vdp_rend_m5.cpp.
	extern SYM(LineBuf)
	
section .text align=64
	
	extern SYM(VDP_Update_Palette)
	extern SYM(VDP_Update_Palette_HS)
	
	; C wrapper functions for templated C++ functions.
	extern SYM(Make_Sprite_Struct)
	extern SYM(Make_Sprite_Struct_Interlaced)
	extern SYM(Make_Sprite_Struct_Partial)
	extern SYM(Make_Sprite_Struct_Partial_Interlaced)
	
	extern SYM(Update_Mask_Sprite)
	extern SYM(Update_Mask_Sprite_Limit)
	
	extern SYM(Get_X_Offset_ScrollA)
	extern SYM(Get_X_Offset_ScrollB)
	
	extern SYM(Update_Y_Offset_ScrollA)
	extern SYM(Update_Y_Offset_ScrollB)
	extern SYM(Update_Y_Offset_ScrollA_Interlaced)
	extern SYM(Update_Y_Offset_ScrollB_Interlaced)
	
	extern SYM(Get_Pattern_Info_ScrollA)
	extern SYM(Get_Pattern_Info_ScrollB)
	
	extern SYM(Get_Pattern_Data)
	extern SYM(Get_Pattern_Data_Interlaced)
	
	extern SYM(PutPixel_P1)
	extern SYM(PutPixel_P1_HS)
	
	extern SYM(PutPixel_Sprite)
	extern SYM(PutPixel_Sprite_Prio)
	extern SYM(PutPixel_Sprite_HS)
	extern SYM(PutPixel_Sprite_Prio_HS)
	
	extern SYM(PutLine_P0_ScrollA)
	extern SYM(PutLine_P0_ScrollA_HS)
	extern SYM(PutLine_P0_ScrollB)
	extern SYM(PutLine_P0_ScrollB_HS)
	
	extern SYM(PutLine_P0_Flip_ScrollA)
	extern SYM(PutLine_P0_Flip_ScrollA_HS)
	extern SYM(PutLine_P0_Flip_ScrollB)
	extern SYM(PutLine_P0_Flip_ScrollB_HS)
	
	extern SYM(PutLine_P1_ScrollA)
	extern SYM(PutLine_P1_ScrollA_HS)
	extern SYM(PutLine_P1_ScrollB)
	extern SYM(PutLine_P1_ScrollB_HS)
	
	extern SYM(PutLine_P1_Flip_ScrollA)
	extern SYM(PutLine_P1_Flip_ScrollA_HS)
	extern SYM(PutLine_P1_Flip_ScrollB)
	extern SYM(PutLine_P1_Flip_ScrollB_HS)
	
;****************************************
; background layer graphics background graphics layer 1
; macro PUTPIXEL_P1
; param :
; %1 = pixel number
; %2 = mask to isolate the good pixel
; %3 = Shift
; %4 = Shadow/Highlight enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 64

%macro PUTPIXEL_P1 4
	
	push	edx	; palette (not modified, so we can restore it later)
	push	ebx	; pattern
	push	%3	; shift
	push	%2	; mask
	push	%1	; pattern pixel number
	push	ebp	; display pixel number

%if %4 > 0
	; S/H
	call SYM(PutPixel_P1_HS)
%else
	; No S/H
	call SYM(PutPixel_P1)
%endif
	
	add	esp, byte 5*4
	pop	edx

%endmacro


;****************************************

; macro PUTPIXEL_SPRITE
; param :
; %1 = pixel number
; %2 = mask to isolate the good pixel
; %3 = Shift
; %4 = Priority
; %5 = Highlight/Shadow Enable
; takes :
; - ebx = Pattern Data
; - edx = Palette number * 16

%macro PUTPIXEL_SPRITE 5

	push	edx	; palette (not modified, so we can restore it later)
	push	ebx	; pattern
	push	%3	; shift
	push	%2	; mask
	push	%1	; pattern pixel number
	push	ebp	; display pixel number
	
%if %4 > 0
	; High Priority
	%if %5 > 0
		; S/H
		call SYM(PutPixel_Sprite_Prio_HS)
	%else
		; No S/H
		call SYM(PutPixel_Sprite_Prio)
	%endif
%else
	; Regular Priority
	%if %5 > 0
		; S/H
		call SYM(PutPixel_Sprite_HS)
	%else
		; No S/H
		call SYM(PutPixel_Sprite)
	%endif
%endif
	
	add	esp, byte 5*4
	pop	edx
	
	; Save the return value.
	or	ch, al

%endmacro


;****************************************

; macro PUTLINE_P0
; param :
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_P0 2

	push	edx	; palette (not modified, so we can restore it later)
	push	ebx	; pattern
	push	ebp	; display pixel number
	
%if %1 > 0
	; Scroll A
	%if %2 > 0
		; S/H
		call SYM(PutLine_P0_ScrollA_HS)
	%else
		; No S/H
		call SYM(PutLine_P0_ScrollA)
	%endif
%else
	; Scroll B
	%if %2 > 0
		; S/H
		call SYM(PutLine_P0_ScrollB_HS)
	%else
		; No S/H
		call SYM(PutLine_P0_ScrollB)
	%endif
%endif
	
	add	esp, byte 2*4
	pop	edx

%endmacro


;****************************************

; macro PUTLINE_FLIP_P0
; param :
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_FLIP_P0 2

	push	edx	; palette (not modified, so we can restore it later)
	push	ebx	; pattern
	push	ebp	; display pixel number
	
%if %1 > 0
	; Scroll A
	%if %2 > 0
		; S/H
		call SYM(PutLine_P0_Flip_ScrollA_HS)
	%else
		; No S/H
		call SYM(PutLine_P0_Flip_ScrollA)
	%endif
%else
	; Scroll B
	%if %2 > 0
		; S/H
		call SYM(PutLine_P0_Flip_ScrollB_HS)
	%else
		; No S/H
		call SYM(PutLine_P0_Flip_ScrollB)
	%endif
%endif
	
	add	esp, byte 2*4
	pop	edx

%endmacro


;****************************************

; macro PUTLINE_P1
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_P1 2

	push	edx	; palette (not modified, so we can restore it later)
	push	ebx	; pattern
	push	ebp	; display pixel number
	
%if %1 > 0
	; Scroll A
	%if %2 > 0
		; S/H
		call SYM(PutLine_P1_ScrollA_HS)
	%else
		; No S/H
		call SYM(PutLine_P1_ScrollA)
	%endif
%else
	; Scroll B
	%if %2 > 0
		; S/H
		call SYM(PutLine_P1_ScrollB_HS)
	%else
		; No S/H
		call SYM(PutLine_P1_ScrollB)
	%endif
%endif
	
	add	esp, byte 2*4
	pop	edx
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_FLIP_P1
; %1 = 0 for scroll B and one if not
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_FLIP_P1 2

	push	edx	; palette (not modified, so we can restore it later)
	push	ebx	; pattern
	push	ebp	; display pixel number
	
%if %1 > 0
	; Scroll A
	%if %2 > 0
		; S/H
		call SYM(PutLine_P1_Flip_ScrollA_HS)
	%else
		; No S/H
		call SYM(PutLine_P1_Flip_ScrollA)
	%endif
%else
	; Scroll B
	%if %2 > 0
		; S/H
		call SYM(PutLine_P1_Flip_ScrollB_HS)
	%else
		; No S/H
		call SYM(PutLine_P1_Flip_ScrollB)
	%endif
%endif
	
	add	esp, byte 2*4
	pop	edx
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_SPRITE
; param :
; %1 = Priority
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest mais sans le screen

%macro PUTLINE_SPRITE 2
	
%if %1 > 0
	; If Sprite High is disabled, don't do anything.
	test	dword [SYM(VDP_Layers)], VDP_LAYER_SPRITE_HIGH
	jz	near %%Full_Trans
%else
	; If Sprite Low is disabled, don't do anything.
	test	dword [SYM(VDP_Layers)], VDP_LAYER_SPRITE_LOW
	jz	near %%Full_Trans
%endif
	
	xor	ecx, ecx
	add	ebp, [esp]
	
	PUTPIXEL_SPRITE 0, 0x0000f000, 12, %1, %2
	PUTPIXEL_SPRITE 1, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 2, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 3, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 4, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 5, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 6, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 7, 0x000f0000, 16, %1, %2
	
	and	ch, 0x20
	sub	ebp, [esp]
	or	byte [SYM(VDP_Status)], ch
	
%%Full_Trans

%endmacro


;****************************************

; macro PUTLINE_SPRITE_FLIP
; param :
; %1 = Priority
; %2 = Highlight/Shadow enable
; entree :
; - ebx = Pattern Data
; - ebp point on dest

%macro PUTLINE_SPRITE_FLIP 2
	
%if %1 > 0
	; If Sprite High is disabled, don't do anything.
	test	dword [SYM(VDP_Layers)], VDP_LAYER_SPRITE_HIGH
	jz	near %%Full_Trans
%else
	; If Sprite Low is disabled, don't do anything.
	test	dword [SYM(VDP_Layers)], VDP_LAYER_SPRITE_LOW
	jz	near %%Full_Trans
%endif
	
	xor	ecx, ecx
	add	ebp, [esp]
	
	PUTPIXEL_SPRITE 0, 0x000f0000, 16, %1, %2
	PUTPIXEL_SPRITE 1, 0x00f00000, 20, %1, %2
	PUTPIXEL_SPRITE 2, 0x0f000000, 24, %1, %2
	PUTPIXEL_SPRITE 3, 0xf0000000, 28, %1, %2
	PUTPIXEL_SPRITE 4, 0x0000000f,  0, %1, %2
	PUTPIXEL_SPRITE 5, 0x000000f0,  4, %1, %2
	PUTPIXEL_SPRITE 6, 0x00000f00,  8, %1, %2
	PUTPIXEL_SPRITE 7, 0x0000f000, 12, %1, %2
	
	and	ch, 0x20
	sub	ebp, [esp]
	or	byte [SYM(VDP_Status)], ch
	
%%Full_Trans

%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_B
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 if V-Scroll mode in 2 cell and 0 if full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_B 3
	
	mov	ebp, [esp]			; ebp point on surface where one renders
	
	call	SYM(Get_X_Offset_ScrollB)
	mov	esi, eax
	
	;mov	eax, esi			; eax = scroll X inv
	xor	esi, 0x3FF			; esi = scroll X norm
	and	eax, byte 7			; eax = completion for offset
	shr	esi, 3				; esi = current cell
	add	ebp, eax			; ebp updated for clipping
	mov	ebx, esi
	and	esi, [SYM(H_Scroll_CMask)]		; prevent H Cell Offset from overflowing
	and	ebx, byte 1
	mov	eax, [SYM(H_Cell)]
	sub	ebx, byte 2			; start with cell -2 or -1 (for V Scroll)
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.X], eax		; number of cells to post
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Cell], ebx	; Current cell for the V Scroll
	
	mov	edi, [SYM(VDP_Current_Line)]	; edi = line number
	mov	eax, [SYM(VSRam) + 2]
	
%if %1 > 0
	shr	eax, 1				; divide Y scroll in 2 if interlaced
%endif
	
	add	edi, eax
	mov	eax, edi
	shr	edi, 3				; V Cell Offset
	and	eax, byte 7			; adjust for pattern
	and	edi, [SYM(V_Scroll_CMask)]		; prevent V Cell Offset from overflowing
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Line_7], eax
	
	jmp	short %%First_Loop
	
	align 16
	
	%%Loop
	
%if %2 > 0
		push	edi
		%if %1 > 0
			call SYM(Update_Y_Offset_ScrollB_Interlaced)
		%else
			call SYM(Update_Y_Offset_ScrollB)
		%endif
		add	esp, byte 4
		mov	edi, eax
%endif
	
	%%First_Loop
		
		push	edi	; Y tile number.
		push	esi	; X tile number.
		call	SYM(Get_Pattern_Info_ScrollB)
		add	esp, byte 8
		
		push	eax
%if %1 > 0
		call	SYM(Get_Pattern_Data_Interlaced)
%else
		call	SYM(Get_Pattern_Data)
%endif
		mov	ebx, eax	; Save the pattern data.
		pop	eax		; Restore the pattern info. ("pattern" isn't modified in Get_Pattern_Data().)
		mov	edx, eax	; Get the palette number. (PalNum * 16: >> 13, << 4: >> 9 total.)
		shr	edx, 9		
		and	edx, byte 0x30
		
		; Check for swapped Scroll B priority.
		test	dword [SYM(VDP_Layers)], VDP_LAYER_SCROLLB_SWAP
		jz	short %%No_Swap_ScrollB_Priority
		xor	ax, 0x8000
		
	%%No_Swap_ScrollB_Priority
		test	eax, 0x0800		; test if H-Flip?
		jz	near %%No_H_Flip	; if yes, then
		
	%%H_Flip
			
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%H_Flip_P1
			
	%%H_Flip_P0
				PUTLINE_FLIP_P0 0, %3
				jmp	%%End_Loop
				
				align 16
				
	%%H_Flip_P1
				PUTLINE_FLIP_P1 0, %3
				jmp	%%End_Loop
				
				align 16
				
	%%No_H_Flip
			
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%No_H_Flip_P1
			
	%%No_H_Flip_P0
				PUTLINE_P0 0, %3
				jmp 	%%End_Loop
				
				align 16
				
	%%No_H_Flip_P1
				PUTLINE_P1 0, %3
				jmp	short %%End_Loop
				
				align 16
				
	%%End_Loop
		inc	dword [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Cell]	; Next H cell for the V Scroll
		inc	esi							; Next H cell
		add	ebp, byte 8						; advance to the next pattern
		and	esi, [SYM(H_Scroll_CMask)]				; prevent H Offset from overflowing
		dec	byte [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.X]		; decrement number of cells to treat
		jns	near %%Loop
		
%%End


%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_A_WIN
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 si V-Scroll mode en 2 cell et 0 si full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_A_WIN 3
	
	mov	eax, [SYM(VDP_Current_Line)]
	movzx	ecx, byte [SYM(VDP_Reg) + VDP_Reg_t.Win_V_Pos]
	shr	eax, 3
	mov	ebx, [SYM(H_Cell)]
	shr	cl, 7				; cl = 1 si window at bottom
	cmp	eax, [SYM(Win_Y_Pos)]
	setae	ch				; ch = 1 si current line >= pos Y window
	xor	cl, ch				; cl = 0 si line window sinon line Scroll A
	jz	near %%Full_Win
	
	test	byte [SYM(VDP_Reg) + VDP_Reg_t.Win_H_Pos], 0x80
	mov	edx, [SYM(Win_X_Pos)]
	jz	short %%Win_Left
	
%%Win_Right
	sub	ebx, edx
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Start_W], edx	; Start Win (Cell)
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Length_W], ebx	; Length Win (Cell)
	dec	edx							; 1 cell en moins car on affiche toujours le dernier à part
	mov	dword [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Start_A], 0	; Start Scroll A (Cell)
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Length_A], edx	; Length Scroll A (Cell)
	jns	short %%Scroll_A
	jmp	%%Window
	
	align 16
	
%%Win_Left
	sub	ebx, edx
	mov	dword [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Start_W], 0	; Start Win (Cell)
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Length_W], edx	; Length Win (Cell)
	dec	ebx							; 1 cell en moins car on affiche toujours le dernier à part
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Start_A], edx	; Start Scroll A (Cell)
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Length_A], ebx	; Length Scroll A (Cell)
	jns	short %%Scroll_A
	jmp	%%Window
	
	align 16
	
%%Scroll_A
	mov	ebp, [esp]			; ebp point on surface where one renders
	
	call	SYM(Get_X_Offset_ScrollA)
	mov	esi, eax
	
	;mov	eax, esi			; eax = scroll X inv
	mov	ebx, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Start_A]	; Premier Cell
	xor	esi, 0x3FF			; esi = scroll X norm
	and	eax, byte 7			; eax = completion pour offset
	shr	esi, 3				; esi = cell courant (début scroll A)
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Mask], eax	; mask pour le dernier pattern
	mov	ecx, esi			; ecx = cell courant (début scroll A) 
	add	esi, ebx			; esi = cell courant ajusté pour window clip
	and	ecx, byte 1
	lea	eax, [eax + ebx * 8]		; clipping + window clip
	sub	ecx, byte 2			; on démarre au cell -2 ou -1 (pour le V Scroll)
	and	esi, [SYM(H_Scroll_CMask)]		; on empeche H Cell Offset de deborder
	add	ebp, eax			; ebp mis à jour pour clipping + window clip
	add	ebx, ecx			; ebx = Cell courant pour le V Scroll
	
	mov	edi, [SYM(VDP_Current_Line)]				; edi = line number
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Cell], ebx	; Cell courant pour le V Scroll
	jns	short %%Not_First_Cell
	
	mov	eax, [SYM(VSRam) + 0]
	jmp	short %%First_VScroll_OK
	
%%Not_First_Cell
	and	ebx, [SYM(V_Scroll_MMask)]
	mov	eax, [SYM(VSRam) + ebx * 2]
	
%%First_VScroll_OK
	
%if %1 > 0
	shr	 eax, 1				; divide Y scroll in 2 if interlaced
%endif
	
	add	edi, eax
	mov	eax, edi
	shr	edi, 3				; V Cell Offset
	and	eax, byte 7			; adjust for pattern
	and	edi, [SYM(V_Scroll_CMask)]		; prevent V Cell Offset from overflowing
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Line_7], eax
	
	jmp	short %%First_Loop_SCA
	
	align 16
	
%%Loop_SCA

%if %2 > 0
		push	edi
		%if %1 > 0
			call SYM(Update_Y_Offset_ScrollA_Interlaced)
		%else
			call SYM(Update_Y_Offset_ScrollA)
		%endif
		add	esp, byte 4
		mov	edi, eax
%endif

%%First_Loop_SCA
		
		push	edi	; Y tile number.
		push	esi	; X tile number.
		call	SYM(Get_Pattern_Info_ScrollA)
		add	esp, byte 8
		
		push	eax
%if %1 > 0
		call	SYM(Get_Pattern_Data_Interlaced)
%else
		call	SYM(Get_Pattern_Data)
%endif
		mov	ebx, eax	; Save the pattern data.
		pop	eax		; Restore the pattern info. ("pattern" isn't modified in Get_Pattern_Data().)
		mov	edx, eax	; Get the palette number. (PalNum * 16: >> 13, << 4: >> 9 total.)
		shr	edx, 9		
		and	edx, byte 0x30
		
		; Check for swapped Scroll A priority.
		test	dword [SYM(VDP_Layers)], VDP_LAYER_SCROLLA_SWAP
		jz	short %%No_Swap_ScrollA_Priority_1
		xor	ax, 0x8000
		
	%%No_Swap_ScrollA_Priority_1
		test	 eax, 0x0800		; test if H-Flip ?
		jz	near %%No_H_Flip	; if yes, then
		
	%%H_Flip
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%H_Flip_P1
			
	%%H_Flip_P0
				PUTLINE_FLIP_P0 1, %3
				jmp	%%End_Loop
				
				align 16
				
	%%H_Flip_P1
				PUTLINE_FLIP_P1 1, %3
				jmp	%%End_Loop
				
				align 16
				
	%%No_H_Flip
			test	eax, 0x8000		; test the priority of the current pattern
			jnz	near %%No_H_Flip_P1
			
	%%No_H_Flip_P0
				PUTLINE_P0 1, %3
				jmp	%%End_Loop
				
				align 16
				
	%%No_H_Flip_P1
				PUTLINE_P1 1, %3
				jmp	short %%End_Loop
				
				align 16
				
	%%End_Loop
		inc	dword [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Cell]	; Next H cell for the V Scroll
		inc	esi							; Next H cell
		add	ebp, byte 8						; advance to the next pattern
		and	esi, [SYM(H_Scroll_CMask)]				; prevent H Offset from overflowing
		dec	byte [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Length_A]	; decrement number of cells to treat for Scroll A
		jns	near %%Loop_SCA

%%LC_SCA

%if %2 > 0
	push	edi
	%if %1 > 0
		call SYM(Update_Y_Offset_ScrollA_Interlaced)
	%else
		call SYM(Update_Y_Offset_ScrollA)
	%endif
	add	esp, byte 4
	mov	edi, eax
%endif

	push	edi	; Y tile number.
	push	esi	; X tile number.
	call	SYM(Get_Pattern_Info_ScrollA)
	add	esp, byte 8
	
	push	eax
%if %1 > 0
	call	SYM(Get_Pattern_Data_Interlaced)
%else
	call	SYM(Get_Pattern_Data)
%endif
	mov	ebx, eax	; Save the pattern data.
	pop	eax		; Restore the pattern info. ("pattern" isn't modified in Get_Pattern_Data().)
	mov	edx, eax	; Get the palette number. (PalNum * 16: >> 13, << 4: >> 9 total.)
	shr	edx, 9		
	and	edx, byte 0x30
	
	; Check for swapped Scroll A priority.
	test	dword [SYM(VDP_Layers)], VDP_LAYER_SCROLLA_SWAP
	jz	short %%No_Swap_ScrollA_Priority_2
	xor	ax, 0x8000
	
%%No_Swap_ScrollA_Priority_2
	test	eax, 0x0800			; test if H-Flip ?
	mov	ecx, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Mask]
	jz	near %%LC_SCA_No_H_Flip		; if yes, then

	%%LC_SCA_H_Flip
		and	ebx, [Mask_F + ecx * 4]		; apply the mask
		test	eax, 0x8000			; test the priority of the current pattern
		jnz	near %%LC_SCA_H_Flip_P1
		
	%%LC_SCA_H_Flip_P0
			PUTLINE_FLIP_P0 1, %3
			jmp	%%LC_SCA_End
			
			align 16
			
	%%LC_SCA_H_Flip_P1
			PUTLINE_FLIP_P1 1, %3
			jmp	%%LC_SCA_End
			
			align 16
			
	%%LC_SCA_No_H_Flip
		and	ebx, [Mask_N + ecx * 4]		; apply the mask
		test	eax, 0x8000			; test the priority of the current pattern
		jnz	near %%LC_SCA_No_H_Flip_P1

	%%LC_SCA_No_H_Flip_P0
			PUTLINE_P0 1, %3
			jmp	%%LC_SCA_End
			
			align 16
			
	%%LC_SCA_No_H_Flip_P1
			PUTLINE_P1 1, %3
			jmp	short %%LC_SCA_End
			
			align 16
			
%%LC_SCA_End
	test	byte [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Length_W], 0xFF
	jnz	short %%Window
	jmp	%%End
	
	align 16
	
%%Full_Win
	xor	esi, esi	; Start Win (Cell)
	mov	edi, ebx	; Length Win (Cell)
	jmp	short %%Window_Initialised
	
	align 16
	
%%Window
	mov	esi, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Start_W]
	mov	edi, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Length_W]		; edi = # of cells to render
	
%%Window_Initialised
	mov	edx, [SYM(VDP_Current_Line)]
	mov	cl, [SYM(H_Win_Mul)]
	mov	ebx, edx				; ebx = Line
	mov	ebp, [esp]				; ebp point on surface where one renders
	shr	edx, 3					; edx = Line / 8
	mov	eax, [SYM(Win_Addr)]
	shl	edx, cl
	lea	ebp, [ebp + esi * 8 + 8]		; no clipping for the window, return directly to the first pixel
	lea	eax, [eax + edx * 2]			; eax point on the pattern data for the window
	and	ebx, byte 7				; ebx = Line & 7 for the V Flip
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Pattern_Adr], eax		; store this pointer
	mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Line_7], ebx		; store Line & 7
	jmp	short %%Loop_Win
	
	align 16
	
%%Loop_Win
		mov	ebx, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Pattern_Adr]
		mov	ax, [ebx + esi * 2]
		
		push	eax
%if %1 > 0
		call	SYM(Get_Pattern_Data_Interlaced)
%else
		call	SYM(Get_Pattern_Data)
%endif
		mov	ebx, eax	; Save the pattern data.
		pop	eax		; Restore the pattern info. ("pattern" isn't modified in Get_Pattern_Data().)
		mov	edx, eax	; Get the palette number. (PalNum * 16: >> 13, << 4: >> 9 total.)
		shr	edx, 9		
		and	edx, byte 0x30
		
		test	ax, 0x0800		; test if H-Flip ?
		jz	near %%W_No_H_Flip	; if yes, then
		
	%%W_H_Flip
			test	ax, 0x8000		; test the priority of the current pattern
			jnz	near %%W_H_Flip_P1
			
	%%W_H_Flip_P0
				PUTLINE_FLIP_P0 1, %3
				jmp	%%End_Loop_Win
				
				align 16
				
	%%W_H_Flip_P1
				PUTLINE_FLIP_P1 1, %3
				jmp	%%End_Loop_Win
				
				align 16
				
	%%W_No_H_Flip
			test	ax, 0x8000		; test the priority of the current pattern
			jnz	near %%W_No_H_Flip_P1
			
	%%W_No_H_Flip_P0
				PUTLINE_P0 1, %3
				jmp	%%End_Loop_Win
				
				align 16
				
	%%W_No_H_Flip_P1
				PUTLINE_P1 1, %3
				jmp	short %%End_Loop_Win
				
				align 16
				
	%%End_Loop_Win
		inc	esi			; next pattern
		add	ebp, byte 8		; next pattern for the render
		dec	edi
		jnz	near %%Loop_Win
		
%%End

%endmacro


;****************************************

; macro RENDER_LINE_SPR
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = Shadow / Highlight (0 = Disable and 1 = Enable)

%macro RENDER_LINE_SPR 2
	
	test	dword [SYM(Sprite_Over)], 1
	jz	near %%No_Sprite_Over
	
%%Sprite_Over
	
	call	SYM(Update_Mask_Sprite_Limit)	; edi point on the sprite to post
	mov	esi, eax
	
	xor	edi, edi
	test	esi, esi
	mov	dword [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.X], edi
	jnz	near %%Sprite_Loop
	jmp	%%End				; quit
	
%%No_Sprite_Over
	
	call	SYM(Update_Mask_Sprite)		; edi = point on the sprite to post
	mov	esi, eax
	
	xor	edi, edi
	test	esi, esi
	mov	dword [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.X], edi
	jnz	short %%Sprite_Loop
	jmp	%%End				; quit
	
	align 16
	
%%Sprite_Loop
		mov	edx, [SYM(VDP_Current_Line)]
		mov	edi, [SYM(Sprite_Visible) + edi]
		mov	eax, [SYM(Sprite_Struct) + edi + 24]		; eax = CellInfo of the sprite
		sub	edx, [SYM(Sprite_Struct) + edi + 4]			; edx = Line - Y Pos (Y Offset)
		mov	ebx, eax					; ebx = CellInfo
		mov	esi, eax					; esi = CellInfo
		
		; Check for swapped sprite priority.
		test	dword [SYM(VDP_Layers)], VDP_LAYER_SPRITE_SWAP
		jz	short %%No_Swap_Sprite_Priority
		xor	ax, 0x8000
		
	%%No_Swap_Sprite_Priority
		shr	bx, 9					; isolate the palette in ebx
		mov	ecx, edx				; ecx = Y Offset
		and	ebx, 0x30				; keep the palette number an even multiple of 16
		
		and	esi, 0x7FF						; esi = number of the first pattern of the sprite
		mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Palette], ebx	; store the palette number * 64 in Palette
		and	edx, 0xF8						; one erases the 3 least significant bits = Num Pattern * 8
		mov	ebx, [SYM(Sprite_Struct) + edi + 12]			; ebx = Size Y
		and	ecx, byte 7						; ecx = (Y Offset & 7) = Line of the current pattern
%if %1 > 0
		shl	ebx, 6					; ebx = Size Y * 64
		lea	edx, [edx * 8]				; edx = Num Pattern * 64
		shl	esi, 6					; esi = point on the contents of the pattern
%else
		shl	ebx, 5					; ebx = Size Y * 32
		lea	edx, [edx * 4]				; edx = Num Pattern * 32
		shl	esi, 5					; esi = point on the contents of the pattern
%endif
		
		test	eax, 0x1000				; test for V Flip
		jz	%%No_V_Flip
		
	%%V_Flip
		sub	ebx, edx
		xor	ecx, 7					; ecx = 7 - (Y Offset & 7)
		add	esi, ebx				; esi = point on the pattern to post
%if %1 > 0
		lea	ebx, [ebx + edx + 64]			; restore the value of ebx + 64
		lea	esi, [esi + ecx * 8]			; and load the good line of the pattern
		jmp	short %%Suite
%else
		lea	ebx, [ebx + edx + 32]			; restore the value of ebx + 64
		lea	esi, [esi + ecx * 4]			; and load the good line of the pattern
		jmp	short %%Suite
%endif
		
		align 16
		
	%%No_V_Flip
		add	esi, edx				; esi = point on the pattern to post
%if %1 > 0
		add	ebx, byte 64				; add 64 to ebx
		lea	esi, [esi + ecx * 8]			; and load the good line of the pattern
%else			
		add	ebx, byte 32				; add 32 to ebx
		lea	esi, [esi + ecx * 4]			; and load the good line of the pattern
%endif
		
	%%Suite
		mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Next_Cell], ebx	; next Cell X of this sprite is with ebx bytes
		mov	edx, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Palette]	; edx = Palette number * 64
		
		test	eax, 0x800				; test H Flip
		jz	near %%No_H_Flip
			
	%%H_Flip
		mov	ebx, [SYM(Sprite_Struct) + edi + 0]
		mov	ebp, [SYM(Sprite_Struct) + edi + 16]	; position for X
		cmp	ebx, -7					; test for the minimum edge of the sprite
		mov	edi, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Next_Cell]
		jg	short %%Spr_X_Min_Norm
		mov	ebx, -7					; minimum edge = clip screen
		
	%%Spr_X_Min_Norm
		mov	[Data_Spr.H_Min], ebx			; spr min = minimum edge
		
	%%Spr_X_Min_OK
		sub	ebp, byte 7				; to post the last pattern in first
		jmp	short %%Spr_Test_X_Max
		
		align 16
		
	%%Spr_Test_X_Max_Loop
			sub	ebp, byte 8			; one moves back on the preceding pattern (screen)
			add	esi, edi			; one goes on next the pattern (mem)
			
	%%Spr_Test_X_Max
			cmp	ebp, [SYM(H_Pix)]
			jge	%%Spr_Test_X_Max_Loop
		
		; Check if sprites should always be on top.
		test	dword [SYM(VDP_Layers)], VDP_LAYER_SPRITE_ALWAYSONTOP
		jnz	near %%H_Flip_P1
		
		test	eax, 0x8000				; test the priority
		jnz	near %%H_Flip_P1
		jmp	short %%H_Flip_P0
		
		align 16
		
	%%H_Flip_P0
	%%H_Flip_P0_Loop
			mov	ebx, [SYM(VRam) + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE_FLIP 0, %2		; one posts the line of the sprite pattern
			
			sub	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Min]		; test if one did all the sprite patterns
			jge	near %%H_Flip_P0_Loop		; if not, continue
		jmp	%%End_Sprite_Loop
		
		align 16
		
	%%H_Flip_P1
	%%H_Flip_P1_Loop
			mov	ebx, [SYM(VRam) + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE_FLIP 1, %2		; one posts the line of the sprite pattern
			
			sub	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Min]		; test if one did all the sprite patterns
			jge	near %%H_Flip_P1_Loop		; if not, continue
		jmp	%%End_Sprite_Loop
		
		align 16
		
	%%No_H_Flip
		mov	ebx, [SYM(Sprite_Struct) + edi + 16]
		mov	ecx, [SYM(H_Pix)]
		mov	ebp, [SYM(Sprite_Struct) + edi + 0]		; position the pointer ebp
		cmp	ebx, ecx				; test for the maximum edge of the sprite
		mov	edi, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Next_Cell]
		jl	%%Spr_X_Max_Norm
		mov	[Data_Spr.H_Max], ecx			; max edge = clip screan
		jmp	short %%Spr_Test_X_Min
		
		align 16
		
	%%Spr_X_Max_Norm
		mov	[Data_Spr.H_Max], ebx			; spr max = max edge
		jmp	short %%Spr_Test_X_Min
		
		align 16
		
	%%Spr_Test_X_Min_Loop
			add	ebp, byte 8			; advance to the next pattern (screen)
			add	esi, edi			; one goes on next the pattern (mem)
			
	%%Spr_Test_X_Min
			cmp	ebp, -7
			jl	%%Spr_Test_X_Min_Loop
		
		; Check if sprites should always be on top.
		test	dword [SYM(VDP_Layers)], VDP_LAYER_SPRITE_ALWAYSONTOP
		jnz	near %%No_H_Flip_P1
		
		test	ax, 0x8000				; test the priority
		jnz	near %%No_H_Flip_P1
		jmp	short %%No_H_Flip_P0
		
		align 16
		
	%%No_H_Flip_P0
	%%No_H_Flip_P0_Loop
			mov	ebx, [SYM(VRam) + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE 0, %2			; one posts the line of the sprite pattern 
			
			add	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Max]		; test if one did all the sprite patterns
			jl	near %%No_H_Flip_P0_Loop	; if not, continue
		jmp	%%End_Sprite_Loop
		
		align 16
		
	%%No_H_Flip_P1
	%%No_H_Flip_P1_Loop
			mov	ebx, [SYM(VRam) + esi]		; ebx = Pattern Data
			PUTLINE_SPRITE 1, %2			; on affiche la ligne du pattern sprite
			
			add	ebp, byte 8			; one posts the previous pattern
			add	esi, edi			; one goes on next the pattern
			cmp	ebp, [Data_Spr.H_Max]		; test if one did all the sprite patterns
			jl	near %%No_H_Flip_P1_Loop	; if not, continue
		jmp	short %%End_Sprite_Loop
		
		align 16
		
	%%End_Sprite_Loop
		mov	edi, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.X]
		add	edi, byte 4
		cmp	edi, [SYM(VDP_Data_Misc) + VDP_Data_Misc_t.Borne]
		mov	[SYM(VDP_Data_Misc) + VDP_Data_Misc_t.X], edi
		jb	near %%Sprite_Loop
		
%%End

%endmacro


;****************************************

; macro RENDER_LINE
; param :
; %1 = 1 for interlace mode and 0 if not
; %2 = Shadow / Highlight (0 = Disable et 1 = Enable)

%macro RENDER_LINE 2
	
	test	byte [SYM(VDP_Reg) + VDP_Reg_t.Set_3], 4
	jz	near %%Full_VScroll
	
%%Cell_VScroll
	RENDER_LINE_SCROLL_B     %1, 1, %2
	RENDER_LINE_SCROLL_A_WIN %1, 1, %2
	jmp	%%Scroll_OK
	
%%Full_VScroll
	RENDER_LINE_SCROLL_B     %1, 0, %2
	RENDER_LINE_SCROLL_A_WIN %1, 0, %2
	
%%Scroll_OK
	RENDER_LINE_SPR          %1, %2
	
%endmacro


; *******************************************************
	
	global SYM(VDP_Render_Line_m5_asm)
	SYM(VDP_Render_Line_m5_asm):
		
		pushad
		
		;mov	ebx, [SYM(VDP_Current_Line)]
		;xor	eax, eax
		;mov	edi, [SYM(TAB336) + ebx * 4]
		xor	edi, edi	; Rendering to linebuffer, so the line number is always 0.
		push	edi		; we need this value later
	
	.Sprite_Struc_OK:
		movzx	eax, byte [SYM(VDP_Reg) + VDP_Reg_t.Set_4]
		and	eax, byte 0xC
		jmp	[.Table_Render_Line + eax]
	
	align 16
	
	.Table_Render_Line:
		dd 	.NHS_NInterlace
		dd 	.NHS_Interlace
		dd 	.HS_NInterlace
		dd 	.HS_Interlace
		
	align 16
	
	.NHS_NInterlace:
			RENDER_LINE 0, 0
			jmp .VDP_OK
	
	align 16
	
	.NHS_Interlace:
			RENDER_LINE 1, 0
			jmp .VDP_OK
	
	align 16
	
	.HS_NInterlace:
			RENDER_LINE 0, 1
			jmp .VDP_OK
	
	align 16
	
	.HS_Interlace:
			RENDER_LINE 1, 1
			jmp short .VDP_OK
	
	align 16
	
	.VDP_OK:
		add	esp, byte 4	; Restore the stack pointer.
		popad
		ret
