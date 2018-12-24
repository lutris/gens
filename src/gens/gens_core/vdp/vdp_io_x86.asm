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
	extern SYM(DMA_Timing_Table)
	
	extern SYM(Genesis_Started)
	extern SYM(SegaCD_Started)
	extern SYM(_32X_Started)
	
	extern SYM(Size_V_Scroll)
	extern SYM(H_Scroll_Mask_Table)
	
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

	global SYM(VRam)
	SYM(VRam):
		resb 64 * 1024
	
	global SYM(CRam)
	SYM(CRam):
		resw 64
	
	global SYM(VSRam_Over)	; VSRam overflow buffer. DO NOT REMOVE!
	SYM(VSRam_Over):
		resd 8
	
	global SYM(VSRam)
	SYM(VSRam):
		resd 64
	
	global SYM(H_Counter_Table)
	SYM(H_Counter_Table):
		resb 512 * 2
	
	global SYM(VDP_Reg)
	SYM(VDP_Reg):
		.Set_1:			resd 1
		.Set_2:			resd 1
		.Pat_ScrA_Adr:		resd 1
		.Pat_WIN_Adr:		resd 1
		.Pat_ScrB_Adr:		resd 1
		.Spr_Att_Adr:		resd 1
		.Reg6:			resd 1
		.BG_Color:		resd 1
		.Reg8:			resd 1
		.Reg9:			resd 1
		.H_Int_Reg:		resd 1
		.Set_3:			resd 1
		.Set_4:			resd 1
		.H_Scr_Adr:		resd 1
		.Reg14:			resd 1
		.Auto_Inc:		resd 1
		.Scr_Size:		resd 1
		.Win_H_Pos:		resd 1
		.Win_V_Pos:		resd 1
		.DMA_Length_L:		resd 1
		.DMA_Length_H:		resd 1
		.DMA_Src_Adr_L:		resd 1
		.DMA_Src_Adr_M:		resd 1
		.DMA_Src_Adr_H:		resd 1
		
		.DMA_Length:		resd 1
		.DMA_Address:		resd 1
	
	global SYM(ScrA_Addr)
	SYM(ScrA_Addr):
		resd 1
	global SYM(ScrB_Addr)
	SYM(ScrB_Addr):
		resd 1
	global SYM(Win_Addr)
	SYM(Win_Addr):
		resd 1
	global SYM(Spr_Addr)
	SYM(Spr_Addr):
		resd 1
	global SYM(H_Scroll_Addr)
	SYM(H_Scroll_Addr):
		resd 1
	global SYM(H_Cell)
	SYM(H_Cell):
		resd 1
	global SYM(H_Win_Mul)
	SYM(H_Win_Mul):
		resd 1
	global SYM(H_Pix)
	SYM(H_Pix):
		resd 1
	global SYM(H_Pix_Begin)
	SYM(H_Pix_Begin):
		resd 1
	
	global SYM(H_Scroll_Mask)
	SYM(H_Scroll_Mask):
		resd 1
	global SYM(H_Scroll_CMul)
	SYM(H_Scroll_CMul):
		resd 1
	global SYM(H_Scroll_CMask)
	SYM(H_Scroll_CMask):
		resd 1
	global SYM(V_Scroll_CMask)
	SYM(V_Scroll_CMask):
		resd 1
	global SYM(V_Scroll_MMask)
	SYM(V_Scroll_MMask):
		resd 1
	
	global SYM(Win_X_Pos)
	SYM(Win_X_Pos):
		resd 1
	global SYM(Win_Y_Pos)
	SYM(Win_Y_Pos):
		resd 1
	
	global SYM(Ctrl)
	SYM(Ctrl):
		.Flag:		resd 1
		.Data:		resd 1
		.Write:		resd 1
		.Access:	resd 1
		.Address:	resd 1
		.DMA_Mode:	resd 1
		.DMA:		resd 1
	
	global SYM(DMAT_Tmp)
	SYM(DMAT_Tmp):
		resd 1
	global SYM(DMAT_Length)
	SYM(DMAT_Length):
		resd 1
	global SYM(DMAT_Type)
	SYM(DMAT_Type):
		resd 1
	
	global SYM(VDP_Status)
	SYM(VDP_Status):
		resd 1
	global SYM(VDP_Int)
	SYM(VDP_Int):
		resd 1
	global SYM(VDP_Current_Line)
	SYM(VDP_Current_Line):
		resd 1
	global SYM(VDP_Num_Lines)
	SYM(VDP_Num_Lines):
		resd 1
	global SYM(VDP_Num_Vis_Lines)
	SYM(VDP_Num_Vis_Lines):
		resd 1
	global SYM(CRam_Flag)
	SYM(CRam_Flag):
		resd 1
	global SYM(VRam_Flag)
	SYM(VRam_Flag):
		resd 1
	
	global SYM(Zero_Length_DMA)
	SYM(Zero_Length_DMA):
		resd 1
	
