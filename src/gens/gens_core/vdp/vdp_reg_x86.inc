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
	
struc VDP_Reg_t
	.Set_1:			resb 1
	.Set_2:			resb 1
	.Pat_ScrA_Adr:		resb 1
	.Pat_Win_Adr:		resb 1
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
	
	; DMAT variables.
	.DMAT_Tmp:		resd 1
	.DMAT_Length:		resd 1
	.DMAT_Type:		resd 1
	
	; VDP address pointers.
	; These are relative to VRam[] and are based on register values.
	.ScrA_Addr:		resd 1
	.ScrB_Addr:		resd 1
	.Win_Addr:		resd 1
	.Spr_Addr:		resd 1
	.H_Scroll_Addr:		resd 1
	
	; VDP convenience values: Horizontal.
	; NOTE: These must be signed for VDP arithmetic to work properly!
	.H_Cell:		resd 1
	.H_Win_Mul:		resd 1
	.H_Pix:			resd 1
	.H_Pix_Begin:		resd 1
	
	; VDP convenience values: Scroll.
	.V_Scroll_MMask:	resd 1
	.H_Scroll_Mask:		resd 1
	
	.H_Scroll_CMul:		resd 1
	.H_Scroll_CMask:	resd 1
	.V_Scroll_CMask:	resd 1
	
	; TODO: Eliminate these.
	.Win_X_Pos:		resd 1
	.Win_Y_Pos:		resd 1
endstruc

struc VDP_Ctrl_t
	.Flag:		resd 1
	.Data:		resd 1
	.Write:		resd 1
	.Access:	resd 1
	.Address:	resd 1
	.DMA_Mode:	resd 1
	.DMA:		resd 1
endstruc
