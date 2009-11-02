;
; Gens: Main 68000 memory management. (SegaCD)
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

%define CYCLE_FOR_TAKE_Z80_BUS_SEGACD 32

section .bss align=64
	
	extern SYM(Ram_Prg)
	extern SYM(Ram_Word_2M)
	extern SYM(Ram_Word_1M)
	extern SYM(Ram_Word_State)
	
	extern SYM(S68K_Mem_WP)
	extern SYM(Int_Mask_S68K)
	
	extern SYM(COMM.Flag)
	extern SYM(COMM.Command)
	extern SYM(COMM.Status)
	
	extern SYM(CDC.RS0)
	extern SYM(CDC.RS1)
	extern SYM(CDC.Host_Data)
	extern SYM(CDC.DMA_Adr)
	extern SYM(CDC.Stop_Watch)
	
	; Z80 state.
	Z80_STATE_ENABLED	equ (1 << 0)
	Z80_STATE_BUSREQ	equ (1 << 1)
	Z80_STATE_RESET		equ (1 << 2)
	
	extern SYM(M_Z80)
	extern SYM(Z80_State)
	extern SYM(Last_BUS_REQ_Cnt)
	extern SYM(Last_BUS_REQ_St)
	
	extern SYM(Game_Mode)
	extern SYM(CPU_Mode)
	extern SYM(Gen_Version)
	
	extern SYM(Z80_M68K_Cycle_Tab)
	extern SYM(Rom_Data)
	extern SYM(Bank_M68K)
	extern SYM(Fake_Fetch)
	
	extern SYM(Cycles_M68K)
	extern SYM(Cycles_Z80)
	
	global SYM(BRAM_Ex_State)
	SYM(BRAM_Ex_State):
		resd 1
	global SYM(BRAM_Ex_Size)
	SYM(BRAM_Ex_Size):
		resd 1
	
	global SYM(S68K_State)
	SYM(S68K_State):
		resd 1
	global SYM(CPL_S68K)
	SYM(CPL_S68K):
		resd 1
	global SYM(Cycles_S68K)
	SYM(Cycles_S68K):
		resd 1
	
	global SYM(Ram_Backup_Ex)
	SYM(Ram_Backup_Ex):
		resb 64 * 1024
	
section .data align=64
	
	extern SYM(Memory_Control_Status)
	extern SYM(Cell_Conv_Tab)
	
	extern SYM(Controller_1_Counter)
	extern SYM(Controller_1_Delay)
	extern SYM(Controller_1_State)
	extern SYM(Controller_1_COM)
	
	extern SYM(Controller_2_Counter)
	extern SYM(Controller_2_Delay)
	extern SYM(Controller_2_State)
	extern SYM(Controller_2_COM)
	