section .text align=64
	
	extern SYM(main68k_readOdometer)
	extern SYM(main68k_releaseCycles)
	extern SYM(main68k_interrupt)
	
	extern SYM(Write_To_68K_Space)
	
	; Functions found in vdp_io.c
	extern SYM(VDP_Update_IRQ_Line)

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
	mov	dword [SYM(VRam_Flag)], 1
	mov	byte [SYM(DMAT_Type)], 0
%else
	%if %2 < 2
		mov	dword [SYM(CRam_Flag)], 1
	%endif
	mov	byte [SYM(DMAT_Type)], 1
%endif
	xor	edi, edi
	mov	dword [SYM(Ctrl.DMA)], 0
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
	
	; unsigned int Update_DMA(void)
	global SYM(Update_DMA)
	SYM(Update_DMA):
		
		push	ebx
		push	ecx
		push	edx
		
		mov	ebx, [SYM(VDP_Reg) + 12 * 4]	; 32 / 40 Cell ?
		mov	edx, [SYM(DMAT_Type)]
		mov	eax, [SYM(VDP_Current_Line)]
		mov	ecx, [SYM(VDP_Num_Vis_Lines)]
		and	ebx, byte 1
		and	edx, byte 3
		cmp	eax, ecx
		lea	ebx, [ebx * 4 + edx]
		jae	short .Blanking
		test	byte [SYM(VDP_Reg) + 1 * 4], 0x40	; VDP Enable ?
		jz	short .Blanking
		
		add	ebx, byte 8
		
	.Blanking:
		movzx	ecx, byte [SYM(DMA_Timing_Table) + ebx]
		mov	eax, [SYM(CPL_M68K)]
		sub	dword [SYM(DMAT_Length)], ecx
		ja	short .DMA_Not_Finished
		
			shl	eax, 16
			mov	ebx, [SYM(DMAT_Length)]
			xor	edx, edx
			add	ebx, ecx
			mov	[SYM(DMAT_Length)], edx
			div	ecx
			and	word [SYM(VDP_Status)], 0xFFFD
			mul	ebx
			shr	eax, 16
			test	byte [SYM(DMAT_Type)], 2
			jnz	short .DMA_68k_CRam_VSRam
		
		pop	edx
		pop	ecx
		pop	ebx
		ret

	.DMA_Not_Finished:
		test	byte [SYM(DMAT_Type)], 2
		jz	short .DMA_68k_VRam

	.DMA_68k_CRam_VSRam:
		xor	eax, eax
			
	.DMA_68k_VRam:
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	; uint16_t Read_VDP_Data(void)
	global SYM(Read_VDP_Data)
	SYM(Read_VDP_Data):
		
		push	ebx
		mov	byte [SYM(Ctrl.Flag)], 0		; on en a finit avec Address Set
		push	ecx
		mov	ebx, [SYM(Ctrl.Address)]
		mov	eax, [SYM(Ctrl.Access)]
		mov	ecx, ebx
		jmp	[.Table_Read + eax * 4]
	
	align 16
	
	.Table_Read:
		dd	error, error, error, error		; Wrong
		dd	error, .RD_VRAM, .RD_CRAM, .RD_VSRAM	; READ
		dd	error, error, error, error		; WRITE
		dd	error, error, error, error		; WRITE & READ (WRONG)
	
	align 16
	
	.RD_VRAM:
		add	ecx, [SYM(VDP_Reg.Auto_Inc)]
		and	ebx, 0xFFFE
		mov	[SYM(Ctrl.Address)], cx
		mov	ax, [SYM(VRam) + ebx]
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.RD_CRAM:
		add	ecx, [SYM(VDP_Reg.Auto_Inc)]
		and	ebx, byte 0x7E
		mov	[SYM(Ctrl.Address)], cx
		mov	ax, [SYM(CRam) + ebx]
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.RD_VSRAM:
		add ecx, [SYM(VDP_Reg.Auto_Inc)]
		and ebx, byte 0x7E
		mov [SYM(Ctrl.Address)], cx
		mov ax, [SYM(VSRam) + ebx]
		pop ecx
		pop ebx
		ret
	
	align 16
	
	; uint16_t Read_VDP_Status(void)
	global SYM(Read_VDP_Status)
	SYM(Read_VDP_Status):
		
		mov	ax, [SYM(VDP_Status)]
		push	ax
		xor	ax, 0xFF00
		and	ax, 0xFF9F
		test	ax, 0x0008
		jnz	short .In_VBlank
		and	ax, 0xFF1F
		
	.In_VBlank:
		mov	[SYM(VDP_Status)], ax
		test	byte [SYM(VDP_Reg.Set_2)], 0x40
		pop	ax
		jz	short .Display_OFF
		ret
	
	align 16
	
	.Display_OFF:
		or	ax, 8
		ret
	
	align 16
	
	; uint8_t Read_VDP_H_Counter(void)
	global SYM(Read_VDP_H_Counter)
	SYM(Read_VDP_H_Counter):
		
		push ebx
		
		call	SYM(main68k_readOdometer)
		mov	ebx, [SYM(Cycles_M68K)]
		sub	ebx, [SYM(CPL_M68K)]
		sub	eax, ebx			; Nb cycles effectués sur cette ligne.
		xor	ebx, ebx
		and	eax, 0x1FF
		test	byte [SYM(VDP_Reg.Set_4)], 0x81	; 40 cell mode ?
		setnz	bl
		mov	al, [SYM(H_Counter_Table) + eax * 2 + ebx]
		xor	ah, ah
		
		pop ebx
		ret
	
	align 16
	
	; uint8_t Read_VDP_V_Counter(void)
	global SYM(Read_VDP_V_Counter)
	SYM(Read_VDP_V_Counter):
		
		push ebx
		
		call	SYM(main68k_readOdometer)
		mov	ebx, [SYM(Cycles_M68K)]
		sub	ebx, [SYM(CPL_M68K)]
		sub	eax, ebx			; Nb cycles effectués sur cette ligne.
		xor	ebx, ebx
		and	eax, 0x1FF
		test	byte [SYM(VDP_Reg.Set_4)], 0x81	; 40 cell mode ?
		jz	short .mode_32
		
	.mode_40:
		mov	al, [SYM(H_Counter_Table) + eax * 2 + 1]
		mov	bl, 0xA4
		jmp	short .ok
		
	align 16
		
	 .mode_32:
		mov	al, [SYM(H_Counter_Table) + eax * 2 + 0]
		mov	bl, 0x84
		
	 .ok:
		cmp	al, 0xE0
		setbe	bh
		cmp	al, bl
		setae	bl
		and	bl, bh
		
		test	byte [SYM(VDP_Status)], 1		; PAL ?
		jnz	short .PAL
		
	.NTSC:
		mov	eax, [SYM(VDP_Current_Line)]
		shr	bl, 1
		adc	eax, 0
		cmp	eax, 0xEB
		jb	short .No_Over_Line_XX
		
		sub	eax, 6
		jmp	short .No_Over_Line_XX
		
	.PAL:
		mov	eax, [SYM(VDP_Current_Line)]
		shr	bl, 1
		adc	eax, byte 0
		cmp	eax, 0x103
		jb	short .No_Over_Line_XX

		sub	eax, byte 56

	.No_Over_Line_XX:
		test	byte [SYM(VDP_Reg.Set_4)], 2
		jz	short .No_Interlace
		
		rol	al, 1
		
	.No_Interlace:
		xor	ah, ah
		pop	ebx
		ret
	
	align 16
	
	; void Write_Byte_VDP_Data(uint8_t Data)
	global SYM(Write_Byte_VDP_Data)
	SYM(Write_Byte_VDP_Data):
		
		test	byte [SYM(Ctrl.DMA)], 0x4
		mov	al, [esp + 4]
		mov	byte [SYM(Ctrl.Flag)], 0			; on en a finit avec Address Set
		mov	ah, al
		jnz	near DMA_Fill
		jmp	short Write_VDP_Data
	
	align 16
	
	;void Write_Word_VDP_Data(uint16_t Data)
	global SYM(Write_Word_VDP_Data)
	SYM(Write_Word_VDP_Data):
		
		test	byte [SYM(Ctrl.DMA)], 0x4
		mov	byte [SYM(Ctrl.Flag)], 0			; on en a finit avec Address Set
		mov	ax, [esp + 4]
		jnz	near DMA_Fill
	
	Write_VDP_Data:
		push	ebx
		push	ecx
		mov	ecx, [SYM(Ctrl.Access)]
		mov	ebx, [SYM(Ctrl.Address)]
		jmp	[.Table_Write_W + ecx * 4]
	
	align 16
	
	.Table_Write_W:
		dd	error, error, error, error		; Wrong
		dd	error, error, error, error		; READ
		dd	error, .WR_VRAM, .WR_CRAM, .WR_VSRAM	; WRITE
		dd	error, error, error, error		; WRITE & READ (WRONG)
	
	align 16
	
	.WR_VRAM:
		mov	ecx, ebx
		shr	ebx, 1
		mov	byte [SYM(VRam_Flag)], 1
		jnc	short .Address_Even
		rol	ax, 8
	
	.Address_Even:
		add	ecx, [SYM(VDP_Reg.Auto_Inc)]
		mov	[SYM(VRam) + ebx * 2], ax
		mov	[SYM(Ctrl.Address)], cx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.WR_CRAM:
		mov	ecx, ebx
		and	ebx, byte 0x7E
		add	ecx, [SYM(VDP_Reg.Auto_Inc)]
		mov	byte [SYM(CRam_Flag)], 1
		mov	[SYM(Ctrl.Address)], cx
		mov	[SYM(CRam) + ebx], ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.WR_VSRAM:
		mov	ecx, ebx
		and	ebx, byte 0x7E
		add	ecx, [SYM(VDP_Reg.Auto_Inc)]
		mov	[SYM(VSRam) + ebx], ax
		mov	[SYM(Ctrl.Address)], cx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	DMA_Fill:
		push	ebx
		push	ecx
		push	edx
		
		mov	ebx, [SYM(Ctrl.Address)]		; bx = Address Dest
		mov	ecx, [SYM(VDP_Reg.DMA_Length)]	; DMA Length
		mov	edx, [SYM(VDP_Reg.Auto_Inc)]	; edx = Auto_Inc
		mov	dword [SYM(VDP_Reg.DMA_Length)], 0	; Clear DMA.Length
		and	ebx, 0xFFFF
		mov	dword [SYM(Ctrl.DMA)], 0		; Flag DMA Fill = 0
		xor	ebx, 1
		or	word [SYM(VDP_Status)], 0x0002
		mov	[SYM(VRam) + ebx], al
		xor	ebx, 1
		mov	dword [SYM(DMAT_Type)], 0x2
		and	ecx, 0xFFFF
		mov	byte [SYM(VRam_Flag)], 1
		mov	dword [SYM(DMAT_Length)], ecx
		jnz	short .Loop
		
		; DMA length is 0. Set it to 65,536 words.
		mov	ecx, 0x10000
		
		mov	[SYM(DMAT_Length)], ecx
		jmp	short .Loop
	
	align 16
	
		.Loop
			mov	[SYM(VRam) + ebx], ah	; VRam[Adr] = Fill Data
			add	bx, dx			; Adr = Adr + Auto_Inc
			dec	ecx			; un transfert de moins
			jns	short .Loop		; s'il en reste alors on continue
		
		mov	[SYM(Ctrl.Address)], bx		; on stocke la nouvelle valeur de Data_Address
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	;void Write_VDP_Ctrl(uint16_t Data)
	global SYM(Write_VDP_Ctrl)
	SYM(Write_VDP_Ctrl):
		
