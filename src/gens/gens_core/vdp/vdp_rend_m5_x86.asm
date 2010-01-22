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
	extern SYM(VDP_Lines)
	
	extern SYM(VDP_Status)
	
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
	extern SYM(Render_Line_ScrollB)
	extern SYM(Render_Line_ScrollB_Interlaced)
	extern SYM(Render_Line_ScrollB_VScroll)
	extern SYM(Render_Line_ScrollB_VScroll_Interlaced)
	extern SYM(Render_Line_ScrollB_HS)
	extern SYM(Render_Line_ScrollB_HS_Interlaced)
	extern SYM(Render_Line_ScrollB_HS_VScroll)
	extern SYM(Render_Line_ScrollB_HS_VScroll_Interlaced)
	
	extern SYM(Render_Line_ScrollA)
	extern SYM(Render_Line_ScrollA_Interlaced)
	extern SYM(Render_Line_ScrollA_VScroll)
	extern SYM(Render_Line_ScrollA_VScroll_Interlaced)
	extern SYM(Render_Line_ScrollA_HS)
	extern SYM(Render_Line_ScrollA_HS_Interlaced)
	extern SYM(Render_Line_ScrollA_HS_VScroll)
	extern SYM(Render_Line_ScrollA_HS_VScroll_Interlaced)
	
	extern SYM(Render_Line_Sprite)
	extern SYM(Render_Line_Sprite_Interlaced)
	extern SYM(Render_Line_Sprite_HS)
	extern SYM(Render_Line_Sprite_HS_Interlaced)
	

;****************************************

; macro RENDER_LINE_SCROLL_B
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 if V-Scroll mode in 2 cell and 0 if full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_B 3

%if %1 > 0
	; Interlaced.
	%if %2 > 0
		; VScroll is 2 cell.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollB_HS_VScroll_Interlaced)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollB_VScroll_Interlaced)
		%endif
	%else
		; VScroll is Full.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollB_HS_Interlaced)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollB_Interlaced)
		%endif
	%endif
%else
	; Not Interlaced.
	%if %2 > 0
		; VScroll is 2 cell.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollB_HS_VScroll)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollB_VScroll)
		%endif
	%else
		; VScroll is Full.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollB_HS)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollB)
		%endif
	%endif
%endif

%endmacro


;****************************************

; macro RENDER_LINE_SCROLL_A_WIN
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = 1 if V-Scroll mode in 2 cell and 0 if full scroll
; %3 = Highlight/Shadow enable

%macro RENDER_LINE_SCROLL_A_WIN 3

%if %1 > 0
	; Interlaced.
	%if %2 > 0
		; VScroll is 2 cell.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollA_HS_VScroll_Interlaced)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollA_VScroll_Interlaced)
		%endif
	%else
		; VScroll is Full.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollA_HS_Interlaced)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollA_Interlaced)
		%endif
	%endif
%else
	; Not Interlaced.
	%if %2 > 0
		; VScroll is 2 cell.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollA_HS_VScroll)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollA_VScroll)
		%endif
	%else
		; VScroll is Full.
		%if %3 > 0
			; Highlight/Shadow.
			call SYM(Render_Line_ScrollA_HS)
		%else
			; No Highlight/Shadow.
			call SYM(Render_Line_ScrollA)
		%endif
	%endif
%endif

%endmacro


;****************************************

; macro RENDER_LINE_SPR
; param :
; %1 = 1 for interlace mode and 0 for normal mode
; %2 = Shadow / Highlight (0 = Disable and 1 = Enable)

%macro RENDER_LINE_SPR 2

%if %1 > 0
	; Interlaced.
	%if %2 > 0
		; Highlight/Shadow.
		call SYM(Render_Line_Sprite_HS_Interlaced)
	%else
		; No Highlight/Shadow.
		call SYM(Render_Line_Sprite_Interlaced)
	%endif
%else
	; Not Interlaced.
	%if %2 > 0
		; Highlight/Shadow.
		call SYM(Render_Line_Sprite_HS)
	%else
		; No Highlight/Shadow.
		call SYM(Render_Line_Sprite)
	%endif
%endif

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
		
		;mov	ebx, [SYM(VDP_Lines) + VDP_Lines_t.Visible_Current]
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
