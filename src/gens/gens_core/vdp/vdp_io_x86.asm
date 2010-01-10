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
	
	extern SYM(Zero_Length_DMA)
	
section .text align=64
	
	extern SYM(main68k_readOdometer)
	extern SYM(main68k_releaseCycles)
	extern SYM(main68k_interrupt)
	
	extern SYM(Write_To_68K_Space)
	
	; Functions found in vdp_io.c
	extern SYM(VDP_Set_Reg)
	extern SYM(VDP_Update_DMA)

; ******************************************

; macro DMA_LOOP
; entree:
; esi = Source Address
; edi = Destination Address
; ecx = Nombre de words a transferer
; edx = Incrementation Destination
; sortie:
; param :
; %1 = 0 : source = ROM, 1 : source = RAM    2 : source = Prog RAM
; %1 = 3 : source = Word RAM 2M              4 : incorrect value
; %1 = 5 : source = Word RAM 1M Bank 1       6 : source = Word RAM 1M Bank 0
; %1 = 7 : source = Cell arranged Bank 0     8 : source = Cell arranged Bank 1
; %2 = 0 : dest = VRAM, 1 : dest = CRAM, 2 : dest = VSRAM

%macro DMA_LOOP 2

%if %1 < 1
	and	esi, 0x003FFFFE
%elif %1 < 2
	and	esi, 0xFFFE
%elif %1 < 3
	and	esi, 0x0001FFFE
	add	esi, [SYM(Bank_M68K)]
%elif %1 < 4
	sub	esi, 2
	and	esi, 0x0003FFFE
%elif %1 < 7
	sub	esi, 2
	and	esi, 0x0001FFFE
%else
	sub	esi, 2		; cell rearranged
	xor	eax, eax	; for offset
	and	esi, 0x0001FFFE
%endif
	mov	ebx, edi
%if %2 < 1
	or	dword [SYM(VDP_Flags)], VDP_FLAG_VRAM
	mov	byte [SYM(VDP_Reg) + VDP_Reg_t.DMAT_Type], 0
%else
	%if %2 < 2
		or	dword [SYM(VDP_Flags)], VDP_FLAG_CRAM
	%endif
	mov	byte [SYM(VDP_Reg) + VDP_Reg_t.DMAT_Type], 1
%endif
	xor	edi, edi
	mov	dword [SYM(VDP_Ctrl) + VDP_Ctrl_t.DMA], 0
	jmp	short %%Loop
	
	align 16
	
; TODO: This causes garbage on TmEE's mmf.bin (correct),
; but the garbage doesn't match Kega Fusion.
%%Loop:
	mov	di, bx
%if %1 < 1
	mov	ax, [SYM(Rom_Data) + esi]
	add	si, 2		; Actual hardware wraps on 128K boundaries.
	jnc	short %%No_Carry
	xor	esi, 0x10000
%elif %1 < 2
	mov	ax, [SYM(Ram_68k) + esi]
	add	si, 2
%elif %1 < 3
	mov	ax, [SYM(Ram_Prg) + esi]
	add	si, 2		; Actual hardware wraps on 128K boundaries.
	jnc	short %%No_Carry
	xor	esi, 0x10000
%elif %1 < 4
	mov	ax, [SYM(Ram_Word_2M) + esi]
	add	si, 2		; Actual hardware wraps on 128K boundaries.
	jnc	short %%No_Carry
	xor	esi, 0x10000
%elif %1 < 6
	mov	ax, [SYM(Ram_Word_1M) + esi + 0x00000]
	add	si, 2		; Actual hardware wraps on 128K boundaries.
	jnc	short %%No_Carry
	xor	esi, 0x10000
%elif %1 < 7
	mov	ax, [SYM(Ram_Word_1M) + esi + 0x20000]
	add	si, 2		; Actual hardware wraps on 128K boundaries.
	jnc	short %%No_Carry
	xor	esi, 0x10000
