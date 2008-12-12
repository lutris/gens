; Position-independent code macros.

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
