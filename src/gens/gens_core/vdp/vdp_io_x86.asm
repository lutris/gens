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
	
	extern SYM(VDP_Status)
	extern SYM(VDP_Int)
	extern SYM(VDP_Current_Line)
	extern SYM(VDP_Num_Lines)
	extern SYM(VDP_Num_Vis_Lines)
	
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
		test	byte [SYM(VDP_Ctrl) + VDP_Ctrl_t.Flag], 1	; est-on à la 2eme ecriture ??
		push	ebx
		jnz	near .Second_Word		; sinon

		mov	ebx, eax
		and	eax, 0xC000			; on isole pour tester le mode
		cmp	eax, 0x8000			; on est en mode set register
		jne	short .First_Word

		mov	eax, ebx
		mov	bl, bh				; bl = numero du registre 
		mov	dword [SYM(VDP_Ctrl) + VDP_Ctrl_t.Access], 5
		and	eax, 0xFF			; on isole la valeur du registre
		mov	dword [SYM(VDP_Ctrl) + VDP_Ctrl_t.Address], 0
		and	ebx, 0x1F			; on isole le numero du registre 
		
		; Set the register.
		push	eax				; Register value.
		push	ebx				; Register number.
		call	SYM(VDP_Set_Reg)
		add	esp, byte 8
		
		pop	ebx
		ret
	
	align 16
	
	.First_Word:	; 1st Write
		push	ecx
		push	edx
		mov	ax, [SYM(VDP_Ctrl) + VDP_Ctrl_t.Data + 2]	; ax = 2nd word (AS)
		mov	ecx, ebx					; cx = bx = 1st word (AS)
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Data], bx		; et on sauvegarde les premiers 16 bits (AS)
		mov	edx, eax					; dx = ax = 2nd word (AS)
		mov	byte [SYM(VDP_Ctrl) + VDP_Ctrl_t.Flag], 1	; la prochaine ecriture sera Second
		shl	eax, 14				; on isole l'adresse
		and	ebx, 0x3FFF			; on isole l'adresse
		and	ecx, 0xC000			; on isole les bits de CD
		or	ebx, eax			; ebx = Address IO VRAM
		and	edx, 0xF0			; on isole les bits de CD
		shr	ecx, 12				; "		"
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Address], bx	; Ctrl.Address = Address de depart pour le port VDP Data
		or	edx, ecx					; edx = CD
		mov	eax, [SYM(CD_Table) + edx]			; eax = Location & Read/Write
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Access], al		; on stocke l'accés
		
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Second_Word:
		push	ecx
		push	edx
		mov	cx, [SYM(VDP_Ctrl) + VDP_Ctrl_t.Data]		; cx = 1st word (AS)
		mov	edx, eax					; dx = ax = 2nd word (AS)
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Data + 2], ax	; on stocke le controle complet
		mov	ebx, ecx			; bx = 1st word (AS)
		shl	eax, 14				; on isole l'adresse
		and	ebx, 0x3FFF			; on isole l'adresse
		and	ecx, 0xC000			; on isole les bits de CD
		or	ebx, eax			; ebx = Address IO VRAM
		and	edx, 0xF0			; on isole les bits de CD
		shr	ecx, 12				;		"		"
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Address], bx	; Ctrl.Address = Address de depart pour le port VDP Data
		or	edx, ecx					; edx = CD
		mov	eax, [SYM(CD_Table) + edx]			; eax = Location & Read/Write
		mov	byte [SYM(VDP_Ctrl) + VDP_Ctrl_t.Flag], 0	; on en a finit avec Address Set
		test	ah, ah						; on teste si il y a transfert DMA
		mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.Access], al		; on stocke l'accés
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
		
		test	byte [SYM(VDP_Reg) + VDP_Reg_t.Set_2], 0x10	; DMA enable ?
		jz	near .NO_DMA
		test	al, 0x4						; DMA FILL ?
		jz	short .No_Fill
			
			cmp	byte [SYM(VDP_Ctrl) + VDP_Ctrl_t.DMA_Mode], 0x80
			jne	short .No_Fill
			mov	[SYM(VDP_Ctrl) + VDP_Ctrl_t.DMA], al	; on stocke le type de DMA
			pop	esi
			pop	edi
			pop	edx
			pop	ecx
			pop	ebx
			ret
	
	align 16
	
	.No_Fill:
		mov 	ecx, [SYM(VDP_Reg) + VDP_Reg_t.DMA_Length]	; ecx = DMA Length
		mov	esi, [SYM(VDP_Reg) + VDP_Reg_t.DMA_Address]	; esi = DMA Source Address / 2
		and	eax, byte 3					; eax = destination DMA (1:VRAM, 2:CRAM, 3:VSRAM)
		and	ecx, 0xFFFF
		mov	edi, [SYM(VDP_Ctrl) + VDP_Ctrl_t.Address]	; edi = Address Dest
		
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
		and	edi, 0xFFFF						; edi = Address Dest
		cmp	byte [SYM(VDP_Ctrl) + VDP_Ctrl_t.DMA_Mode], 0xC0	; DMA Copy ?
		movzx	edx, byte [SYM(VDP_Reg) + VDP_Reg_t.Auto_Inc]	; edx = Auto Inc
		je	near .V_RAM_Copy
	
	.MEM_To_V_RAM:
		add	esi, esi				; esi = DMA Source Address
		test	byte [SYM(VDP_Ctrl) + VDP_Ctrl_t.DMA_Mode], 0x80
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
	
	.V_RAM_Copy:
		or	word [SYM(VDP_Status)], 0x0002
		and	esi, 0xFFFF
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
