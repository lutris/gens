;
; Gens: Main 68000 memory management.
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

%define CYCLE_FOR_TAKE_Z80_BUS_GENESIS 16

section .data align=64
	
	extern SYM(Controller_1_Counter)
	extern SYM(Controller_1_Delay)
	extern SYM(Controller_1_State)
	extern SYM(Controller_1_COM)
	
	extern SYM(Controller_2_Counter)
	extern SYM(Controller_2_Delay)
	extern SYM(Controller_2_State)
	extern SYM(Controller_2_COM)
	
	; Current Main 68000 Jump Table
	
	global SYM(M68K_Read_Byte_Table)
	SYM(M68K_Read_Byte_Table):
		times 32	dd M68K_Read_Byte_Bad
	
	global SYM(M68K_Read_Word_Table)
	SYM(M68K_Read_Word_Table):
		times 32	dd M68K_Read_Byte_Bad
	
	global SYM(M68K_Write_Byte_Table)
	SYM(M68K_Write_Byte_Table):
		times 16	dd M68K_Write_Bad
	
	global SYM(M68K_Write_Word_Table)
	SYM(M68K_Write_Word_Table):
		times 16	dd M68K_Write_Bad
	
section .rodata align=64
	
	global SYM(Genesis_M68K_Read_Byte_Table)
	SYM(Genesis_M68K_Read_Byte_Table):
		dd	M68K_Read_Byte_Rom0,		; 0x000000 - 0x07FFFF
		dd	M68K_Read_Byte_Rom1,		; 0x080000 - 0x0FFFFF
		dd	M68K_Read_Byte_Rom2,		; 0x100000 - 0x17FFFF
		dd	M68K_Read_Byte_Rom3,		; 0x180000 - 0x1FFFFF
		dd	M68K_Read_Byte_Rom4,		; 0x200000 - 0x27FFFF
		dd	M68K_Read_Byte_Rom5,		; 0x280000 - 0x2FFFFF
		dd	M68K_Read_Byte_Rom6,		; 0x300000 - 0x37FFFF
		dd	M68K_Read_Byte_Rom7,		; 0x380000 - 0x3FFFFF
		dd	M68K_Read_Byte_Rom8,		; 0x400000 - 0x47FFFF
		dd	M68K_Read_Byte_Rom9,		; 0x480000 - 0x4FFFFF
		dd	M68K_Read_Byte_RomA,		; 0x500000 - 0x57FFFF
		dd	M68K_Read_Byte_RomB,		; 0x580000 - 0x5FFFFF
		dd	M68K_Read_Byte_RomC,		; 0x600000 - 0x67FFFF
		dd	M68K_Read_Byte_RomD,		; 0x680000 - 0x6FFFFF
		dd	M68K_Read_Byte_RomE,		; 0x700000 - 0x77FFFF
		dd	M68K_Read_Byte_RomF,		; 0x780000 - 0x7FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x800000 - 0x87FFFF
		dd	M68K_Read_Byte_Bad,		; 0x880000 - 0x8FFFFF
		dd	M68K_Read_Byte_Bad,		; 0x900000 - 0x97FFFF
		dd	M68K_Read_Byte_Bad,		; 0x980000 - 0x9FFFFF
		dd	M68K_Read_Byte_Misc,		; 0xA00000 - 0xA7FFFF
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
	
	global SYM(Genesis_M68K_Read_Word_Table)
	SYM(Genesis_M68K_Read_Word_Table):
		dd	M68K_Read_Word_Rom0,		; 0x000000 - 0x07FFFF
		dd	M68K_Read_Word_Rom1,		; 0x080000 - 0x0FFFFF
		dd	M68K_Read_Word_Rom2,		; 0x100000 - 0x17FFFF
		dd	M68K_Read_Word_Rom3,		; 0x180000 - 0x1FFFFF
		dd	M68K_Read_Word_Rom4,		; 0x200000 - 0x27FFFF
		dd	M68K_Read_Word_Rom5,		; 0x280000 - 0x2FFFFF
		dd	M68K_Read_Word_Rom6,		; 0x300000 - 0x37FFFF
		dd	M68K_Read_Word_Rom7,		; 0x380000 - 0x3FFFFF
		dd	M68K_Read_Word_Rom8,		; 0x400000 - 0x47FFFF
		dd	M68K_Read_Word_Rom9,		; 0x480000 - 0x4FFFFF
		dd	M68K_Read_Word_RomA,		; 0x500000 - 0x57FFFF
		dd	M68K_Read_Word_RomB,		; 0x580000 - 0x5FFFFF
		dd	M68K_Read_Word_RomC,		; 0x600000 - 0x67FFFF
		dd	M68K_Read_Word_RomD,		; 0x680000 - 0x6FFFFF
		dd	M68K_Read_Word_RomE,		; 0x700000 - 0x77FFFF
		dd	M68K_Read_Word_RomF,		; 0x780000 - 0x7FFFFF
		dd	M68K_Read_Word_Bad,		; 0x800000 - 0x87FFFF
		dd	M68K_Read_Word_Bad,		; 0x880000 - 0x8FFFFF
		dd	M68K_Read_Word_Bad,		; 0x900000 - 0x97FFFF
		dd	M68K_Read_Word_Bad,		; 0x980000 - 0x9FFFFF
		dd	M68K_Read_Word_Misc,		; 0xA00000 - 0xA7FFFF
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
	
	global SYM(Genesis_M68K_Write_Byte_Table)
	SYM(Genesis_M68K_Write_Byte_Table):
		dd	M68K_Write_Byte_SRAM,		; 0x000000 - 0x0FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x100000 - 0x1FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x200000 - 0x2FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x300000 - 0x3FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x400000 - 0x4FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x500000 - 0x5FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x600000 - 0x6FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x700000 - 0x7FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x800000 - 0x8FFFFF
		dd	M68K_Write_Byte_SRAM,		; 0x900000 - 0x9FFFFF
		dd	M68K_Write_Byte_Misc,		; 0xA00000 - 0xAFFFFF
		dd	M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd	M68K_Write_Byte_VDP,		; 0xC00000 - 0xCFFFFF
		dd	M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd	M68K_Write_Byte_Ram,		; 0xE00000 - 0xEFFFFF
		dd	M68K_Write_Byte_Ram,		; 0xF00000 - 0xFFFFFF
	
	global SYM(Genesis_M68K_Write_Word_Table)
	SYM(Genesis_M68K_Write_Word_Table):
		dd	M68K_Write_Word_SRAM,		; 0x000000 - 0x0FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x100000 - 0x1FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x200000 - 0x2FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x300000 - 0x3FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x400000 - 0x4FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x500000 - 0x5FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x600000 - 0x6FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x700000 - 0x7FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x800000 - 0x8FFFFF
		dd	M68K_Write_Word_SRAM,		; 0x900000 - 0x9FFFFF
		dd	M68K_Write_Word_Misc,		; 0xA00000 - 0xAFFFFF
		dd	M68K_Write_Bad,			; 0xB00000 - 0xBFFFFF
		dd	M68K_Write_Word_VDP,		; 0xC00000 - 0xCFFFFF
		dd	M68K_Write_Bad,			; 0xD00000 - 0xDFFFFF
		dd	M68K_Write_Word_Ram,		; 0xE00000 - 0xEFFFFF
		dd	M68K_Write_Word_Ram,		; 0xF00000 - 0xFFFFFF
	
	; SegaCD Jump Tables
	extern SYM(SegaCD_M68K_Read_Byte_Table)
	extern SYM(SegaCD_M68K_Read_Word_Table)
	extern SYM(SegaCD_M68K_Write_Byte_Table)
	extern SYM(SegaCD_M68K_Write_Word_Table)
	
	; 32X Jump Tables
	extern SYM(_32X_M68K_Read_Byte_Table)
	extern SYM(_32X_M68K_Read_Word_Table)
	extern SYM(_32X_M68K_Write_Byte_Table)
	extern SYM(_32X_M68K_Write_Word_Table)
	
