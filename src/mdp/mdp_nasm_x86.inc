;
; MDP: NASM (x86) macros.
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

; Determine the output format.
%ifidn	__OUTPUT_FORMAT__, elf
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, elf32
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, elf64
	%define	__OBJ_ELF
%elifidn __OUTPUT_FORMAT__, win32
	%define	__OBJ_WIN32
	%define __PLATFORM_WINDOWS
	%define	.rodata	.rdata
%elifidn __OUTPUT_FORMAT__, win64
	%define	__OBJ_WIN64
	%define __PLATFORM_WINDOWS
	%define	.rodata	.rdata
%elifidn __OUTPUT_FORMAT__, macho
	%define	__OBJ_MACHO
%endif

%ifdef __OBJ_ELF
	; Mark the stack as non-executable on ELF.
	section .note.GNU-stack noalloc noexec nowrite progbits
%endif

; Symbol declaration.
%ifdef __OBJ_ELF
	%define SYM(x) x
%else
	%define SYM(x) _ %+ x
%endif

%ifdef __PIC
	; Position-independent code.
	; These macros assist with PIC data location.
	
	%ifdef __OBJ_ELF
		%define	__GLOBAL_OFFSET_TABLE	_GLOBAL_OFFSET_TABLE
	%endif
	
	extern __GLOBAL_OFFSET_TABLE
	
	; Get the Global Offset Table.
	%macro get_GOT 0
		call	%%getgot
		%%getgot:
		pop	ebx
		add	ebx, __GLOBAL_OFFSET_TABLE + $$ - %%getgot wrt ..gotpc
	%endmacro
	
	; Get a local data variable. (mov)
	; Syntax: get_mov_localvar destRegister, varName
	%macro get_mov_localvar 2
		mov	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
	
	; Get a local data variable. (movq)
	; Syntax: get_movq_localvar destRegister, varName
	%macro get_movq_localvar 2
		movq	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
	
	; Write a local data variable. (mov)
	; Syntax: put_mov_localvar varName, srcRegister
	%macro put_mov_localvar 2
		mov	[ebx + %1 wrt ..gotoff], %2
	%endmacro
	
	; Write a local data variable. (mov, explicit dword operand size)
	; Syntax: put_mov_dword_localvar varName, srcRegister/immed
	%macro put_mov_dword_localvar 2
		mov	dword [ebx + %1 wrt ..gotoff], %2
	%endmacro
	
	; Write a local data variable. (movq)
	; Syntax: put_movq_localvar varName, srcRegister
	%macro put_movq_localvar 2
		movq	[ebx + %1 wrt ..gotoff], %2
	%endmacro
	
	; Logical "or" a memory address. (explicit dword operand size)
	; Syntax: or_dword_localvar destRegister, srcRegister/immed
	%macro or_dword_localvar 2
		or	dword [ebx + %1 wrt ..gotoff], %2
	%endmacro
	
	; Logical "or" an mmx register with a data variable.
	; Syntax: por_localvar destRegister, varName
	%macro por_localvar 2
		por	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
	
	; Logical "and" an mmx register with a data variable.
	; Syntax: pand_localvar destRegister, varName
	%macro pand_localvar 2
		pand	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
	
	; Packed Subtract Unsigned with Saturation.
	; Syntax: psubusb destRegister, varName
	%macro psubusb_localvar 2
		psubusb	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
%else
	; Not PIC. Define dummy macros.
	%macro get_GOT 0
	%endmacro
	
	%macro get_mov_localvar 2
		mov	%1, [%2]
	%endmacro
	
	%macro get_movq_localvar 2
		movq	%1, [%2]
	%endmacro
	
	%macro put_mov_localvar 2
		mov	[%1], %2
	%endmacro
	
	%macro put_mov_dword_localvar 2
		mov	dword [%1], %2
	%endmacro
	
	%macro put_movq_localvar 2
		movq	[%1], %2
	%endmacro
	
	%macro or_dword_localvar 2
		or	dword [%1], %2
	%endmacro
	
	%macro por_localvar 2
		por	%1, [%2]
	%endmacro
	
	%macro pand_localvar 2
		pand	%1, [%2]
	%endmacro
	
	%macro psubusb_localvar 2
		psubusb	%1, [%2]
	%endmacro
%endif