section .rodata align=64
	; Sega CD Default Jump Table
	
	global SYM(SegaCD_M68K_Read_Byte_Table)
	SYM(SegaCD_M68K_Read_Byte_Table):
		dd	M68K_Read_Byte_Bios_CD,		; 0x000000 - 0x07FFFF
		dd	M68K_Read_Byte_Bad,		; 0x080000 - 0x0FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x100000 - 0x17FFFF
		dd	M68K_Read_Byte_Bad,		; 0x180000 - 0x1FFFFF
		dd	M68K_Read_Byte_WRam,		; 0x200000 - 0x27FFFF
		dd	M68K_Read_Byte_Bad,		; 0x280000 - 0x2FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x300000 - 0x37FFFF
		dd	M68K_Read_Byte_Bad,		; 0x380000 - 0x3FFFFF
		dd	M68K_Read_Byte_BRAM_L,		; 0x400000 - 0x47FFFF
		dd	M68K_Read_Byte_Bad,		; 0x480000 - 0x4FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x500000 - 0x57FFFF
		dd	M68K_Read_Byte_Bad,		; 0x580000 - 0x5FFFFF
		dd	M68K_Read_Byte_BRAM,		; 0x600000 - 0x67FFFF
		dd	M68K_Read_Byte_Bad,		; 0x680000 - 0x6FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x700000 - 0x77FFFF
		dd	M68K_Read_Byte_BRAM_W,		; 0x780000 - 0x7FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x800000 - 0x87FFFF
		dd	M68K_Read_Byte_Bad,		; 0x880000 - 0x8FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x900000 - 0x97FFFF
		dd	M68K_Read_Byte_Bad,		; 0x980000 - 0x9FFFFF
		dd	M68K_Read_Byte_Misc_CD,		; 0xA00000 - 0xA7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xA80000 - 0xAFFFFF
		dd	M68K_Read_Byte_Bad,		; 0xB00000 - 0xB7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xB80000 - 0xBFFFFF
		dd	M68K_Read_Byte_VDP,		; 0xC00000 - 0xC7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xC80000 - 0xCFFFFF
		dd	M68K_Read_Byte_Bad,		; 0xD00000 - 0xD7FFFF
		dd	M68K_Read_Byte_Bad,		; 0xD80000 - 0xDFFFFF
		dd	M68K_Read_Byte_Ram,		; 0xE00000 - 0xE7FFFF
		dd	M68K_Read_Byte_Ram,		; 0xE80000 - 0xEFFFFF
		dd	M68K_Read_Byte_Ram,		; 0xF00000 - 0xF7FFFF
		dd	M68K_Read_Byte_Ram,		; 0xF80000 - 0xFFFFFF
	
	global SYM(SegaCD_M68K_Read_Word_Table)
	SYM(SegaCD_M68K_Read_Word_Table):
		dd	M68K_Read_Word_Bios_CD,		; 0x000000 - 0x07FFFF
		dd	M68K_Read_Word_Bad,		; 0x080000 - 0x0FFFFF
		dd	M68K_Read_Word_Bad,		; 0x100000 - 0x17FFFF
		dd	M68K_Read_Word_Bad,		; 0x180000 - 0x1FFFFF
		dd	M68K_Read_Word_WRam,		; 0x200000 - 0x27FFFF
		dd	M68K_Read_Word_Bad,		; 0x280000 - 0x2FFFFF
		dd	M68K_Read_Word_Bad,		; 0x300000 - 0x37FFFF
		dd	M68K_Read_Word_Bad,		; 0x380000 - 0x3FFFFF
		dd	M68K_Read_Word_BRAM_L,		; 0x400000 - 0x47FFFF
		dd	M68K_Read_Word_Bad,		; 0x480000 - 0x4FFFFF
		dd	M68K_Read_Word_Bad,		; 0x500000 - 0x57FFFF
		dd	M68K_Read_Word_Bad,		; 0x580000 - 0x5FFFFF
		dd	M68K_Read_Word_BRAM,		; 0x600000 - 0x67FFFF
		dd	M68K_Read_Word_Bad,		; 0x680000 - 0x6FFFFF
		dd	M68K_Read_Word_Bad,		; 0x700000 - 0x77FFFF
		dd	M68K_Read_Word_BRAM_W,		; 0x780000 - 0x7FFFFF
		dd	M68K_Read_Word_Bad,		; 0x800000 - 0x87FFFF
		dd	M68K_Read_Word_Bad,		; 0x880000 - 0x8FFFFF
		dd	M68K_Read_Word_Bad,		; 0x900000 - 0x97FFFF
		dd	M68K_Read_Word_Bad,		; 0x980000 - 0x9FFFFF
		dd	M68K_Read_Word_Misc_CD,		; 0xA00000 - 0xA7FFFF
		dd	M68K_Read_Word_Bad,		; 0xA80000 - 0xAFFFFF
		dd	M68K_Read_Word_Bad,		; 0xB00000 - 0xB7FFFF
		dd	M68K_Read_Word_Bad,		; 0xB80000 - 0xBFFFFF
		dd	M68K_Read_Word_VDP,		; 0xC00000 - 0xC7FFFF
		dd	M68K_Read_Word_Bad,		; 0xC80000 - 0xCFFFFF
		dd	M68K_Read_Word_Bad,		; 0xD00000 - 0xD7FFFF
		dd	M68K_Read_Word_Bad,		; 0xD80000 - 0xDFFFFF
		dd	M68K_Read_Word_Ram,		; 0xE00000 - 0xE7FFFF
		dd	M68K_Read_Word_Ram,		; 0xE80000 - 0xEFFFFF
		dd	M68K_Read_Word_Ram,		; 0xF00000 - 0xF7FFFF
		dd	M68K_Read_Word_Ram,		; 0xF80000 - 0xFFFFFF
	
	global SYM(SegaCD_M68K_Write_Byte_Table)
	SYM(SegaCD_M68K_Write_Byte_Table):
		dd	M68K_Write_Byte_Bios_CD,	; 0x000000 - 0x0FFFFF
		dd	M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd	M68K_Write_Byte_WRam,		; 0x200000 - 0x2FFFFF
		dd	M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd	M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd	M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd	M68K_Write_Byte_BRAM,		; 0x600000 - 0x6FFFFF
		dd	M68K_Write_Byte_BRAM_W,		; 0x700000 - 0x7FFFFF
		dd	M68K_Write_Bad,			; 0x800000 - 0x8FFFFF
		dd	M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd	M68K_Write_Byte_Misc_CD,	; 0xA00000 - 0xAFFFFF
		dd	M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd	M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd	M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd	M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd	M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF
	
	global SYM(SegaCD_M68K_Write_Word_Table)
	SYM(SegaCD_M68K_Write_Word_Table):
		dd	M68K_Write_Word_Bios_CD,	; 0x000000 - 0x0FFFFF
		dd	M68K_Write_Bad,			; 0x100000 - 0x1FFFFF
		dd	M68K_Write_Word_WRam,		; 0x200000 - 0x2FFFFF
		dd	M68K_Write_Bad,			; 0x300000 - 0x3FFFFF
		dd	M68K_Write_Bad,			; 0x400000 - 0x4FFFFF
		dd	M68K_Write_Bad,			; 0x500000 - 0x5FFFFF
		dd	M68K_Write_Word_BRAM,		; 0x600000 - 0x6FFFFF
		dd	M68K_Write_Word_BRAM_W,		; 0x700000 - 0x7FFFFF
		dd	M68K_Write_Bad,			; 0x800000 - 0x8FFFFF
		dd	M68K_Write_Bad,			; 0x900000 - 0x9FFFFF
		dd	M68K_Write_Word_Misc_CD,	; 0xA00000 - 0xAFFFFF
		dd	M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd	M68K_Write_Word_VDP,		; 0xC00000 - 0xCFFFFF
		dd	M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd	M68K_Write_Word_Ram,		; 0xE00000 - 0xEFFFFF
		dd	M68K_Write_Word_Ram,		; 0xF00000 - 0xFFFFFF
	