section .bss align=64
	
	extern SYM(M_Z80)
	
	global SYM(Ram_68k)
	SYM(Ram_68k):
		resb 64 * 1024
	
	global SYM(Rom_Data)
	SYM(Rom_Data):
		resb 6 * 1024 * 1024
	
	; Genesis firmware
	global SYM(Genesis_Rom)
	SYM(Genesis_Rom):
		resb 2 * 1024
	
	global SYM(Rom_Size)
	SYM(Rom_Size):
		resd 1
	
	; Save RAM
	global SYM(SRAM)
	SYM(SRAM):
		resb 64 * 1024
	global SYM(SRAM_Start)
	SYM(SRAM_Start):
		resd 1
	global SYM(SRAM_End)
	SYM(SRAM_End):
		resd 1
	global SYM(SRAM_ON)
	SYM(SRAM_ON):
		resd 1
	global SYM(SRAM_Write)
	SYM(SRAM_Write):
		resd 1
	global SYM(SRAM_Custom)
	SYM(SRAM_Custom):
		resd 1
	global SYM(SRAM_Enabled)
	SYM(SRAM_Enabled):
		resd 1
	
	alignb 64
	
	global SYM(Z80_M68K_Cycle_Tab)
	SYM(Z80_M68K_Cycle_Tab):
		resd 512
	
	; Z80 state.
	Z80_STATE_ENABLED	equ (1 << 0)
	Z80_STATE_BUSREQ	equ (1 << 1)
	Z80_STATE_RESET		equ (1 << 2)
	
	global SYM(Z80_State)
	SYM(Z80_State):
		resd 1
	global SYM(Last_BUS_REQ_Cnt)
	SYM(Last_BUS_REQ_Cnt):
		resd 1
	global SYM(Last_BUS_REQ_St)
	SYM(Last_BUS_REQ_St):
		resd 1
	global SYM(Bank_M68K)
	SYM(Bank_M68K):
		resd 1
	global SYM(Fake_Fetch)
	SYM(Fake_Fetch):
		resd 1
	
	global SYM(Game_Mode)
	SYM(Game_Mode):
		resd 1
	global SYM(CPU_Mode)
	SYM(CPU_Mode):
		resd 1
	global SYM(Gen_Mode)	; NOTE: This is unused! (except in GSXv7)
	SYM(Gen_Mode):
		resd 1
	global SYM(Gen_Version)
	SYM(Gen_Version):
		resd 1
	
	global SYM(CPL_M68K)
	SYM(CPL_M68K):
		resd 1
	global SYM(CPL_Z80)
	SYM(CPL_Z80):
		resd 1
	global SYM(Cycles_M68K)
	SYM(Cycles_M68K):
		resd 1
	global SYM(Cycles_Z80)
	SYM(Cycles_Z80):
		resd 1
	
