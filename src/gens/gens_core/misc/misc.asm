;
; Gens: Miscellaneous assembly language functions.
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008-2010 by David Korth
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
	
	extern SYM(CDD.Control)
	extern SYM(CDD.Rcv_Status)
	extern SYM(CDD.Status)
	extern SYM(CDD.Minute)
	extern SYM(CDD.Seconde)
	extern SYM(CDD.Frame)
	extern SYM(CDD.Ext)
	
%include "vdp/vdp_reg_x86.inc"
	extern SYM(VDP_Reg)
	
	; MD bpp
	extern SYM(bppMD)
	
	
section .text align=64
	
	; void CDD_Export_Status(void)
	global SYM(CDD_Export_Status)
	SYM(CDD_Export_Status):
		push	ebx
		push	ecx
		
		mov	ax, [SYM(CDD.Status)]
		mov	bx, [SYM(CDD.Minute)]
		mov	cx, [SYM(CDD.Seconde)]
		mov	[SYM(CDD.Rcv_Status) + 0], ax
		mov	[SYM(CDD.Rcv_Status) + 2], bx
		mov	[SYM(CDD.Rcv_Status) + 4], cx
		add	al, bl
		add	al, bh
		mov	bx, [SYM(CDD.Frame)]
		add	al, ch
		add	al, cl
		and	byte [SYM(CDD.Control)], 0x3
		add	al, ah
		add	al, bl
		mov	ah, [SYM(CDD.Ext)]
		add	al, bh
		add	al, ah
		mov	[SYM(CDD.Rcv_Status) + 6], bx
		not	al
		and	al, 0x0F
		mov	[SYM(CDD.Rcv_Status) + 8], ax
		
		pop	ecx
		pop	ebx
		ret