section .text align=64
	
	extern SYM(Z80_ReadB_Table)
	extern SYM(Z80_WriteB_Table)
	
	extern M68K_Read_Byte_Bad
	extern M68K_Read_Byte_VDP
	extern M68K_Read_Byte_Ram
	extern M68K_Read_Word_Bad
	extern M68K_Read_Word_VDP
	extern M68K_Read_Word_Ram
	extern M68K_Write_Bad
	extern M68K_Write_Byte_VDP
	extern M68K_Write_Byte_Ram
	extern M68K_Write_Word_VDP
	extern M68K_Write_Word_Ram
	
	extern SYM(MS68K_Set_Word_Ram)
	extern SYM(M68K_Set_Prg_Ram)
	
	extern SYM(main68k_readOdometer)
	extern SYM(sub68k_reset)
	extern SYM(sub68k_interrupt)
	extern SYM(mdZ80_reset)
	extern z80_Exec
	extern SYM(mdZ80_set_odo)
	
	extern SYM(YM2612_Reset)
	
	extern SYM(Read_CDC_Host_MAIN)
	
	extern SYM(RD_Controller_1)
	extern SYM(RD_Controller_2) 
	extern SYM(WR_Controller_1)
	extern SYM(WR_Controller_2)
	
	; SegaCD extended Read Byte
	; *******************************************
	
	align 64
	
	M68K_Read_Byte_Bios_CD:
		cmp	ebx, 0x1FFFF
		ja	short .Bank_RAM
		
		xor	ebx, 1
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	.Bank_RAM:
		cmp	ebx, 0x3FFFF
		ja	near M68K_Read_Byte_Bad
		
		add	ebx, [SYM(Bank_M68K)]
		cmp	byte [SYM(S68K_State)], 1		; BUS available ?
		je	near M68K_Read_Byte_Bad
		
		xor	ebx, 1
		mov	al, [SYM(Ram_Prg) + ebx - 0x20000]		
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_WRam:
		cmp	ebx, 0x23FFFF
		mov	eax, [SYM(Ram_Word_State)]
		ja	short .bad
		and	eax, 0x3
		jmp	[.Table_Word_Ram + eax * 4]
	
	align 16
	
	.Table_Word_Ram:
		;dd	.Word_Ram_2M, .bad
		dd	.Word_Ram_2M, .Word_Ram_2M
		dd	.Word_Ram_1M_0, .Word_Ram_1M_1
	
	align 16
	
	.Word_Ram_2M:
		xor	ebx, 1
		mov	al, [SYM(Ram_Word_2M) + ebx - 0x200000]
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_0:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_0
		
		xor	ebx, 1
		mov	al, [SYM(Ram_Word_1M) + ebx - 0x200000]
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_1:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_1
		
		xor	ebx, 1
		mov	al, [SYM(Ram_Word_1M) + ebx - 0x200000 + 0x20000]
		pop	ebx
		ret
	
	align 4
	
	.bad:
		mov	al, 0
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_0:
		shr	ebx, 1
		mov	eax, 0
		mov	bx, [SYM(Cell_Conv_Tab) + ebx * 2 - 0x220000]
		adc	eax, 0
		and	ebx, 0xFFFF
		mov	al, [SYM(Ram_Word_1M) + ebx * 2 + eax]
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_1:
		shr	ebx, 1
		mov	eax, 0
		mov	bx, [SYM(Cell_Conv_Tab) + ebx * 2 - 0x220000]
		adc	eax, 0
		and	ebx, 0xFFFF
		mov	al, [SYM(Ram_Word_1M) + ebx * 2 + eax + 0x20000]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_BRAM_L:
		cmp	ebx, 0x400001
		mov	al, 0
		jne	short .bad
		
		mov	al, [SYM(BRAM_Ex_Size)]
		
	.bad:
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_BRAM:
		cmp	ebx, 0x61FFFF
		mov	al, 0
		ja	short .bad
		
		test	word [SYM(BRAM_Ex_State)], 0x100
		jz	short .bad
		
		and	ebx, 0x1FFFF
		shr	ebx, 1
		mov	al, [SYM(Ram_Backup_Ex) + ebx]
	
	.bad:
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_BRAM_W:
		cmp	ebx, 0x7FFFFF
		mov	al, 0
		jne	short .bad
		
		mov	al, [SYM(BRAM_Ex_State)]
	
	.bad:
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_Misc_CD:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_mem
		
		test	byte [SYM(Z80_State)], (Z80_STATE_BUSREQ | Z80_STATE_RESET)
		jnz	near .bad
		
		push	ecx
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		call	[SYM(Z80_ReadB_Table) + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_mem:
		cmp	ebx, 0xA11100
		jne	short .no_busreq
		
		test	byte [SYM(Z80_State)], Z80_STATE_BUSREQ
		jnz	short .z80_on
	
	.z80_off:
		call	SYM(main68k_readOdometer)
		sub	eax, [SYM(Last_BUS_REQ_Cnt)]
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_SEGACD
		ja	short .bus_taken
		
		mov	al, [SYM(Last_BUS_REQ_St)]
		pop	ebx
		or	al, 0x80
		ret
	
	align 4
	
	.bus_taken:
		mov	al, 0x80
		pop	ebx
		ret
	
	align 4
	
	.z80_on:
		mov	al, 0x81
		pop	ebx
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA1000D
		ja	.CD_Reg
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_RB + ebx * 2]
	
	align 16
	
	.Table_IO_RB:
		dd	.MD_Spec, .Pad_1, .Pad_2, .bad
		dd	.CT_Pad_1, .CT_Pad_2, .bad, .bad
	
	align 16
	
	.bad:
		mov	al, 0
		pop	ebx
		ret
	
	align 8
	
	.MD_Spec:
		mov	al, [SYM(Game_Mode)]
		add	al, al
		or	al, [SYM(CPU_Mode)]
		shl	al, 6
		pop	ebx
		or	al, [SYM(Gen_Version)]
		ret
	
	align 8
	
	.Pad_1:
		call	SYM(RD_Controller_1)
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		call	SYM(RD_Controller_2)
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	al, [SYM(Controller_1_COM)]
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	al, [SYM(Controller_2_COM)]
		pop	ebx
		ret
	
	align 16
	
	.CD_Reg:
		cmp	ebx, 0xA12000
		jb	short .bad
		cmp	ebx, 0xA1202F
		ja	short .bad
		
		and	ebx, 0x3F
		jmp	[.Table_Extended_IO + ebx * 4]
	
	align 16

	.Table_Extended_IO:
		dd	.S68K_Ctrl_H, .S68K_Ctrl_L, .Memory_Ctrl_H, .Memory_Ctrl_L
		dd	.CDC_Mode_H, .CDC_Mode_L, .HINT_Vector_H, .HINT_Vector_L
		dd	.CDC_Host_Data_H, .CDC_Host_Data_L, .Unknow, .Unknow
		dd	.Stop_Watch_H, .Stop_Watch_L, .Com_Flag_H, .Com_Flag_L
		dd	.Com_D0_H, .Com_D0_L, .Com_D1_H, .Com_D1_L
		dd	.Com_D2_H, .Com_D2_L, .Com_D3_H, .Com_D3_L
		dd	.Com_D4_H, .Com_D4_L, .Com_D5_H, .Com_D5_L
		dd	.Com_D6_H, .Com_D6_L, .Com_D7_H, .Com_D7_L
		dd	.Com_S0_H, .Com_S0_L, .Com_S1_H, .Com_S1_L
		dd	.Com_S2_H, .Com_S2_L, .Com_S3_H, .Com_S3_L
		dd	.Com_S4_H, .Com_S4_L, .Com_S5_H, .Com_S5_L
		dd	.Com_S6_H, .Com_S6_L, .Com_S7_H, .Com_S7_L
	
	align 16
	
	.S68K_Ctrl_L:
		mov	al, [SYM(S68K_State)]
		pop	ebx
		ret
	
	align 16
	
	.S68K_Ctrl_H:
		mov	al, [SYM(Int_Mask_S68K)]
		and	al, 4
		shl	al, 5
		pop	ebx
		ret
	
	align 16
	
	.Memory_Ctrl_L:
		mov	eax, [SYM(Bank_M68K)]
		mov	ebx, [SYM(Ram_Word_State)]
		shr	eax, 11
		and	ebx, 3
		or	al, [SYM(Memory_Control_Status) + ebx]
		pop	ebx
		ret
	
	align 16
	
	.Memory_Ctrl_H:
		mov	al, [SYM(S68K_Mem_WP)]
		pop	ebx
		ret
	
	align 4
	
	.CDC_Mode_H:
		mov	al, [SYM(CDC.RS0) + 1]
		pop	ebx
		ret
	
	align 4
	
	.CDC_Mode_L:
		mov	al, 0
		pop	ebx
		ret
	
	align 4
	
	.HINT_Vector_H:
		mov	al, [SYM(Rom_Data) + 0x73]
		pop	ebx
		ret
	
	align 4

	.HINT_Vector_L:
		mov	al, [SYM(Rom_Data) + 0x72]
		pop	ebx
		ret
	
	align 4
	
	.CDC_Host_Data_H:
		xor	al, al
		pop	ebx
		ret
	
	align 4
	
	.CDC_Host_Data_L:
		xor	al, al
		pop	ebx
		ret
	
	align 4
	
	.Unknow:
		mov	al, 0
		pop	ebx
		ret
	
	align 4
	
	.Stop_Watch_H:
		mov	al, [SYM(CDC.Stop_Watch) + 3]
		pop	ebx
		ret
	
	align 4
	
	.Stop_Watch_L:
		mov	al, [SYM(CDC.Stop_Watch) + 2]
		pop	ebx
		ret
	
	align 4
	
	.Com_Flag_H:
		mov	al, [SYM(COMM.Flag) + 1]
		pop	ebx
		ret
	
	align 4
	
	.Com_Flag_L:
		mov	al, [SYM(COMM.Flag)]
		pop	ebx
		ret
	
	align 4
	
	.Com_D0_H:
	.Com_D0_L:
	.Com_D1_H:
	.Com_D1_L:
	.Com_D2_H:
	.Com_D2_L:
	.Com_D3_H:
	.Com_D3_L:
	.Com_D4_H:
	.Com_D4_L:
	.Com_D5_H:
	.Com_D5_L:
	.Com_D6_H:
	.Com_D6_L:
	.Com_D7_H:
	.Com_D7_L:
		xor	ebx, 1
		mov	al, [SYM(COMM.Command) + ebx - 0x10]
		pop	ebx
		ret
	
	align 4
	
	.Com_S0_H:
	.Com_S0_L:
	.Com_S1_H:
	.Com_S1_L:
	.Com_S2_H:
	.Com_S2_L:
	.Com_S3_H:
	.Com_S3_L:
	.Com_S4_H:
	.Com_S4_L:
	.Com_S5_H:
	.Com_S5_L:
	.Com_S6_H:
	.Com_S6_L:
	.Com_S7_H:
	.Com_S7_L:
		xor	ebx, 1
		mov	al, [SYM(COMM.Status) + ebx - 0x20]
		pop	ebx
		ret
	
	; SegaCD extended Read Word
	; *******************************************
	
	align 64
	
	M68K_Read_Word_Bios_CD:
		cmp	ebx, 0x1FFFF
		ja	short .Bank_RAM
		
		mov	ax, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	.Bank_RAM:
		cmp	ebx, 0x3FFFF
		ja	near M68K_Read_Word_Bad
		
		add	ebx, [SYM(Bank_M68K)]
		cmp	byte [SYM(S68K_State)], 1		; BUS available ?
		je	near M68K_Read_Byte_Bad
		
		mov	ax, [SYM(Ram_Prg) + ebx - 0x20000]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_WRam:
		cmp	ebx, 0x23FFFF
		mov	eax, [SYM(Ram_Word_State)]
		ja	short .bad
		and	eax, 0x3
		jmp	[.Table_Word_Ram + eax * 4]
	
	align 16
	
	.Table_Word_Ram:
		;dd	.Word_Ram_2M, .bad
		dd	.Word_Ram_2M, .Word_Ram_2M
		dd	.Word_Ram_1M_0, .Word_Ram_1M_1
	
	align 16
	
	.Word_Ram_2M:
		mov	ax, [SYM(Ram_Word_2M) + ebx - 0x200000]
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_0:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_0
		
		mov	ax, [SYM(Ram_Word_1M) + ebx - 0x200000]
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_1:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_1
		
		mov	ax, [SYM(Ram_Word_1M) + ebx - 0x200000 + 0x20000]
		pop	ebx
		ret
	
	align 4
	
	.bad:
		mov	ax, 0
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_0:
		xor	eax, eax
		mov	ax, [SYM(Cell_Conv_Tab) + ebx - 0x220000]
		mov	ax, [SYM(Ram_Word_1M) + eax * 2]
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_1:
		xor	eax, eax
		mov	ax, [SYM(Cell_Conv_Tab) + ebx - 0x220000]
		mov	ax, [SYM(Ram_Word_1M) + eax * 2 + 0x20000]
		pop	ebx
		ret
	
	align 16

	M68K_Read_Word_BRAM_L:
		cmp	ebx, 0x400000
		mov	ax, 0
		jne	short .bad
		
		mov	ax, [SYM(BRAM_Ex_Size)]
	
	.bad:
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_BRAM:
		cmp	ebx, 0x61FFFF
		mov	ax, 0
		ja	short .bad
		
		test	word [SYM(BRAM_Ex_State)], 0x100
		jz	short .bad
		
		and	ebx, 0x1FFFF
		shr	ebx, 1
		mov	ax, [SYM(Ram_Backup_Ex) + ebx]
	
	.bad:
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_BRAM_W:
		cmp	ebx, 0x7FFFFE
		mov	ax, 0
		jne	short .bad
		
		xor	ah, ah
		mov	al, [SYM(BRAM_Ex_State)]
		
	.bad:
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_Misc_CD:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_ram
		
		test	byte [SYM(Z80_State)], (Z80_STATE_BUSREQ | Z80_STATE_RESET)
		jnz	near .bad
		
		push	ecx
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		call	[SYM(Z80_ReadB_Table) + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_ram:
		cmp	ebx, 0xA11100
		jne	short .no_busreq
		
		test	byte [SYM(Z80_State)], Z80_STATE_BUSREQ
		jnz	short .z80_on
	
	.z80_off:
		call	SYM(main68k_readOdometer)
		sub	eax, [SYM(Last_BUS_REQ_Cnt)]
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_SEGACD
		ja	short .bus_taken
		
		mov	al, [SYM(Fake_Fetch)]
		mov	ah, [SYM(Last_BUS_REQ_St)]
		xor	al, 0xFF
		or	ah, 0x80
		mov	[SYM(Fake_Fetch)], al		; fake the next fetched instruction (random)
		pop	ebx
		ret
	
	align 16
	
	.bus_taken:
		mov	al, [SYM(Fake_Fetch)]
		mov	ah, 0x80
		xor	al, 0xFF
		pop	ebx
		mov	[SYM(Fake_Fetch)], al		; fake the next fetched instruction (random)
		ret
	
	align 16
	
	.z80_on:
		mov	al, [SYM(Fake_Fetch)]
		mov	ah, 0x81
		xor	al, 0xFF
		pop	ebx
		mov	[SYM(Fake_Fetch)], al		; fake the next fetched instruction (random)
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA1000D
		ja	.CD_Reg
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_RW + ebx * 2]
	
	align 8
	
	.Table_IO_RW:
		dd	.MD_Spec, .Pad_1, .Pad_2, .bad
		dd	.CT_Pad_1, .CT_Pad_2, .bad, .bad
	
	align 8
	
	.MD_Spec:
		mov	ax, [SYM(Game_Mode)]
		add	ax, ax
		or	ax, [SYM(CPU_Mode)]
		shl	ax, 6
		pop	ebx
		or	ax, [SYM(Gen_Version)]		; on recupere les infos hardware de la machine
		ret
	
	align 8
	
	.Pad_1:
		call	SYM(RD_Controller_1)
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		call	SYM(RD_Controller_2)
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	ax, [SYM(Controller_1_COM)]
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	ax, [SYM(Controller_2_COM)]
		pop	ebx
		ret
	
	align 8
	
	.bad:
		xor	ax, ax
		pop	ebx
		ret
	
	align 16

	.CD_Reg:
		cmp	ebx, 0xA12000
		jb	short .bad
		cmp	ebx, 0xA1202F
		ja	short .bad
		
		and	ebx, 0x3E
		jmp	[.Table_Extended_IO + ebx * 2]
	
	align 16
	
	.Table_Extended_IO:
		dd	.S68K_Ctrl, .Memory_Ctrl, .CDC_Mode, .HINT_Vector
		dd	.CDC_Host_Data, .Unknow, .Stop_Watch, .Com_Flag
		dd	.Com_D0, .Com_D1, .Com_D2, .Com_D3
		dd	.Com_D4, .Com_D5, .Com_D6, .Com_D7
		dd	.Com_S0, .Com_S1, .Com_S2, .Com_S3
		dd	.Com_S4, .Com_S5, .Com_S6, .Com_S7
	
	align 16
	
	.S68K_Ctrl:
		mov	ah, [SYM(Int_Mask_S68K)]
		mov	al, [SYM(S68K_State)]
		and	ah, 4
		shl	ah, 5
		pop	ebx
		ret
	
	align 16
	
	.Memory_Ctrl:
		mov	eax, [SYM(Bank_M68K)]
		mov	ebx, [SYM(Ram_Word_State)]
		shr	eax, 11
		and	ebx, 3
		mov	ah, [SYM(S68K_Mem_WP)]
		or	al, [SYM(Memory_Control_Status) + ebx]
		pop	ebx
		ret
	
	align 8
	
	.CDC_Mode:
		mov	ah, [SYM(CDC.RS0) + 1]
		mov	al, 0
		pop	ebx
		ret
	
	align 8
	
	.HINT_Vector:
		mov	ax, [SYM(Rom_Data) + 0x72]
		pop	ebx
		ret
	
	align 8
	
	.CDC_Host_Data:
		call	SYM(Read_CDC_Host_MAIN)
		pop	ebx
		ret
	
	align 4
	
	.Unknow:
		mov	ax, 0
		pop	ebx
		ret
	
	align 8
	
	.Stop_Watch:
		mov	ax, [SYM(CDC.Stop_Watch) + 2]
		pop	ebx
		ret
	
	align 8
	
	.Com_Flag:
		mov	ax, [SYM(COMM.Flag)]
		pop	ebx
		ret
	
	align 8
	
	.Com_D0:
	.Com_D1:
	.Com_D2:
	.Com_D3:
	.Com_D4:
	.Com_D5:
	.Com_D6:
	.Com_D7:
		mov	ax, [SYM(COMM.Command) + ebx - 0x10]
		pop	ebx
		ret
	
	align 8
	
	.Com_S0:
	.Com_S1:
	.Com_S2:
	.Com_S3:
	.Com_S4:
	.Com_S5:
	.Com_S6:
	.Com_S7:
		mov	ax, [SYM(COMM.Status) + ebx - 0x20]
		pop	ebx
		ret
	
	; SegaCD extended Write Byte
	; *******************************************
	
	align 64
	
	M68K_Write_Byte_Bios_CD:
		cmp	ebx, 0x20000
		jb	short .bad
		cmp	ebx, 0x3FFFF
		ja	short .bad
		
		add	ebx, [SYM(Bank_M68K)]
		cmp	byte [SYM(S68K_State)], 1		; BUS available ?
		je	short .bad
		
		xor	ebx, 1
		mov	[SYM(Ram_Prg) + ebx - 0x20000], al
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Byte_WRam:
		cmp	ebx, 0x23FFFF
		mov	ecx, [SYM(Ram_Word_State)]
		ja	short .bad
		and	ecx, 0x3
		jmp	[.Table_Word_Ram + ecx * 4]
	
	align 16
	
	.Table_Word_Ram:
		;dd	.Word_Ram_2M, .bad
		dd	.Word_Ram_2M, .Word_Ram_2M
		dd	.Word_Ram_1M_0, .Word_Ram_1M_1
	
	align 16
	
	.Word_Ram_2M:
		xor	ebx, 1
		mov	[SYM(Ram_Word_2M) + ebx - 0x200000], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_0:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_0
		xor	ebx, 1
		mov	[SYM(Ram_Word_1M) + ebx - 0x200000], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_1:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_1
		
		xor	ebx, 1
		mov	[SYM(Ram_Word_1M) + ebx - 0x200000 + 0x20000], al
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		mov	ax, 0
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_0:
		shr	ebx, 1
		mov	ecx, 0
		mov	bx, [SYM(Cell_Conv_Tab) + ebx * 2 - 0x220000]
		adc	ecx, 0
		and	ebx, 0xFFFF
		mov	[SYM(Ram_Word_1M) + ebx * 2 + ecx], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_1:
		shr	ebx, 1
		mov	ecx, 0
		mov	bx, [SYM(Cell_Conv_Tab) + ebx * 2 - 0x220000]
		adc	ecx, 0
		and	ebx, 0xFFFF
		mov	[SYM(Ram_Word_1M) + ebx * 2 + ecx + 0x20000], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Byte_BRAM:
		cmp	ebx, 0x61FFFF
		ja	short .bad
		
		cmp	word [SYM(BRAM_Ex_State)], 0x101
		jne	short .bad
		
		and	ebx, 0x1FFFF
		shr	ebx, 1
		mov	[SYM(Ram_Backup_Ex) + ebx], al
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Byte_BRAM_W:
		cmp	ebx, 0x7FFFFF
		jne	short .bad
		
		mov	[SYM(BRAM_Ex_State)], al
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Byte_Misc_CD:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_mem
		
		test	byte [SYM(Z80_State)], (Z80_STATE_BUSREQ | Z80_STATE_RESET)
		jnz	short .bad
		
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		mov	edx, eax
		call	[SYM(Z80_WriteB_Table) + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_mem:
		cmp	ebx, 0xA11100
		jne	near .no_busreq
		
		xor	ecx, ecx
		mov	ah, [SYM(Z80_State)]
		mov	dword [SYM(Controller_1_Counter)], ecx
		test	al, 1	; TODO: Should this be ah, Z80_STATE_ENABLED ?
		mov	dword [SYM(Controller_1_Delay)], ecx
		mov	dword [SYM(Controller_2_Counter)], ecx
		mov	dword [SYM(Controller_2_Delay)], ecx
		jnz	short .deactivated
		
		test	ah, Z80_STATE_BUSREQ
		jnz	short .already_activated
		
		or	ah, Z80_STATE_BUSREQ
		push	edx
		mov	[SYM(Z80_State)], ah
		mov	ebx, [SYM(Cycles_M68K)]
		call	SYM(main68k_readOdometer)
		sub	ebx, eax
		mov	edx, [SYM(Cycles_Z80)]
		mov	ebx, [SYM(Z80_M68K_Cycle_Tab) + ebx * 4]
		sub	edx, ebx
		
		push	edx
		push	SYM(M_Z80)
		call	SYM(mdZ80_set_odo)
		add	esp, 8
		pop	edx
	
	.already_activated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.deactivated:
		call	SYM(main68k_readOdometer)
		mov	cl, [SYM(Z80_State)]
		mov	[SYM(Last_BUS_REQ_Cnt)], eax
		test	cl, Z80_STATE_BUSREQ
		setnz	[SYM(Last_BUS_REQ_St)]
		jz	short .already_deactivated
		
		push	edx
		mov	ebx, [SYM(Cycles_M68K)]
		and	cl, ~Z80_STATE_BUSREQ
		sub	ebx, eax
		mov	[SYM(Z80_State)], cl
		mov	edx, [SYM(Cycles_Z80)]
		mov	ebx, [SYM(Z80_M68K_Cycle_Tab) + ebx * 4]
		mov	ecx, SYM(M_Z80)
		sub	edx, ebx
		call	z80_Exec
		pop	edx
	
	.already_deactivated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA11200
		jne	short .no_reset_z80
		
		test	al, 1
		jnz	short .no_reset
		
		push	edx
		
		push	SYM(M_Z80)
		call	SYM(mdZ80_reset)
		add	esp, 4
		
		or	byte [SYM(Z80_State)], Z80_STATE_RESET
		call	SYM(YM2612_Reset)
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	.no_reset:
		and	byte [SYM(Z80_State)], ~Z80_STATE_RESET
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_reset_z80:
		cmp	ebx, 0xA1000D
		ja	.no_ctrl_io
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_WB + ebx * 2]
	
	align 16
	
	.Table_IO_WB:
		dd	.bad, .Pad_1, .Pad_2, .bad
		dd	.CT_Pad_1, .CT_Pad_2, .bad, .bad
	
	align 16
	
	.Pad_1:
		push	eax
		call	SYM(WR_Controller_1)
		add	esp, 4
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		push	eax
		call	SYM(WR_Controller_2)
		add	esp, 4
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	[SYM(Controller_1_COM)], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	[SYM(Controller_2_COM)], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_ctrl_io:
		cmp	ebx, 0xA12000
		jb	near M68K_Write_Bad
		cmp	ebx, 0xA1202F
		ja	near M68K_Write_Bad
		
		and	ebx, 0x3F
		jmp	[.Table_Extended_IO + ebx * 4]
	
	align 16
	
	.Table_Extended_IO:
		dd	.S68K_Ctrl_H, .S68K_Ctrl_L, .Memory_Ctrl_H, .Memory_Ctrl_L
		dd	.CDC_Mode_H, .CDC_Mode_L, .HINT_Vector_H, .HINT_Vector_L
		dd	.CDC_Host_Data_H, .CDC_Host_Data_L, .Unknow, .Unknow
		dd	.Stop_Watch_H, .Stop_Watch_L, .Com_Flag_H, .Com_Flag_L
		dd	.Com_D0_H, .Com_D0_L, .Com_D1_H, .Com_D1_L
		dd	.Com_D2_H, .Com_D2_L, .Com_D3_H, .Com_D3_L
		dd	.Com_D4_H, .Com_D4_L, .Com_D5_H, .Com_D5_L
		dd	.Com_D6_H, .Com_D6_L, .Com_D7_H, .Com_D7_L
		dd	.Com_S0_H, .Com_S0_L, .Com_S1_H, .Com_S1_L
		dd	.Com_S2_H, .Com_S2_L, .Com_S3_H, .Com_S3_L
		dd	.Com_S4_H, .Com_S4_L, .Com_S5_H, .Com_S5_L
		dd	.Com_S6_H, .Com_S6_L, .Com_S7_H, .Com_S7_L
	
	align 16
	
	.S68K_Ctrl_L:
		test	al, 1
		jz	short .S68K_Reseting
		test	byte [SYM(S68K_State)], 1
		jnz	short .S68K_Already_Running
	
	.S68K_Restart:
		push	eax
		call	SYM(sub68k_reset)
		pop	eax
	
	.S68K_Reseting:
	.S68K_Already_Running:
		and	al, 3
		mov	[SYM(S68K_State)], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.S68K_Ctrl_H:
		test	al, 0x1
		jz	.No_Process_INT2
		test	byte [SYM(Int_Mask_S68K)], 0x4
		jz	.No_Process_INT2
		
		push	dword -1
		push	dword 2
		call	SYM(sub68k_interrupt)
		add	esp, 8
	
	.No_Process_INT2:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Memory_Ctrl_L:
		;pushad
		;push	eax
		;push	ebx
		;add	dword [esp], 0xA12000
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		mov	ebx, eax
		shr	eax, 1
		and	ebx, 0xC0
		test	byte [SYM(Ram_Word_State)], 0x2
		jnz	short .Mode_1M
	
	.Mode_2M:
		shl	ebx, 11
		test	al, 1
		mov	[SYM(Bank_M68K)], ebx
		jz	short .No_DMNA
		
		mov	byte [SYM(Ram_Word_State)], 1
		call	SYM(MS68K_Set_Word_Ram)
	
	.No_DMNA:
		call	SYM(M68K_Set_Prg_Ram)
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Mode_1M:
		shl	ebx, 11
		test	al, 1
		jnz	short .no_swap
		
		or	word [SYM(Memory_Control_Status) + 2], 0x0202		; DMNA bit = 1
	
	.no_swap:
		mov	[SYM(Bank_M68K)], ebx
		call	SYM(M68K_Set_Prg_Ram)
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Memory_Ctrl_H:
		mov	[SYM(S68K_Mem_WP)], al
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.CDC_Mode_H:
	.CDC_Mode_L:
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.HINT_Vector_H:
		mov	[SYM(Rom_Data) + 0x73], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.HINT_Vector_L:
		mov	[SYM(Rom_Data) + 0x72], al
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.CDC_Host_Data_H:
	.CDC_Host_Data_L:
	.Unknow:
	.Stop_Watch_H:
	.Stop_Watch_L:
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Com_Flag_H:
		;pushad
		;push	eax
		;push	ebx
		;add	dword [esp], 0xA12000
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		mov	[SYM(COMM.Flag) + 1], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Com_Flag_L:
		;pushad
		;push	eax
		;push	ebx
		;add	dword [esp], 0xA12000
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		rol	al, 1
		mov	byte [SYM(COMM.Flag) + 1], al
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Com_D0_H:
	.Com_D0_L:
	.Com_D1_H:
	.Com_D1_L:
	.Com_D2_H:
	.Com_D2_L:
	.Com_D3_H:
	.Com_D3_L:
	.Com_D4_H:
	.Com_D4_L:
	.Com_D5_H:
	.Com_D5_L:
	.Com_D6_H:
	.Com_D6_L:
	.Com_D7_H:
	.Com_D7_L:
		;pushad
		;push	eax
		;push	ebx
		;add	dword [esp], 0xA12000
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		xor	ebx, 1
		mov	[SYM(COMM.Command) + ebx - 0x10], al
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.Com_S0_H:
	.Com_S0_L:
	.Com_S1_H:
	.Com_S1_L:
	.Com_S2_H:
	.Com_S2_L:
	.Com_S3_H:
	.Com_S3_L:
	.Com_S4_H:
	.Com_S4_L:
	.Com_S5_H:
	.Com_S5_L:
	.Com_S6_H:
	.Com_S6_L:
	.Com_S7_H:
	.Com_S7_L:
		pop	ecx
		pop	ebx
		ret
	
	; SegaCD extended Write Word
	; *******************************************
	
	align 64
	
	M68K_Write_Word_Bios_CD:
		cmp	ebx, 0x20000
		jb	short .bad
		cmp	ebx, 0x3FFFF
		ja	short .bad
		
		add	ebx, [SYM(Bank_M68K)]
		cmp	byte [SYM(S68K_State)], 1		; BUS available ?
		je	short .bad
		
		mov	[SYM(Ram_Prg) + ebx - 0x20000], ax
		
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Word_WRam:
		cmp	ebx, 0x23FFFF
		mov	ecx, [SYM(Ram_Word_State)]
		ja	short .bad
		and	ecx, 0x3
		jmp	[.Table_Word_Ram + ecx * 4]
	
	align 16
	
	.Table_Word_Ram:
		dd	.Word_Ram_2M, .Word_Ram_2M
		dd	.Word_Ram_1M_0, .Word_Ram_1M_1
	
	align 16
	
	.Word_Ram_2M:
		mov	[SYM(Ram_Word_2M) + ebx - 0x200000], ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_0:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_0
		
		mov	[SYM(Ram_Word_1M) + ebx - 0x200000], ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Word_Ram_1M_1:
		cmp	ebx, 0x21FFFF
		ja	short .Cell_Arranged_1
		
		mov	[SYM(Ram_Word_1M) + ebx  - 0x200000 + 0x20000], ax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_0:
		mov	bx, [SYM(Cell_Conv_Tab) + ebx - 0x220000]
		and	ebx, 0xFFFF
		pop	ecx
		mov	[SYM(Ram_Word_1M) + ebx * 2], ax
		pop	ebx
		ret
	
	align 16
	
	.Cell_Arranged_1:
		mov	bx, [SYM(Cell_Conv_Tab) + ebx - 0x220000]
		and	ebx, 0xFFFF
		pop	ecx
		mov	[SYM(Ram_Word_1M) + ebx * 2 + 0x20000], ax
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Word_BRAM:
		cmp	ebx, 0x61FFFF
		ja	short .bad
		
		cmp	word [SYM(BRAM_Ex_State)], 0x101
		jne	short .bad
		
		and	ebx, 0x1FFFE
		shr	ebx, 1
		mov	[SYM(Ram_Backup_Ex) + ebx], ax
		
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Word_BRAM_W:
		cmp	ebx, 0x7FFFFE
		jne	short .bad
		
		mov	[SYM(BRAM_Ex_State)], al
		
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Word_Misc_CD:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_ram
		
		test	byte [SYM(Z80_State)], (Z80_STATE_BUSREQ | Z80_STATE_RESET)
		jnz	near .bad
		
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		mov	dh, al		; Potential bug: SYM(Z80_WriteB_Table) uses FASTCALL; this overwrites the "data" parameter.
		shr	ebx, 10
		mov	dl, al		; Potential bug: SYM(Z80_WriteB_Table) uses FASTCALL; this overwrites the "data" parameter.
		call	[SYM(Z80_WriteB_Table) + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_Z80_ram:
		cmp	ebx, 0xA11100
		jne	near .no_busreq
		
		xor	ecx, ecx
		mov	al, [SYM(Z80_State)]
		mov	dword [SYM(Controller_1_Counter)], ecx
		test	ah, 1	; TODO: Should this be al, Z80_STATE_ENABLED ?
		mov	dword [SYM(Controller_1_Delay)], ecx
		mov	dword [SYM(Controller_2_Counter)], ecx
		mov	dword [SYM(Controller_2_Delay)], ecx
		jnz	short .deactivated
		
		test	al, Z80_STATE_BUSREQ
		jnz	short .already_activated
		
		or	al, Z80_STATE_BUSREQ
		push	edx
		mov	[SYM(Z80_State)], al
		mov	ebx, [SYM(Cycles_M68K)]
		call	SYM(main68k_readOdometer)
		sub	ebx, eax
		mov	edx, [SYM(Cycles_Z80)]
		mov	ebx, [SYM(Z80_M68K_Cycle_Tab) + ebx * 4]
		sub	edx, ebx
		
		push	edx
		push	SYM(M_Z80)
		call	SYM(mdZ80_set_odo)
		add	esp, 8
		pop	edx
	
	.already_activated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.deactivated:
		call	SYM(main68k_readOdometer)
		mov	cl, [SYM(Z80_State)]
		mov	[SYM(Last_BUS_REQ_Cnt)], eax
		test	cl, Z80_STATE_BUSREQ
		setnz	[SYM(Last_BUS_REQ_St)]
		jz	short .already_deactivated
		
		push	edx
		mov	ebx, [SYM(Cycles_M68K)]
		and	cl, ~Z80_STATE_BUSREQ
		sub	ebx, eax
		mov	[SYM(Z80_State)], cl
		mov	edx, [SYM(Cycles_Z80)]
		mov	ebx, [SYM(Z80_M68K_Cycle_Tab) + ebx * 4]
		mov	ecx, SYM(M_Z80)
		sub	edx, ebx
		call	z80_Exec
		pop	edx
	
	.already_deactivated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_busreq:
		cmp	ebx, 0xA11200
		jne	short .no_reset_z80
		
		test	ah, 1
		jnz	short .no_reset
		
		push	edx
		
		push	SYM(M_Z80)
		call	SYM(mdZ80_reset)
		add	esp, 4
		
		or	byte [SYM(Z80_State)], Z80_STATE_RESET
		call	SYM(YM2612_Reset)
		pop	edx
		pop	ecx
		pop	ebx
		ret
	
	.no_reset:
		and	byte [SYM(Z80_State)], ~Z80_STATE_RESET
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_reset_z80:
		cmp	ebx, 0xA1000D
		ja	.no_ctrl_io
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_WW + ebx * 2]
	
	align 16
	
	.Table_IO_WW:
		dd	.bad, .Pad_1, .Pad_2, .bad
		dd	.CT_Pad_1, .CT_Pad_2, .bad, .bad
	
	align 16
	
	.Pad_1:
		push	eax
		call	SYM(WR_Controller_1)
		add	esp, 4
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		push	eax
		call	SYM(WR_Controller_2)
		add	esp, 4
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	[SYM(Controller_1_COM)], ax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	[SYM(Controller_2_COM)], ax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_ctrl_io:
		cmp	ebx, 0xA12000
		jb	short .bad
		cmp	ebx, 0xA1202F
		ja	short .bad
		
		and	ebx, 0x3E
		jmp	[.Table_Extended_IO + ebx * 2]
	
	align 16
	
	.Table_Extended_IO:
		dd	.S68K_Ctrl, .Memory_Ctrl, .CDC_Mode, .HINT_Vector
		dd	.CDC_Host_Data, .Unknow, .Stop_Watch, .Com_Flag
		dd	.Com_D0, .Com_D1, .Com_D2, .Com_D3
		dd	.Com_D4, .Com_D5, .Com_D6, .Com_D7
		dd	.Com_S0, .Com_S1, .Com_S2, .Com_S3
		dd	.Com_S4, .Com_S5, .Com_S6, .Com_S7
	
	align 16
	
	.S68K_Ctrl:
		test	al, 1
		jz	short .S68K_Reseting
		test	byte [SYM(S68K_State)], 1
		jnz	short .S68K_Already_Running
	
	.S68K_Restart:
		push	eax
		call	SYM(sub68k_reset)
		pop 	ax
	
	.S68K_Reseting:
	.S68K_Already_Running:
		and	al, 3
		test	ah, 1
		mov	[SYM(S68K_State)], al
		jz	short .No_Process_INT2
		test	byte [SYM(Int_Mask_S68K)], 0x4
		jz	short .No_Process_INT2
		
		push	dword -1
		push	dword 2
		call	SYM(sub68k_interrupt)
		add	esp, 8
	
	.No_Process_INT2:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Memory_Ctrl:
		;pushad
		;push	eax
		;push	ebx
		;add	dword [esp], 0xA12000
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		mov	[SYM(S68K_Mem_WP)], ah
		mov	ebx, eax
		shr	eax, 1
		and	ebx, 0xC0
		test	byte [SYM(Ram_Word_State)], 0x2
		jnz	short .Mode_1M
	
	.Mode_2M:
		shl	ebx, 11
		test	al, 1
		mov	[SYM(Bank_M68K)], ebx
		jz	short .No_DMNA
		
		mov	byte [SYM(Ram_Word_State)], 1
		call	SYM(MS68K_Set_Word_Ram)
	
	.No_DMNA:
		call	SYM(M68K_Set_Prg_Ram)
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.Mode_1M:
		shl	ebx, 11
		test	al, 1
		jnz	short .no_swap
		
		or	word [SYM(Memory_Control_Status) + 2], 0x0202		; DMNA bit = 1
	
	.no_swap:
		mov	[SYM(Bank_M68K)], ebx
		call	SYM(M68K_Set_Prg_Ram)
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.CDC_Mode:
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.HINT_Vector:
		mov	[SYM(Rom_Data) + 0x72], ax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.CDC_Host_Data:
	.Unknow:
	.Stop_Watch:
		pop	ecx
		pop	ebx
		ret
	
	align 8

	.Com_Flag:
		;pushad
		;push	eax
		;push	ebx
		;add	dword [esp], 0xA12000
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		mov	[SYM(COMM.Flag) + 1], ah
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Com_D0:
	.Com_D1:
	.Com_D2:
	.Com_D3:
	.Com_D4:
	.Com_D5:
	.Com_D6:
	.Com_D7:
		;pushad
		;push	eax
		;push	ebx
		;add	dword [esp], 0xA12000
		;call	_Write_To_68K_Space
		;pop	ebx
		;pop	eax
		;popad
		
		mov	[SYM(COMM.Command) + ebx - 0x10], ax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.Com_S0:
	.Com_S1:
	.Com_S2:
	.Com_S3:
	.Com_S4:
	.Com_S5:
	.Com_S6:
	.Com_S7:
		pop	ecx
		pop	ebx
		ret
