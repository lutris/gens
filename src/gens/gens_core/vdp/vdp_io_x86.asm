;
; Gens: VDP I/O functions.
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

IN_VRAM		equ 0
IN_CRAM		equ 1
IN_VSRAM	equ 2
RD_Mode		equ 0
WR_Mode		equ 1

section .data align=64
	
	extern SYM(CD_Table)
	
	extern SYM(Genesis_Started)
	extern SYM(SegaCD_Started)
	extern SYM(_32X_Started)
	
section .bss align=64
	
	extern SYM(Rom_Data)
	extern SYM(Rom_Size)
	extern SYM(Sprite_Struct)
	extern SYM(Sprite_Visible)
	extern SYM(CPL_M68K)
	extern SYM(Cycles_M68K)
	extern SYM(main68k_context)	; Starscream context (for interrupts)
	
	; SegaCD
	extern SYM(Cell_Conv_Tab)
	extern SYM(Ram_Word_State)
	
	extern SYM(MD_Screen)
	extern SYM(MD_Palette)
	extern SYM(MD_Screen32)
	extern SYM(MD_Palette32)
	
	extern SYM(Ram_68k)
	extern SYM(Ram_Prg)
	extern SYM(Ram_Word_2M)
	extern SYM(Ram_Word_1M)
	extern SYM(Bank_M68K)
	
	extern SYM(VRam);
	extern SYM(CRam);
	
	global SYM(VSRam_Over)	; VSRam overflow buffer. DO NOT REMOVE!
	SYM(VSRam_Over):
		resd 8
	
	global SYM(VSRam)
	SYM(VSRam):
		resd 64
	
%include "vdp_reg_x86.inc"
	extern SYM(VDP_Reg)
	extern SYM(VDP_Ctrl)
	extern SYM(VDP_Lines)
	
	extern SYM(VDP_Status)
	extern SYM(VDP_Int)
	
	; Flags.
	extern SYM(VDP_Flags)
	VDP_FLAG_VRAM			equ	(1 << 0)
	VDP_FLAG_VRAM_SPR		equ	(1 << 1)
	VDP_FLAG_CRAM			equ	(1 << 2)
	
section .text align=64

	;void VDP_Do_DMA_asm(unsigned int src_address, unsigned int dest_address, int length, unsigned int auto_inc)
	global SYM(VDP_Do_DMA_COPY_asm)
	SYM(VDP_Do_DMA_COPY_asm):
		push	ecx
		push	esi
		push	edi
		push	edx
		
		mov	esi, [esp + 20]	; src_address: DMA source address / 2.
		mov	edi, [esp + 24]	; dest_address: Destination address.
		mov	ecx, [esp + 28]	; length: DMA length.
		mov	edx, [esp + 32]	; auto_inc: VDP auto-increment.
		
		or	word [SYM(VDP_Status)], 0x0002
		mov	dword [SYM(VDP_Reg) + VDP_Reg_t.DMA_Length], 0
		mov	dword [SYM(VDP_Reg) + VDP_Reg_t.DMAT_Length], ecx
		mov	dword [SYM(VDP_Reg) + VDP_Reg_t.DMAT_Type], 0x3
		or	dword [SYM(VDP_Flags)], VDP_FLAG_VRAM
		jmp	short .VRam_Copy_Loop
	
	align 16

	.VRam_Copy_Loop:
			mov	al, [SYM(VRam) + esi]		; ax = Src
			inc	si				; on augment pointeur Src de 1
			mov	[SYM(VRam) + edi], al		; VRam[Dest] = Src.W
			add	di, dx				; Adr = Adr + Auto_Inc
			dec	ecx				; un transfert de moins
			jnz	short .VRam_Copy_Loop		; si DMA Length >= 0 alors on continue le transfert DMA
		
		mov	[SYM(VDP_Reg) + VDP_Reg_t.DMA_Address], esi
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Address], di	; on stocke la nouvelle Data_Address
		
		pop	edx
		pop	edi
		pop	esi
		pop	ecx
		ret
	
	align 16
	
	.NO_DMA:
		mov	dword [SYM(VDP_Ctrl) + VDP_Ctrl_t.DMA], 0
		pop	edx
		pop	edi
		pop	esi
		pop	ecx
		ret
