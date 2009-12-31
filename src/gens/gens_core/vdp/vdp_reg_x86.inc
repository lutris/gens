;
; Gens: VDP Register Structure. (x86 asm)
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
	
struc Reg_VDP_Type
	.Set_1:			resb 1
	.Set_2:			resb 1
	.Pat_ScrA_Adr:		resb 1
	.Pat_WIN_Adr:		resb 1
	.Pat_ScrB_Adr:		resb 1
	.Spr_Att_Adr:		resb 1
	.Reg6:			resb 1
	.BG_Color:		resb 1
	.Reg8:			resb 1
	.Reg9:			resb 1
	.H_Int_Reg:		resb 1
	.Set_3:			resb 1
	.Set_4:			resb 1
	.H_Scr_Adr:		resb 1
	.Reg14:			resb 1
	.Auto_Inc:		resb 1
	.Scr_Size:		resb 1
	.Win_H_Pos:		resb 1
	.Win_V_Pos:		resb 1
	.DMA_Length_L:		resb 1
	.DMA_Length_H:		resb 1
	.DMA_Src_Adr_L:		resb 1
	.DMA_Src_Adr_M:		resb 1
	.DMA_Src_Adr_H:		resb 1
	
	.DMA_Length:		resd 1
	.DMA_Address:		resd 1
endstruc
