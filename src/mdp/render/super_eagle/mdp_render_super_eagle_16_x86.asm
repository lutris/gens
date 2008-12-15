;
; Gens: [MDP] Super Eagle renderer. [16-bit color] (x86 asm version)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008 by David Korth
; 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian
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

; Symbol redefines for ELF.
%ifdef __OBJ_ELF
	%define	_mdp_render_super_eagle_16_x86_mmx	mdp_render_super_eagle_16_x86_mmx
%endif

srcPtr		equ 28+8
srcPitch	equ 28+12
width		equ 28+16
dstOffset	equ 28+20
dstPitch	equ 28+24
dstSegment	equ 28+28

colorI		equ -2
colorE		equ 0
colorF		equ 2
colorJ		equ 4

colorG		equ -2
colorA		equ 0
colorB		equ 2
colorK		equ 4

colorH		equ -2
colorC		equ 0
colorD		equ 2
colorL		equ 4

colorM		equ -2
colorN		equ 0
colorO		equ 2
colorP		equ 4

; Position-independent code macros.
%include "pic.inc"

section .data align=64
	
	; Current color masks.
	; Default value is 16-bit.
	
	colorMask:		dd 0xF7DEF7DE, 0xF7DEF7DE
	lowPixelMask:		dd 0x08210821, 0x08210821
	
	qcolorMask:		dd 0xE79CE79C, 0xE79CE79C
	qlowpixelMask:		dd 0x18631863, 0x18631863
	
	; Previous Mode 555 setting.
	PrevMode555:		dd 0x00000000
	
; Read-only data on Win32 uses the section name ".rdata".
%ifdef __OBJ_WIN32
	%define .rodata .rdata
%endif

section .rodata align=64

	; 15-bit color masks.
	
	colorMask15:		dd 0x7BDE7BDE, 0x7BDE7BDE
	lowPixelMask15:		dd 0x04210421, 0x04210421
	
	qcolorMask15:		dd 0x739C739C, 0x739C739C
	qlowpixelMask15:	dd 0x0C630C63, 0x0C630C63
	
	; 16-bit color masks.
	
	colorMask16:		dd 0xF7DEF7DE, 0xF7DEF7DE
	lowPixelMask16:		dd 0x08210821, 0x08210821
	
	qcolorMask16:		dd 0xE79CE79C, 0xE79CE79C
	qlowpixelMask16:	dd 0x18631863, 0x18631863
	
	; Constants.
	ONE:			dd 0x00010001, 0x00010001
	
section .bss align=64
	
	Mask1:		resb 8
	Mask2:		resb 8
	ACPixel:	resb 8
	
section .text align=64

arg_destScreen	equ 28+8
arg_mdScreen	equ 28+12
arg_destPitch	equ 28+16
arg_srcPitch	equ 28+20
arg_width	equ 28+24
arg_height	equ 28+28
arg_mode555	equ 28+32


;************************************************************************
; void mdp_render_super_eagle_16_x86_mmx(uint16_t *destScreen, uint16_t *mdScreen,
;					 int destPitch, int srcPitch,
;					 int width, int height, int mode555);
global _mdp_render_super_eagle_16_x86_mmx
_mdp_render_super_eagle_16_x86_mmx:
	
	ret