%elif %1 < 8
	mov	ax, [SYM(Cell_Conv_Tab) + esi]
	add	si, 2		; Actual hardware wraps on 128K boundaries.
	jnc	short %%No_Carry_8
	xor	esi, 0x10000
%%No_Carry_8:
	mov	ax, [SYM(Ram_Word_1M) + eax * 2 + 0x00000]
%elif %1 < 9
	mov	ax, [SYM(Cell_Conv_Tab) + esi]
	add	si, 2		; Actual hardware wraps on 128K boundaries.
	jnc	short %%No_Carry_9
	xor	esi, 0x10000
%%No_Carry_9:
	mov	ax, [SYM(Ram_Word_1M) + eax * 2 + 0x20000]
%endif
%%No_Carry:
%if %2 < 1
	shr	di, 1
	jnc	short %%No_Swap
	rol	ax, 8
%%No_Swap:
%else
	and	di, byte 0x7E
%endif
	add	bx, dx
	dec	ecx
%if %2 < 1
	mov	[SYM(VRam) + edi * 2], ax
%elif %2 < 2
	mov	[SYM(CRam) + edi], ax
%else
	mov	[SYM(VSRam) + edi], ax
%endif
	jnz	short %%Loop
	
%%End_Loop:
	jmp .End_DMA
	
%endmacro