;		push	ebx
;		mov	eax, [esp + 8]
;		
;		mov	ebx, eax
;		and	eax, 0xC000			; on isole pour tester le mode
;		cmp	eax, 0x8000			; on est en mode set register
;		je	short .Set_Register
;
;		test	dword [SYM(Ctrl.Flag)], 1		; est-on à la 1ère ecriture ??
;		mov	eax, ebx
;		jz	short .First_Word		; si oui on y va !
;		jmp	.Second_Word			; sinon
;	
;	align 16
;	
;	.Set_Register
;		mov	eax, ebx
;		shr	ebx, 8				; ebx = numero du registre 
;		mov	byte [SYM(Ctrl.Access)], 5
;		and	eax, 0xFF			; on isole la valeur du registre
;		and	ebx, 0x1F			; on isole le numero du registre
;		mov	word [SYM(Ctrl.Address)], 0
;		jmp	[Table_Set_Reg + ebx * 4]	; on affecte en fonction
		
		mov	eax, [esp + 4]
		test	byte [SYM(Ctrl.Flag)], 1		; est-on à la 2eme ecriture ??
		push	ebx
		jnz	near .Second_Word		; sinon

		mov	ebx, eax
		and	eax, 0xC000			; on isole pour tester le mode
		cmp	eax, 0x8000			; on est en mode set register
		jne	short .First_Word

		mov	eax, ebx
		mov	bl, bh				; bl = numero du registre 
		mov	dword [SYM(Ctrl.Access)], 5
		and	eax, 0xFF			; on isole la valeur du registre
		mov	dword [SYM(Ctrl.Address)], 0
		and	ebx, 0x1F			; on isole le numero du registre 
		jmp	[Table_Set_Reg + ebx * 4]	; on affecte en fonction
	
	align 16
	
	.First_Word:	; 1st Write
		push	ecx
		push	edx
		mov	ax, [SYM(Ctrl.Data) + 2]		; ax = 2nd word (AS)
		mov	ecx, ebx			; cx = bx = 1st word (AS)
		mov	[SYM(Ctrl.Data)], bx		; et on sauvegarde les premiers 16 bits (AS)
		mov	edx, eax			; dx = ax = 2nd word (AS)
		mov	byte [SYM(Ctrl.Flag)], 1		; la prochaine ecriture sera Second
		shl	eax, 14				; on isole l'adresse
		and	ebx, 0x3FFF			; on isole l'adresse
		and	ecx, 0xC000			; on isole les bits de CD
		or	ebx, eax			; ebx = Address IO VRAM
		and	edx, 0xF0			; on isole les bits de CD
		shr	ecx, 12				;		"		"
		mov	[SYM(Ctrl.Address)], bx		; Ctrl.Address = Address de depart pour le port VDP Data
		or	edx, ecx			; edx = CD
		mov	eax, [SYM(CD_Table) + edx]		; eax = Location & Read/Write
		mov	[SYM(Ctrl.Access)], al		; on stocke l'accés
		
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Second_Word:
		push	ecx
		push	edx
		mov	cx, [SYM(Ctrl.Data)]			; cx = 1st word (AS)
		mov	edx, eax			; dx = ax = 2nd word (AS)
		mov	[SYM(Ctrl.Data) + 2], ax		; on stocke le controle complet
		mov	ebx, ecx			; bx = 1st word (AS)
		shl	eax, 14				; on isole l'adresse
		and	ebx, 0x3FFF			; on isole l'adresse
		and	ecx, 0xC000			; on isole les bits de CD
		or	ebx, eax			; ebx = Address IO VRAM
		and	edx, 0xF0			; on isole les bits de CD
		shr	ecx, 12				;		"		"
		mov	[SYM(Ctrl.Address)], bx		; Ctrl.Address = Address de depart pour le port VDP Data
		or	edx, ecx			; edx = CD
		mov	eax, [SYM(CD_Table) + edx]		; eax = Location & Read/Write
		mov	byte [SYM(Ctrl.Flag)], 0		; on en a finit avec Address Set
		test	ah, ah				; on teste si il y a transfert DMA
		mov	[SYM(Ctrl.Access)], al		; on stocke l'accés
		mov	al, ah
		jnz	short .DO_DMA			; si oui on y va
		
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.DO_DMA:
		push	edi
		push	esi
		
		test	dword [SYM(VDP_Reg.Set_2)], 0x10		; DMA enable ?
		jz	near .NO_DMA
		test	al, 0x4					; DMA FILL ?
		jz	short .No_Fill
			
			cmp	byte [SYM(Ctrl.DMA_Mode)], 0x80
			jne	short .No_Fill
			mov	[SYM(Ctrl.DMA)], al			; on stocke le type de DMA
			pop	esi
			pop	edi
			pop	edx
			pop	ecx
			pop	ebx
			ret
	
	align 16
	
	.No_Fill:
		mov 	ecx, [SYM(VDP_Reg.DMA_Length)]		; ecx = DMA Length
		mov	esi, [SYM(VDP_Reg.DMA_Address)]		; esi = DMA Source Address / 2
		and	eax, byte 3				; eax = destination DMA (1:VRAM, 2:CRAM, 3:VSRAM)
		and	ecx, 0xFFFF
		mov	edi, [SYM(Ctrl.Address)]			; edi = Address Dest
		
		; If the DMA length is 0, set it to 65,536 words.
		jnz	short .non_zero_DMA
		
		; If Zero_Length_DMA is enabled, don't do any DMA request.
		test	byte [SYM(Zero_Length_DMA)], 1
		jnz	near .NO_DMA
		
		; Zero_Length_DMA is disabled.
		; The MD VDP decrements the DMA length counter before checking if it has
		; reached zero. So, doing a zero-length DMA request will actually do a
		; DMA request for 65,536 words.
		;
		; NOTE: This only appears to be correct for MEM-to-VRAM, not MEM-to-CRAM or
		; MEM-to-VSRAM. Zero-length CRAM and VSRAM transfers are ignored for now.
		
		cmp	eax, 1		; VRAM
		jne	near .NO_DMA
		mov	ecx, 0x10000
		
	.non_zero_DMA:
		and	edi, 0xFFFF				; edi = Address Dest
		cmp	byte [SYM(Ctrl.DMA_Mode)], 0xC0		; DMA Copy ?
		mov	edx, [SYM(VDP_Reg.Auto_Inc)]		; edx = Auto Inc
		je	near .V_RAM_Copy
	
	.MEM_To_V_RAM:
		add	esi, esi				; esi = DMA Source Address
		test	dword [SYM(Ctrl.DMA_Mode)], 0x80
		jnz	near .NO_DMA
		xor	ebx, ebx
		cmp	esi, [SYM(Rom_Size)]
		jb	short .DMA_Src_OK			; Src = ROM (ebx = 0)
		mov	ebx, 1
		test	byte [SYM(SegaCD_Started)], 0xFF
		jz	short .DMA_Src_OK			; Src = Normal RAM (ebx = 1)
		
		cmp	esi, 0x00240000
		jae	short .DMA_Src_OK			; Src = Normal RAM (ebx = 1)
		cmp	esi, 0x00040000
		mov	ebx, 2
		jb	short .DMA_Src_OK			; Src = PRG RAM (ebx = 2)
		
		mov	bh, [SYM(Ram_Word_State)]
		mov	bl, 3					; Src = WORD RAM ; 3 = WORD RAM 2M
		and	bh, 3					; 4 = BAD
		add	bl, bh					; 5 = WORD RAM 1M Bank 0
		xor	bh, bh					; 6 = WORD RAM 1M Bank 1
		cmp	bl, 5
		jb	short .DMA_Src_OK
		cmp	esi, 0x00220000
		jb	short .DMA_Src_OK			; 7 = CELL ARRANGED Bank 0
		add	bl, 2					; 8 = CELL ARRANGED Bank 1
	
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
		mov	eax, [SYM(VDP_Reg.DMA_Length)]
		mov	[SYM(Ctrl.Address)], bx
		mov	esi, [SYM(VDP_Reg.DMA_Address)]
		sub	eax, ecx
		mov	[SYM(VDP_Reg.DMA_Length)], ecx
		lea	esi, [esi + eax]
		jbe	short .Nothing_To_Do
		
		and	esi, 0x7FFFFF
		mov	[SYM(DMAT_Length)], eax
		mov	[SYM(VDP_Reg.DMA_Address)], esi
		call	SYM(Update_DMA)
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
	
	.V_RAM_Copy:
		or	word [SYM(VDP_Status)], 0x0002
		and	esi, 0xFFFF
		mov	dword [SYM(VDP_Reg.DMA_Length)], 0
		mov	dword [SYM(DMAT_Length)], ecx
		mov	dword [SYM(DMAT_Type)], 0x3
		mov	dword [SYM(VRam_Flag)], 1
		jmp	short .VRam_Copy_Loop
	
	align 16

	.VRam_Copy_Loop:
			mov	al, [SYM(VRam) + esi]		; ax = Src
			inc	si				; on augment pointeur Src de 1
			mov	[SYM(VRam) + edi], al		; VRam[Dest] = Src.W
			add	di, dx				; Adr = Adr + Auto_Inc
			dec	ecx				; un transfert de moins
			jnz	short .VRam_Copy_Loop		; si DMA Length >= 0 alors on continue le transfert DMA
		
		mov	[SYM(VDP_Reg.DMA_Address)], esi
		mov	[SYM(Ctrl.Address)], di			; on stocke la nouvelle Data_Address
		pop	esi
		pop	edi
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.NO_DMA:
		mov	dword [SYM(Ctrl.DMA)], 0
		pop	esi
		pop	edi
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	Table_Set_Reg:
		dd	Set_Regs.Set1,	Set_Regs.Set2,	Set_Regs.ScrA,	Set_Regs.Win
		dd	Set_Regs.ScrB,	Set_Regs.Spr,	Set_Regs,	Set_Regs.BGCol
		dd	Set_Regs,	Set_Regs,	Set_Regs.HInt,	Set_Regs.Set3
		dd	Set_Regs.Set4,	Set_Regs.HScr,	Set_Regs,	Set_Regs
		dd	Set_Regs.ScrSize, Set_Regs.WinH, Set_Regs.WinV,	Set_Regs.DMALL
		dd	Set_Regs.DMALH,	Set_Regs.DMAAL,	Set_Regs.DMAAM,	Set_Regs.DMAAH
		dd	Set_Regs.Wrong,	Set_Regs.Wrong,	Set_Regs.Wrong,	Set_Regs.Wrong
		dd	Set_Regs.Wrong,	Set_Regs.Wrong,	Set_Regs.Wrong,	Set_Regs.Wrong
		
	align 16
	
	;void Set_VDP_Reg(int Num_Reg, uint8_t val);
	global SYM(Set_VDP_Reg)
	SYM(Set_VDP_Reg):
		
		push	ebx
		
		mov	ebx, [esp + 8]
		movzx	eax, byte [esp + 12]
		and	ebx, byte 0x1F
		jmp	[Table_Set_Reg + ebx * 4]
	
	align 16
	
	Set_Regs:
	; al = valeur
	; ebx = numero de registre
	; ne pas oublier de depiler ebx a la fin
		
		mov	[SYM(VDP_Reg) + ebx * 4], al
		pop	ebx
		ret
	
	align 16
	
	.Set1:
		mov	[SYM(VDP_Reg.Set_1)], al
		call	SYM(VDP_Update_IRQ_Line)
		
		; VDP register 0, bit 2: Palette Select
		; If cleared, only the LSBs of each CRAM component is used.
		mov	dword [SYM(CRam_Flag)], 1
		
		pop	ebx
		ret
	
	align 16
	
	.Set2:
		mov	[SYM(VDP_Reg.Set_2)], al
		call	SYM(VDP_Update_IRQ_Line)
		pop	ebx
		ret
	
	align 16
	
	.Set3:
		test	al, 4
		mov	[SYM(VDP_Reg.Set_3)], al
		jnz	short .VScroll_Cell
		
		and	eax, 3
		pop	ebx
		mov	eax, [SYM(H_Scroll_Mask_Table) + eax * 4]
		mov	byte [SYM(V_Scroll_MMask)], 0
		mov	[SYM(H_Scroll_Mask)], eax
		ret
	
	align 16
	
	.VScroll_Cell:
		and	eax, 3
		pop	ebx
		mov	eax, [SYM(H_Scroll_Mask_Table) + eax * 4]
		mov	byte [SYM(V_Scroll_MMask)], 0x7E
		mov	[SYM(H_Scroll_Mask)], eax
		ret
	
	align 16
	
	.Set4:
		mov	[SYM(VDP_Reg.Set_4)], al
		mov	byte [SYM(CRam_Flag)], 1
		test	al, 0x81
		pop	ebx
		jz	short .HCell_32
		
		mov	dword [SYM(H_Cell)], 40
		mov	dword [SYM(H_Win_Mul)], 6
		mov	dword [SYM(H_Pix)], 320
		mov	dword [SYM(H_Pix_Begin)], 0
		
		mov	eax, [SYM(VDP_Reg.Pat_WIN_Adr)]
		and	eax, byte 0x3C
		shl	eax, 10
		add	eax, SYM(VRam)
		mov	[SYM(Win_Addr)], eax
		
		mov	al, [SYM(VDP_Reg.Win_H_Pos)]
		and	al, 0x1F
		add	al, al
		cmp	al, 40
		jbe	short .HCell_40_ok
		
		mov	al, 40
		
	.HCell_40_ok:
		mov	[SYM(Win_X_Pos)], al
		mov	eax, [SYM(VDP_Reg.Spr_Att_Adr)]
		and	eax, byte 0x7E
		shl	eax, 9
		add	eax, SYM(VRam)
		mov	[SYM(Spr_Addr)], eax
		ret
	
	align 16
	
	.HCell_32:
		mov	dword [SYM(H_Cell)], 32
		mov	dword [SYM(H_Win_Mul)], 5
		mov	dword [SYM(H_Pix)], 256
		mov	dword [SYM(H_Pix_Begin)], 32
		
		mov	eax, [SYM(VDP_Reg.Pat_WIN_Adr)]
		and	eax, byte 0x3E
		shl	eax, 10
		add	eax, SYM(VRam)
		mov	[SYM(Win_Addr)], eax
		
		mov	al, [SYM(VDP_Reg.Win_H_Pos)]
		and	al, 0x1F
		add	al, al
		cmp	al, 32
		jbe	short .HCell_32_ok
		
		mov	al, 32
		
	.HCell_32_ok:
		mov	[SYM(Win_X_Pos)], al
		mov	eax, [SYM(VDP_Reg.Spr_Att_Adr)]
		and	eax, byte 0x7F
		shl	eax, 9
		add	eax, SYM(VRam)
		mov	[SYM(Spr_Addr)], eax
		ret
	
	align 16
	
	.ScrA:
		mov	[SYM(VDP_Reg.Pat_ScrA_Adr)], al
		and	eax, 0x38
		shl	eax, 10
		pop	ebx
		add	eax, SYM(VRam)
		mov	[SYM(ScrA_Addr)], eax
		ret
	
	align 16
	
	.Win:
		test	byte [SYM(VDP_Reg.Set_4)], 0x1
		mov	[SYM(VDP_Reg.Pat_WIN_Adr)], al
		jnz	short .w2
		
		and	eax, 0x3E
		shl	eax, 10
		pop	ebx
		add	eax, SYM(VRam)
		mov	[SYM(Win_Addr)], eax
		ret
	
	align 16
	
	.w2:
		and	eax, 0x3C
		shl	eax, 10
		pop	ebx
		add	eax, SYM(VRam)
		mov	[SYM(Win_Addr)], eax
		ret
	
	align 16
	
	.ScrB:
		mov	[SYM(VDP_Reg.Pat_ScrB_Adr)], al
		and	eax, 0x7
		shl	eax, 13
		pop	ebx
		add	eax, SYM(VRam)
		mov	[SYM(ScrB_Addr)], eax
		ret
	
	align 16
	
	.Spr:
		test	byte [SYM(VDP_Reg.Set_4)], 0x1
		mov	[SYM(VDP_Reg.Spr_Att_Adr)], al
		jnz	short .spr2
		
		and	eax, 0x7F
		or	byte [SYM(VRam_Flag)], 2
		shl	eax, 9
		pop	ebx
		add	eax, SYM(VRam)
		mov	[SYM(Spr_Addr)], eax
		ret
	
	align 16
	
	.spr2:
		and	eax, 0x7E
		or	byte [SYM(VRam_Flag)], 2
		shl	eax, 9
		pop	ebx
		add	eax, SYM(VRam)
		mov	[SYM(Spr_Addr)], eax
		ret
	
	align 16
	
	.BGCol:
		and	eax, 0x3F
		pop	ebx
		mov	byte [SYM(CRam_Flag)], 1
		mov	[SYM(VDP_Reg.BG_Color)], eax
		ret
	
	align 16
	
	.HInt:
		mov	[SYM(VDP_Reg.H_Int_Reg)], al
		pop	ebx
		ret
	
	align 16
	
	.HScr:
		mov	[SYM(VDP_Reg.H_Scr_Adr)], al
		and	eax, 0x3F
		shl	eax, 10
		pop	ebx
		add	eax, SYM(VRam)
		mov	[SYM(H_Scroll_Addr)], eax
		ret
	
	align 16
	
	.ScrSize:
		mov	ebx, eax
		mov	[SYM(VDP_Reg.Scr_Size)], al
		and	ebx, 0x03
		and	eax, 0x30
		jmp	[.ScrSize_Table + eax + ebx * 4]
	
	align 16
	
	.ScrSize_Table:
		dd	.V32_H32, .V32_H64, .V32_HXX, .V32_H128
		dd	.V64_H32, .V64_H64, .V64_HXX, .V64_H128
		dd	.VXX_H32, .VXX_H64, .VXX_HXX, .VXX_H128
		dd	.V128_H32, .V128_H64, .V128_HXX, .V128_H128
	
	align 16
	
	.V32_H32:
	.VXX_H32:
		mov	dword [SYM(H_Scroll_CMul)], 5
		mov	dword [SYM(H_Scroll_CMask)], 31
		mov	dword [SYM(V_Scroll_CMask)], 31
		pop	ebx
		ret
	
	align 16
	
	.V64_H32:
		mov	dword [SYM(H_Scroll_CMul)], 5
		mov	dword [SYM(H_Scroll_CMask)], 31
		mov	dword [SYM(V_Scroll_CMask)], 63
		pop	ebx
		ret
	
	align 16
	
	.V128_H32:
		mov	dword [SYM(H_Scroll_CMul)], 5
		mov	dword [SYM(H_Scroll_CMask)], 31
		mov	dword [SYM(V_Scroll_CMask)], 127
		pop	ebx
		ret
	
	align 16
	
	.V32_H64:
	.VXX_H64:
		mov	dword [SYM(H_Scroll_CMul)], 6
		mov	dword [SYM(H_Scroll_CMask)], 63
		mov	dword [SYM(V_Scroll_CMask)], 31
		pop	ebx
		ret
	
	align 16
	
	.V64_H64:
	.V128_H64:
		mov	dword [SYM(H_Scroll_CMul)], 6
		mov	dword [SYM(H_Scroll_CMask)], 63
		mov	dword [SYM(V_Scroll_CMask)], 63
		pop	ebx
		ret
	
	align 16
	
	.V32_HXX:
	.V64_HXX:
	.VXX_HXX:
	.V128_HXX:
		mov	dword [SYM(H_Scroll_CMul)], 6
		mov	dword [SYM(H_Scroll_CMask)], 63
		mov	dword [SYM(V_Scroll_CMask)], 0
		pop	ebx
		ret
	
	align 16
	
	.V32_H128:
	.V64_H128:
	.VXX_H128:
	.V128_H128:
		mov	dword [SYM(H_Scroll_CMul)], 7
		mov	dword [SYM(H_Scroll_CMask)], 127
		mov	dword [SYM(V_Scroll_CMask)], 31
		pop	ebx
		ret
	
	align 16
	
	.WinH:
		mov	[SYM(VDP_Reg.Win_H_Pos)], al
		and	eax, 0x1F
		pop	ebx
		add	eax, eax
		cmp	eax, [SYM(H_Cell)]
		jbe	short .WinH_ok
		
		mov	eax, [SYM(H_Cell)]
		
	.WinH_ok:
		mov [SYM(Win_X_Pos)], eax
		ret
	
	align 16
	
	.WinV:
		mov	[SYM(VDP_Reg.Win_V_Pos)], al
		and	eax, 0x1F
		pop	ebx
		mov	[SYM(Win_Y_Pos)], eax
		ret
	
	align 16
	
	.DMALL:
		mov	[SYM(VDP_Reg.DMA_Length_L)], al
		pop	ebx
		mov	[SYM(VDP_Reg.DMA_Length)], al
		ret
	
	align 16
	
	.DMALH:
		mov	[SYM(VDP_Reg.DMA_Length_H)], al
		pop	ebx
		mov	[SYM(VDP_Reg.DMA_Length) + 1], al
		ret
	
	align 16
	
	.DMAAL:
		mov	[SYM(VDP_Reg.DMA_Src_Adr_L)], al
		pop	ebx
		mov	[SYM(VDP_Reg.DMA_Address)], al
		ret
	
	align 16
	
	.DMAAM:
		mov	[SYM(VDP_Reg.DMA_Src_Adr_M)], al
		pop	ebx
		mov	[SYM(VDP_Reg.DMA_Address) + 1], al
		ret
	
	align 16
	
	.DMAAH:
		mov	[SYM(VDP_Reg.DMA_Src_Adr_H)], al
		mov	ebx, eax
		and	eax, 0x7F
		and	ebx, 0xC0
		mov	[SYM(VDP_Reg.DMA_Address) + 2], al
		mov	[SYM(Ctrl.DMA_Mode)], ebx				; DMA Mode
		pop	ebx
		ret
	
	align 16
	
	.Wrong:
		pop	ebx
		ret
