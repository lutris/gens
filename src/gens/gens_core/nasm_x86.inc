;
; Gens: NASM (x86) macros.
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

; Output format.
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