section .text align=64
	
	extern SYM(Z80_ReadB_Table)
	extern SYM(Z80_WriteB_Table)
	
	extern SYM(Read_VDP_Data)
	extern SYM(Read_VDP_Status)
	extern SYM(Read_VDP_V_Counter) 
	extern SYM(Read_VDP_H_Counter)
	extern SYM(Write_Byte_VDP_Data)
	extern SYM(Write_Word_VDP_Data)
	extern SYM(Write_VDP_Ctrl)
	
	extern SYM(main68k_readOdometer)
	extern SYM(mdZ80_reset)
	extern z80_Exec
	extern SYM(mdZ80_set_odo)
	extern SYM(YM2612_Reset)
	extern SYM(PSG_Write)
	
	extern SYM(RD_Controller_1)
	extern SYM(RD_Controller_2) 
	extern SYM(WR_Controller_1)
	extern SYM(WR_Controller_2)
	
	;void Init_Memory_M68K(int System_ID)
	global SYM(Init_Memory_M68K)
	SYM(Init_Memory_M68K):
		
		push	eax
		push	ebx
		mov	ebx, 15
		cmp	byte [esp + 12], 1
		ja	near .SegaCD
		je	near ._32X
		jmp	short .Genesis
	
	align 16
	
	.Genesis:
			mov	eax, [SYM(Genesis_M68K_Read_Byte_Table) + ebx * 8]
			mov	[SYM(M68K_Read_Byte_Table) + ebx * 8], eax
			mov	eax, [SYM(Genesis_M68K_Read_Byte_Table) + ebx * 8 + 4]
			mov	[SYM(M68K_Read_Byte_Table) + ebx * 8 + 4], eax
			mov	eax, [SYM(Genesis_M68K_Read_Word_Table) + ebx * 8]
			mov	[SYM(M68K_Read_Word_Table) + ebx * 8], eax
			mov	eax, [SYM(Genesis_M68K_Read_Word_Table) + ebx * 8 + 4]
			mov	[SYM(M68K_Read_Word_Table) + ebx * 8 + 4], eax
			
			mov	eax, [SYM(Genesis_M68K_Write_Byte_Table) + ebx * 4]
			mov	[SYM(M68K_Write_Byte_Table) + ebx * 4], eax
			mov	eax, [SYM(Genesis_M68K_Write_Word_Table) + ebx * 4]
			mov	[SYM(M68K_Write_Word_Table) + ebx * 4], eax
			
			dec	ebx
			jns	short .Genesis
		
		pop	ebx
		pop	eax
		ret
	
	align 16
	
	._32X:
			mov	eax, [SYM(_32X_M68K_Read_Byte_Table) + ebx * 8]
			mov	[SYM(M68K_Read_Byte_Table) + ebx * 8], eax
			mov	eax, [SYM(_32X_M68K_Read_Byte_Table) + ebx * 8 + 4]
			mov	[SYM(M68K_Read_Byte_Table) + ebx * 8 + 4], eax
			mov	eax, [SYM(_32X_M68K_Read_Word_Table) + ebx * 8]
			mov	[SYM(M68K_Read_Word_Table) + ebx * 8], eax
			mov	eax, [SYM(_32X_M68K_Read_Word_Table) + ebx * 8 + 4]
			mov	[SYM(M68K_Read_Word_Table) + ebx * 8 + 4], eax
			
			mov	eax, [SYM(_32X_M68K_Write_Byte_Table) + ebx * 4]
			mov	[SYM(M68K_Write_Byte_Table) + ebx * 4], eax
			mov	eax, [SYM(_32X_M68K_Write_Word_Table) + ebx * 4]
			mov	[SYM(M68K_Write_Word_Table) + ebx * 4], eax
			
			dec	ebx
			jns	short ._32X

		mov	eax, [SYM(_32X_M68K_Read_Byte_Table) + 6 * 8]
		mov	[SYM(M68K_Read_Byte_Table) + 8 * 8 - 4], eax
		mov	eax, [SYM(_32X_M68K_Read_Word_Table) + 6 * 8]
		mov	[SYM(M68K_Read_Word_Table) + 8 * 8 - 4], eax
		
		mov	eax, [SYM(_32X_M68K_Write_Byte_Table) + 6 * 4]
		mov	[SYM(M68K_Write_Byte_Table) + 8 * 4 + 4], eax
		mov	eax, [SYM(_32X_M68K_Write_Word_Table) + 6 * 4]
		mov	[SYM(M68K_Write_Word_Table) + 8 * 4 + 4], eax
		
		pop	ebx
		pop	eax
		ret
	
	align 16
	
	.SegaCD:
			mov	eax, [SYM(SegaCD_M68K_Read_Byte_Table) + ebx * 8]
			mov	[SYM(M68K_Read_Byte_Table) + ebx * 8], eax
			mov	eax, [SYM(SegaCD_M68K_Read_Byte_Table) + ebx * 8 + 4]
			mov	[SYM(M68K_Read_Byte_Table) + ebx * 8 + 4], eax
			mov	eax, [SYM(SegaCD_M68K_Read_Word_Table) + ebx * 8]
			mov	[SYM(M68K_Read_Word_Table) + ebx * 8], eax
			mov	eax, [SYM(SegaCD_M68K_Read_Word_Table) + ebx * 8 + 4]
			mov	[SYM(M68K_Read_Word_Table) + ebx * 8 + 4], eax
			
			mov	eax, [SYM(SegaCD_M68K_Write_Byte_Table) + ebx * 4]
			mov	[SYM(M68K_Write_Byte_Table) + ebx * 4], eax
			mov	eax, [SYM(SegaCD_M68K_Write_Word_Table) + ebx * 4]
			mov	[SYM(M68K_Write_Word_Table) + ebx * 4], eax
			
			dec	ebx
			jns	short .SegaCD
		
		pop	ebx
		pop	eax
		ret
	
	align 64
	
	;unsigned char M68K_RB(unsigned int Adr)
	global SYM(M68K_RB)
	SYM(M68K_RB):
		
		mov	eax, [esp + 4]		; Address
		push	ebx
		
		mov	ebx, eax
		and	eax, 0xF80000
		shr	eax, 17
		and	ebx, 0xFFFFFF
		jmp	[SYM(M68K_Read_Byte_Table) + eax]
	
	align 64
	
	;unsigned short M68K_RW(unsigned int Adr)
	global SYM(M68K_RW)
	SYM(M68K_RW):
		
		mov	eax, [esp + 4]		; Address
		push	ebx
		
		mov	ebx, eax
		and	eax, 0xF80000
		shr	eax, 17
		and	ebx, 0xFFFFFF
		jmp	[SYM(M68K_Read_Word_Table) + eax]
	
	align 64
	
	;void M68K_WB(unsigned int Adr, unsigned char Data)
	global SYM(M68K_WB)
	SYM(M68K_WB)
		
		push	ebx
		push	ecx
		
		mov	ecx, [esp + 12]		; Address
		mov	eax, [esp + 16]		; Data
		mov	ebx, ecx
		and	ecx, 0xF00000
		and	eax, 0xFF
		shr	ecx, 18
		and	ebx, 0xFFFFFF
		jmp	[SYM(M68K_Write_Byte_Table) + ecx]
	
	align 64
	
	;void M68K_WW(unsigned int Adr, unsigned short Data)
	global SYM(M68K_WW)
	SYM(M68K_WW):
		
		push	ebx
		push	ecx
		
		mov	ecx, [esp + 12]		; Address
		mov	eax, [esp + 16]		; Data
		mov	ebx, ecx
		and	ecx, 0xF00000
		and	eax, 0xFFFF
		shr	ecx, 18
		and	ebx, 0xFFFFFF
		jmp	[SYM(M68K_Write_Word_Table) + ecx]
	
	
	;******** Read Byte Proc
	
	align 64
	
	global M68K_Read_Byte_Bad
	M68K_Read_Byte_Bad:
		xor	al, al
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom0
	M68K_Read_Byte_Rom0:
		and	ebx, 0x7FFFF
		xor	ebx, 1
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom1
	M68K_Read_Byte_Rom1:
		and	ebx, 0x7FFFF
		xor	ebx, 0x080001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom2
	M68K_Read_Byte_Rom2:
		and	ebx, 0x7FFFF
		xor	ebx, 0x100001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom3
	M68K_Read_Byte_Rom3:
		and	ebx, 0x7FFFF
		xor	ebx, 0x180001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom4
	M68K_Read_Byte_Rom4:
		test	byte [SYM(SRAM_ON)], 1
		jz	short .Rom
		cmp	ebx, [SYM(SRAM_Start)]
		jb	short .Rom
		cmp	ebx, [SYM(SRAM_End)]
		ja	short .Rom
		
		; If SRAM is disabled by the user, act like SRAM is disabled.
		test	byte [SYM(SRAM_Enabled)], 1
		jz	short .Rom
		
		test	byte [SYM(SRAM_Custom)], 1
		jnz	short .Custom_SRAM
		
		sub	ebx, [SYM(SRAM_Start)]
		mov	al, [SYM(SRAM) + ebx]	; no byte swapped
		pop	ebx
		ret
	
	align 16
	
	.Rom:
		and	ebx, 0x7FFFF
		xor	ebx, 0x200001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 4
	
	.Custom_SRAM:
		mov	al, 0
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom5
	M68K_Read_Byte_Rom5:
		and	ebx, 0x7FFFF
		xor	ebx, 0x280001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom6
	M68K_Read_Byte_Rom6:
		and	ebx, 0x7FFFF
		xor	ebx, 0x300001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Rom7
	M68K_Read_Byte_Rom7:
		and	ebx, 0x7FFFF
		xor	ebx, 0x380001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_Rom8:
		and	ebx, 0x7FFFF
		xor	ebx, 0x400001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_Rom9:
		and	ebx, 0x7FFFF
		xor	ebx, 0x480001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_RomA:
		and	ebx, 0x7FFFF
		xor	ebx, 0x500001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_RomB:
		and	ebx, 0x7FFFF
		xor	ebx, 0x580001
		mov	al, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_RomC:
	M68K_Read_Byte_RomD:
	M68K_Read_Byte_RomE:
	M68K_Read_Byte_RomF:
		xor	al, al
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_Ram
	M68K_Read_Byte_Ram:
		and	ebx, 0xFFFF
		xor	ebx, 1
		mov	al, [SYM(Ram_68k) + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Byte_Misc:
		cmp	ebx, 0xA0FFFF
		ja	short .no_Z80_mem
		
		test	byte [SYM(Z80_State)], (Z80_STATE_BUSREQ | Z80_STATE_RESET)
		jnz	short .bad
		
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
	
	align 4
	
	.bad:
		xor	al, al
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
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
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
		ja	short .bad
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_RB + ebx * 2]
	
	align 16
	
	.Table_IO_RB:
		dd	.MD_Spec, .Pad_1, .Pad_2, .Ser
		dd	.CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad
	
	align 16
	
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
	
	.Ser:
		mov	al, 0xFF
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
	
	align 8
	
	.CT_Ser:
		xor	al, al
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Byte_VDP
	M68K_Read_Byte_VDP:
		cmp	ebx, 0xC00004
		jb	short .bad
		cmp	ebx, 0xC00008
		jb	short .vdp_status
		cmp	ebx, 0xC00009
		ja	short .bad
	
	.vdp_counter:
		test	ebx, 1
		jnz	short .vdp_h_counter
	
	.vdp_v_counter:
		call	SYM(Read_VDP_V_Counter)
		pop	ebx
		ret
	
	align 4
	
	.vdp_h_counter:
		call	SYM(Read_VDP_H_Counter)
		pop	ebx
		ret
	
	align 4
	
	.bad:
		xor	al, al
		pop	ebx
		ret
	
	align 16
	
	.vdp_status:
		call	SYM(Read_VDP_Status)
		test	ebx, 1
		jnz	.no_swap_status
		mov	al, ah			; on lit que le poids fort
	
	.no_swap_status:
		pop	ebx
		ret
	
	;******** Read Word Proc
	
	align 64
	
	global M68K_Read_Word_Bad
	M68K_Read_Word_Bad:
		xor	ax, ax
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom0
	M68K_Read_Word_Rom0:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom1
	M68K_Read_Word_Rom1:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x080000]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom2
	M68K_Read_Word_Rom2:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x100000]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom3
	M68K_Read_Word_Rom3:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x180000]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom4
	M68K_Read_Word_Rom4:
		test	dword [SYM(SRAM_ON)], 1
		jz	short .Rom
		cmp	ebx, [SYM(SRAM_Start)]
		jb	short .Rom
		cmp	ebx, [SYM(SRAM_End)]
		ja	short .Rom
		
		; If SRAM is disabled by the user, act like SRAM is disabled.
		test	byte [SYM(SRAM_Enabled)], 1
		jz	short .Rom
		
		test	byte [SYM(SRAM_Custom)], 1
		jnz	short .Custom_SRAM
		
		sub	ebx, [SYM(SRAM_Start)]
		mov	ax, [SYM(SRAM) + ebx]	; no byte swapped
		rol	ax, 8
		pop	ebx
		ret
	
	align 16
	
	.Rom:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x200000]
		pop	ebx
		ret
	
	align 4

	.Custom_SRAM:
		mov	ax, 0
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom5
	M68K_Read_Word_Rom5:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x280000]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom6
	M68K_Read_Word_Rom6:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x300000]
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Rom7
	M68K_Read_Word_Rom7:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x380000]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_Rom8:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x400000]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_Rom9:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x480000]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_RomA:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x500000]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_RomB:
		and	ebx, 0x7FFFF
		mov	ax, [SYM(Rom_Data) + ebx + 0x580000]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_RomC:
	M68K_Read_Word_RomD:
	M68K_Read_Word_RomE:
	M68K_Read_Word_RomF:
		xor	ax, ax
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_Ram
	M68K_Read_Word_Ram:
		and	ebx, 0xFFFF
		mov	ax, [SYM(Ram_68k) + ebx]
		pop	ebx
		ret
	
	align 16
	
	M68K_Read_Word_Misc:
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
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja	short .bus_taken
		
		mov	al, [SYM(Fake_Fetch)]
		mov	ah, [SYM(Last_BUS_REQ_St)]
		xor	al, 0xFF
		add	ah, 0x80
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
		ja	short .bad
		
		and	ebx, 0x00000E
		jmp	[.Table_IO_RW + ebx * 2]
	
	align 4
	
	.bad:
		xor	eax, eax
		pop	ebx
		ret
	
	align 16
	
	.Table_IO_RW:
		dd	.MD_Spec, .Pad_1, .Pad_2, .Ser
		dd	.CT_Pad_1, .CT_Pad_2, .CT_Ser, .bad
	
	align 16
	
	.MD_Spec:
		mov	eax, [SYM(Game_Mode)]
		add	eax, eax
		or	eax, [SYM(CPU_Mode)]
		shl	eax, 6
		pop	ebx
		or	eax, [SYM(Gen_Version)]		; on recupere les infos hardware de la machine
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
	
	.Ser:
		mov	ax, 0xFF00
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_1:
		mov	eax, [SYM(Controller_1_COM)]
		pop	ebx
		ret
	
	align 8
	
	.CT_Pad_2:
		mov	eax, [SYM(Controller_2_COM)]
		pop	ebx
		ret
	
	align 8
	
	.CT_Ser:
		xor	eax, eax
		pop	ebx
		ret
	
	align 16
	
	global M68K_Read_Word_VDP
	M68K_Read_Word_VDP:
		cmp	ebx, 0xC00003
		ja	short .no_vdp_data
		
		call	SYM(Read_VDP_Data)
		pop	ebx
		ret
	
	align 16
	
	.no_vdp_data:
		cmp	ebx, 0xC00007
		ja	.no_vdp_status
		
		call	SYM(Read_VDP_Status)
		pop	ebx
		ret
	
	align 16
	
	.no_vdp_status:
		cmp	ebx, 0xC00009
		ja	short .bad
		call	SYM(Read_VDP_V_Counter)
		mov	bl, al
		call	SYM(Read_VDP_H_Counter)
		mov	ah, bl
		pop	ebx
		ret
	
	align 4
	
	.bad:
		xor	eax, eax
		pop	ebx
		ret
	
	;******** Write Byte Proc
	
	align 64
	
	global M68K_Write_Bad
	M68K_Write_Bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	global M68K_Write_Byte_SRAM
	M68K_Write_Byte_SRAM:
		test	dword [SYM(SRAM_ON)], 1
		jz	short M68K_Write_Bad
		test	dword [SYM(SRAM_Write)], 1
		jz	short M68K_Write_Bad
		cmp	ebx, [SYM(SRAM_Start)]
		jb	short  M68K_Write_Bad
		cmp	ebx, [SYM(SRAM_End)]
		ja	near M68K_Write_Bad
		
		; If SRAM is disabled by the user, don't do anything.
		test	byte [SYM(SRAM_Enabled)], 1
		jz	near M68K_Write_Bad
		
		sub	ebx, [SYM(SRAM_Start)]
		mov	[SYM(SRAM) + ebx], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	global M68K_Write_Byte_Ram
	M68K_Write_Byte_Ram:
		and	ebx, 0xFFFF
		xor	ebx, 1
		mov	[SYM(Ram_68k) + ebx], al
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Byte_Misc:
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
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		push	eax
		call	SYM(WR_Controller_2)
		pop	eax
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
		cmp	ebx, 0xA130F1
		jb	near .bad
		jne	short .no_sram_ctrl
		
		test	al, 1
		setnz	[SYM(SRAM_ON)]
		test	al, 2
		setz	[SYM(SRAM_Write)]
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_sram_ctrl:
		cmp	ebx, 0xA130FF
		ja	near .bad
		
		; Super Street Fighter II (SSF2) bankswitching system.
		and	ebx, 0xF
		and	eax, 0x1F
		shr	ebx, 1
		mov	ecx, [SYM(Genesis_M68K_Read_Byte_Table) + eax * 4]
		mov	[SYM(M68K_Read_Byte_Table) + ebx * 4], ecx
		mov	ecx, [SYM(Genesis_M68K_Read_Word_Table) + eax * 4]
		mov	[SYM(M68K_Read_Word_Table) + ebx * 4], ecx
		
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	global M68K_Write_Byte_VDP
	M68K_Write_Byte_VDP:
		cmp	ebx, 0xC00003
		ja	short .no_data_port
		
		push	eax
		call	SYM(Write_Byte_VDP_Data)
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_data_port:
		cmp	ebx, 0xC00011
		jne	.bad
		
		push	eax
		call	SYM(PSG_Write)
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	;******** Write Word Proc
	
	align 64
	
	global M68K_Write_Word_SRAM
	M68K_Write_Word_SRAM:
		test	dword [SYM(SRAM_ON)], 1
		jz	short .bad
		test	dword [SYM(SRAM_Write)], 1
		jz	short .bad
		cmp	ebx, [SYM(SRAM_Start)]
		jb	short .bad
		cmp	ebx, [SYM(SRAM_End)]
		ja	short .bad
		
		; If SRAM is disabled by the user, don't do anything.
		test	byte [SYM(SRAM_Enabled)], 1
		jz	near M68K_Write_Bad
		
		rol	ax, 8
		sub	ebx, [SYM(SRAM_Start)]
		mov	[SYM(SRAM) + ebx], ax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	global M68K_Write_Word_Ram
	M68K_Write_Word_Ram:
		and	ebx, 0xFFFF
		mov	[SYM(Ram_68k) + ebx], ax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	M68K_Write_Word_Misc:
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
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 8
	
	.Pad_2:
		push	eax
		call	SYM(WR_Controller_2)
		pop	eax
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
	
	align 16
	
	.no_ctrl_io:
		cmp	ebx, 0xA130F0
		jb	short .bad
		jne	short .no_sram_ctrl
		
		test	ax, 0x1
		setnz	[SYM(SRAM_ON)]
		test	ax, 0x2
		setz	[SYM(SRAM_Write)]
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_sram_ctrl:
		cmp	ebx, 0xA130FF
		ja	short .bad
		
		mov	al, ah
		and	ebx, 0xF
		and	eax, 0x1F
		shr	ebx, 1
		mov	ecx, [SYM(Genesis_M68K_Read_Byte_Table) + eax * 4]
		mov	[SYM(M68K_Read_Byte_Table) + ebx * 4], ecx
		mov	ecx, [SYM(Genesis_M68K_Read_Word_Table) + eax * 4]
		mov	[SYM(M68K_Read_Word_Table) + ebx * 4], ecx
		
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	global M68K_Write_Word_VDP
	M68K_Write_Word_VDP:
		cmp	ebx, 0xC00003
		ja	short .no_data_port
		
		push	eax
		call	SYM(Write_Word_VDP_Data)
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.no_data_port:
		cmp	ebx, 0xC00007
		ja	short .bad
		
		push	eax
		call	SYM(Write_VDP_Ctrl)
		pop	eax
		pop	ecx
		pop	ebx
		ret
	
	align 4
	
	.bad
		pop ecx
		pop ebx
		ret