;***********************************************
	
	align 16
	
	; Generic error handler.
	error:
		xor	ax, ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	;void VDP_Do_DMA_asm(unsigned int access, unsigned int src_address, unsigned int dest_address,
	;		     int length, unsigned int auto_inc, int src_component)
	global SYM(VDP_Do_DMA_asm)
	SYM(VDP_Do_DMA_asm):
		push	ebx
		push	ecx
		push	edx
		push	edi
		push	esi
		
		mov	eax, [esp + 24]	; access: DMA access mode from CD_Table[].
		mov	esi, [esp + 28]	; src_address: DMA source address.
		mov	edi, [esp + 32]	; dest_address: Destination address.
		mov	ecx, [esp + 36]	; length: DMA length.
		mov	edx, [esp + 40]	; auto_inc: VDP auto-increment.
		mov	ebx, [esp + 44]	; src_component: DMA source component.
	
	.DMA_Src_OK:
		test	eax, 0x2				; Dest = CRAM or VSRAM
		lea	ebx, [ebx * 4 + eax]
		jz	short .DMA_Dest_OK
		and	edi, byte 0x7F
	
	.DMA_Dest_OK:
		or	word [SYM(VDP_Status)], 0x0002
		xor	eax, eax
		jmp	[.Table_DMA + ebx * 4]			; on effectue le transfert DMA adéquat
	
	align 16
	
	.Table_DMA:
		dd	.NO_DMA, .ROM_To_VRam_Star, .ROM_To_CRam_Star, .ROM_To_VSRam_Star
		dd	.NO_DMA, .RAM_To_VRam_Star, .RAM_To_CRam_Star, .RAM_To_VSRam_Star
		dd	.NO_DMA, .RAMPRG_To_VRam_Star, .RAMPRG_To_CRam_Star, .RAMPRG_To_VSRam_Star
		dd	.NO_DMA, .RAMWORD2M_To_VRam_Star, .RAMWORD2M_To_CRam_Star, .RAMWORD2M_To_VSRam_Star
		
		dd	.NO_DMA, .NO_DMA, .NO_DMA, .NO_DMA
		dd	.NO_DMA, .RAMWORD1M0_To_VRam_Star, .RAMWORD1M0_To_CRam_Star, .RAMWORD1M0_To_VSRam_Star
		dd	.NO_DMA, .RAMWORD1M1_To_VRam_Star, .RAMWORD1M1_To_CRam_Star, .RAMWORD1M1_To_VSRam_Star
		dd	.NO_DMA, .CELL0_To_VRam_Star, .CELL0_To_CRam_Star, .CELL0_To_VSRam_Star
		
		dd	.NO_DMA, .CELL1_To_VRam_Star, .CELL1_To_CRam_Star, .CELL1_To_VSRam_Star
		dd	.NO_DMA, .NO_DMA, .NO_DMA, .NO_DMA
		dd	.NO_DMA, .NO_DMA, .NO_DMA, .NO_DMA
		dd	.NO_DMA, .NO_DMA, .NO_DMA, .NO_DMA
	
	align 16
	
	.ROM_To_VRam_Star:
		DMA_LOOP 0, 0
	
	align 16
	
	.ROM_To_CRam_Star:
		DMA_LOOP 0, 1
	
	align 16
	
	.ROM_To_VSRam_Star:
		DMA_LOOP 0, 2
	
	align 16
	
	.RAM_To_VRam_Star:
		DMA_LOOP 1, 0
	
	align 16
	
	.RAM_To_CRam_Star:
		DMA_LOOP 1, 1
	
	align 16
	
	.RAM_To_VSRam_Star:
		DMA_LOOP 1, 2
	
	align 16
	
	.RAMPRG_To_VRam_Star:
		DMA_LOOP 2, 0
	
	align 16
	
	.RAMPRG_To_CRam_Star:
		DMA_LOOP 2, 1
	
	align 16
	
	.RAMPRG_To_VSRam_Star:
		DMA_LOOP 2, 2
	
	align 16
	
	.RAMWORD2M_To_VRam_Star:
		DMA_LOOP 3, 0
	
	align 16
	
	.RAMWORD2M_To_CRam_Star:
		DMA_LOOP 3, 1
	
	align 16
	
	.RAMWORD2M_To_VSRam_Star:
		DMA_LOOP 3, 2
	
	align 16
	
	.RAMWORD1M0_To_VRam_Star:
		DMA_LOOP 5, 0
	
	align 16
	
	.RAMWORD1M0_To_CRam_Star:
		DMA_LOOP 5, 1
	
	align 16
	
	.RAMWORD1M0_To_VSRam_Star:
		DMA_LOOP 5, 2
	
	align 16
	
	.RAMWORD1M1_To_VRam_Star:
		DMA_LOOP 6, 0
	
	align 16
	
	.RAMWORD1M1_To_CRam_Star:
		DMA_LOOP 6, 1
	
	align 16
	
	.RAMWORD1M1_To_VSRam_Star:
		DMA_LOOP 6, 2
	
	align 16
	
	.CELL0_To_VRam_Star:
		DMA_LOOP 7, 0
	
	align 16
	
	.CELL0_To_CRam_Star:
		DMA_LOOP 7, 1
	
	align 16
	
	.CELL0_To_VSRam_Star:
		DMA_LOOP 7, 2
	
	align 16
	
	.CELL1_To_VRam_Star:
		DMA_LOOP 8, 0
	
	align 16
	
	.CELL1_To_CRam_Star:
		DMA_LOOP 8, 1
	
	align 16
	
	.CELL1_To_VSRam_Star:
		DMA_LOOP 8, 2
	
	align 16
	
	.End_DMA:
		mov	eax, [SYM(VDP_Reg) + VDP_Reg_t.DMA_Length]
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Address], bx
		mov	esi, [SYM(VDP_Reg) + VDP_Reg_t.DMA_Address]
		sub	eax, ecx
		mov	[SYM(VDP_Reg) + VDP_Reg_t.DMA_Length], ecx
		lea	esi, [esi + eax]
		jbe	short .Nothing_To_Do
		
		and	esi, 0x7FFFFF
		mov	[SYM(VDP_Reg) + VDP_Reg_t.DMAT_Length], eax
		mov	[SYM(VDP_Reg) + VDP_Reg_t.DMA_Address], esi
		call	SYM(VDP_Update_DMA)
		call	SYM(main68k_releaseCycles)
		pop	esi
		pop	edi
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Nothing_To_Do:
		and	word [SYM(VDP_Status)], 0xFFFD
		pop	esi
		pop	edi
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.NO_DMA:
		mov	dword [SYM(VDP_Ctrl) + VDP_Ctrl_t.DMA], 0
		pop	esi
		pop	edi
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
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
